/*
 * Copyright © 2012 Intel Corporation
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Jackie Li <yaodong.li@intel.com>
 *
 */
#include <cutils/ashmem.h>
#include <sys/mman.h>
#include <math.h>

#include <IntelHWComposerDrm.h>
#include <IntelOverlayPlane.h>
#include <IntelOverlayUtil.h>
#include <IntelWidiPlane.h>

IntelOverlayContext::~IntelOverlayContext()
{

}

bool IntelOverlayContext::create()
{
    IntelDisplayBuffer *backBuffer;
    int backBufferSize;
    int size = sizeof(intel_overlay_context_t);
    int ret;

    if (!mBufferManager) {
        LOGE("%s: no buffer manager found\n", __func__);
        return false;
    }

    mHandle = ashmem_create_region("intel_overlay_context", size);
    if (mHandle < 0) {
        LOGE("%s: create share memory failed\n", __func__);
        return false;
    }

    mContext = (intel_overlay_context_t*)mmap(NULL, size,
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED | MAP_LOCKED, mHandle, 0);
    if (mContext == MAP_FAILED) {
        LOGE("%s: Map shared Context failed\n", __func__);
        goto mmap_err;
    }

    memset(mContext, 0, size);
    mContext->refCount = 1;

    if (pthread_mutexattr_init(&mContext->attr)) {
        LOGE("%s: Initialize overlay mutex attr failed\n", __func__);
        goto mutexattr_init_err;
    }

    if (pthread_mutexattr_setpshared(&mContext->attr,
                                     PTHREAD_PROCESS_SHARED)) {
        LOGE("%s: set pshared error\n", __func__);
        goto pshared_err;
    }


    if (pthread_mutex_init(&mContext->lock, &mContext->attr)) {
        LOGE("%s: shared block lock init failed\n", __func__);
        goto mutex_init_err;
    }

    // allocate back buffer
    backBufferSize = sizeof(intel_overlay_back_buffer_t);
    backBuffer = mBufferManager->get(backBufferSize, 64 * 1024);
    if (!backBuffer) {
        LOGE("%s: failed to allocate back buffer\n", __func__);
        goto mutex_init_err;
    }

    mContext->gtt_offset_in_page = backBuffer->getGttOffsetInPage();
    mContext->back_buffer_handle = backBuffer->getHandle();

    mSize = size;
    mOverlayBackBuffer = (intel_overlay_back_buffer_t*)backBuffer->getCpuAddr();
    mBackBuffer = backBuffer;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: created overlay context!\n", __func__);

    return true;

mutex_init_err:
pshared_err:
    pthread_mutexattr_destroy(&mContext->attr);
mutexattr_init_err:
    munmap(mContext, size);
mmap_err:
    close(mDrmFd);
    return false;
}

bool IntelOverlayContext::open(int handle, int size)
{
    LOGD_IF(ALLOW_OVERLAY_PRINT,
            "%s: fd %d, size %d\n", __func__, handle, size);

    if (!mBufferManager) {
        LOGE("%s: no buffer manager found\n", __func__);
        return false;
    }

    if (handle <= 0 || size <= 0) {
        LOGE("%s: Invalid parameter\n", __func__);
        return false;
    }

    if ((mHandle > 0) || mContext) {
        LOGE("%s: context exists\n", __func__);
        return false;
    }

    mContext = (intel_overlay_context_t*)mmap(0, size, PROT_READ | PROT_WRITE,
                                              MAP_SHARED | MAP_LOCKED,
                                              handle, 0);

    if (mContext == MAP_FAILED || !mContext) {
        LOGE("%s: map shared context failed\n", __func__);
        return false;
    }

    android_atomic_inc(&mContext->refCount);

    mHandle = handle;
    mSize = size;

    // map back buffer;
    uint32_t backBufferHandle = mContext->back_buffer_handle;
    IntelDisplayBuffer *backBuffer = mBufferManager->map(backBufferHandle);
    if (!backBuffer ||
        backBuffer->getGttOffsetInPage() != mContext->gtt_offset_in_page) {
        LOGE("%s: failed to map back buffer with handle 0x%x\n", __func__,
            backBufferHandle);
        goto map_err;
    }

    mOverlayBackBuffer = (intel_overlay_back_buffer_t*)backBuffer->getCpuAddr();
    mBackBuffer = backBuffer;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: opened overlay context\n", __func__);
    return true;

map_err:
    destroy();
    return false;
}

bool IntelOverlayContext::destroy()
{
    bool ret = true;
    bool closeFd = false;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s\n", __func__);

    if (!mContext) {
        LOGE("%s: context doesn't exist\n", __func__);
        return false;
    }

    if (android_atomic_dec(&mContext->refCount) == 1) {
        LOGD_IF(ALLOW_OVERLAY_PRINT,
               "%s: refcount = 0, destroy mutex\n", __func__);
        if (pthread_mutex_destroy(&mContext->lock)) {
            LOGE("%s: destroy share context lock failed\n", __func__);
            ret = false;
        }

        if (pthread_mutexattr_destroy(&mContext->attr)) {
            LOGE("%s: destroy mutex attr failed\n", __func__);
            ret = false;
        }

        closeFd = true;
    }

    if (munmap(mContext, mSize)) {
        LOGE("%s: failed to destroy overlay context\n", __func__);
        ret = false;
    }

    // destory back buffer;
    if (mBufferManager && mBackBuffer) {
        mBufferManager->put(mBackBuffer);
        mBackBuffer = 0;
        mBufferManager = 0;
    }

    mContext = 0;
    mSize = 0;
    mOverlayBackBuffer = 0;

    if (closeFd && (mHandle > 0)) {
        close(mHandle);
        mHandle = -1;
    }

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: destroyed overlay context\n", __func__);
    return ret;
}

void IntelOverlayContext::clean()
{
    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s\n", __func__);
}

void IntelOverlayContext::lock()
{
    //if (mContext)
    //     pthread_mutex_lock(&mContext->lock);
}

void IntelOverlayContext::unlock()
{
    //if (mContext)
    //    pthread_mutex_unlock(&mContext->lock);
}

void IntelOverlayContext::setBackBufferGttOffset(const uint32_t gttOffset)
{
    lock();

    if (mContext)
        mContext->gtt_offset_in_page = gttOffset;

    unlock();
}

uint32_t IntelOverlayContext::getGttOffsetInPage()
{
    uint32_t offset = 0;

    lock();

    if (mContext)
        offset = mContext->gtt_offset_in_page;

    unlock();

    return offset;
}

intel_overlay_orientation_t IntelOverlayContext::getOrientation()
{
    intel_overlay_orientation_t orientation = OVERLAY_ORIENTATION_PORTRAINT;

    if (!mContext)
        return orientation;

    lock();

    orientation = mContext->orientation;

    unlock();

    return orientation;
}

bool IntelOverlayContext::flush(uint32_t flags)
{
    if (!flags || mDrmFd <= 0)
        return false;

    if (!mContext || !(flags & IntelDisplayPlane::FLASH_NEEDED))
        return false;

    if (!mContext->gtt_offset_in_page) {
        LOGE("%s: invalid gtt offset\n", __func__);
        return false;
    }

    struct drm_psb_register_rw_arg arg;

    memset(&arg, 0, sizeof(struct drm_psb_register_rw_arg));
    arg.overlay_write_mask = 1;
    arg.overlay_read_mask = 0;
    arg.overlay.b_wms = 0;
    arg.overlay.b_wait_vblank = (flags & IntelDisplayPlane::WAIT_VBLANK) ? 1 : 0;
    arg.overlay.OVADD = (mContext->gtt_offset_in_page << 12);
    // pipe select
    arg.overlay.OVADD |= mContext->pipe;
    if (flags & IntelDisplayPlane::UPDATE_COEF)
        arg.overlay.OVADD |= 1;
    int ret = drmCommandWriteRead(mDrmFd,
                                  DRM_PSB_REGISTER_RW,
                                  &arg, sizeof(arg));
    if (ret) {
        LOGW("%s: overlay update failed with error code %d\n",
             __func__, ret);
        return false;
    }

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done\n", __func__);
    return true;
}

bool IntelOverlayContext::waitForFlip()
{
    if (mDrmFd <= 0)
        return false;

    if (!mContext)
        return false;

    struct drm_psb_register_rw_arg arg;

    memset(&arg, 0, sizeof(struct drm_psb_register_rw_arg));
    arg.overlay_write_mask = OV_REGRWBITS_WAIT_FLIP;
    // pipe select
    arg.overlay.OVADD |= mContext->pipe;

    int ret = drmCommandWriteRead(mDrmFd,
                                  DRM_PSB_REGISTER_RW,
                                  &arg, sizeof(arg));
    if (ret) {
        LOGW("%s: overlay update failed with error code %d\n",
             __func__, ret);
        return false;
    }

    LOGV("%s: done\n", __func__);
    return true;
}

bool IntelOverlayContext::backBufferInit()
{
    if(!mOverlayBackBuffer) {
        LOGE("%s: Control Block is NULL\n", __func__);
        return false;
    }

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: control block init...\n", __func__);

    memset(mOverlayBackBuffer, 0, sizeof(intel_overlay_back_buffer_t));

    /*reset overlay*/
    mOverlayBackBuffer->OCLRC0 = (OVERLAY_INIT_CONTRAST << 18) |
                (OVERLAY_INIT_BRIGHTNESS & 0xff);
    mOverlayBackBuffer->OCLRC1 = OVERLAY_INIT_SATURATION;
    mOverlayBackBuffer->DCLRKV = OVERLAY_INIT_COLORKEY;
    mOverlayBackBuffer->DCLRKM = OVERLAY_INIT_COLORKEYMASK;
    mOverlayBackBuffer->OCONFIG = 0;
    mOverlayBackBuffer->OCONFIG |= (0x1 << 3);
    mOverlayBackBuffer->OCONFIG |= (0x1 << 27);
    mOverlayBackBuffer->SCHRKEN &= ~(0x7 << 24);
    mOverlayBackBuffer->SCHRKEN |= 0xff;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: successfully.\n", __func__);
    return true;
}

bool IntelOverlayContext::bufferOffsetSetup(IntelDisplayDataBuffer &buf)
{
    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: setting up buffer offset...\n", __func__);

    if(!mOverlayBackBuffer) {
        LOGE("%s: invalid buf\n", __func__);
        return false;
    }

    uint32_t format = buf.getFormat();
    uint32_t gttOffsetInBytes = (buf.getGttOffsetInPage() << 12);
    yStride = buf.getYStride();
    uvStride = buf.getUVStride();
    uint32_t w = buf.getWidth();
    uint32_t h = buf.getHeight();
    uint32_t srcX= buf.getSrcX();
    uint32_t srcY= buf.getSrcY();

    // clear original format setting
    mOverlayBackBuffer->OCMD &= ~(0xf << 10);

    // Y/U/V plane must be 4k bytes aligned.
    mOverlayBackBuffer->OSTART_0Y = gttOffsetInBytes;
    mOverlayBackBuffer->OSTART_0U = gttOffsetInBytes;
    mOverlayBackBuffer->OSTART_0V = gttOffsetInBytes;

    mOverlayBackBuffer->OSTART_1Y = mOverlayBackBuffer->OSTART_0Y;
    mOverlayBackBuffer->OSTART_1U = mOverlayBackBuffer->OSTART_0U;
    mOverlayBackBuffer->OSTART_1V = mOverlayBackBuffer->OSTART_0V;

    switch(format) {
    case HAL_PIXEL_FORMAT_YV12:    /*YV12*/
        mOverlayBackBuffer->OBUF_0Y = 0;
        mOverlayBackBuffer->OBUF_0V = yStride * h;
        mOverlayBackBuffer->OBUF_0U = mOverlayBackBuffer->OBUF_0V +
                                        (uvStride * (h / 2));

        mOverlayBackBuffer->OCMD |= OVERLAY_FORMAT_PLANAR_YUV420;
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_I420:    /*I420*/
        mOverlayBackBuffer->OBUF_0Y = 0;
        mOverlayBackBuffer->OBUF_0U = yStride * h;
        mOverlayBackBuffer->OBUF_0V = mOverlayBackBuffer->OBUF_0U +
                                        (uvStride * (h / 2));

        mOverlayBackBuffer->OCMD |= OVERLAY_FORMAT_PLANAR_YUV420;
        break;
    /**
     * NOTE: this is the decoded video format, align the height to 32B
     * as it's defined by video driver
     */
    case HAL_PIXEL_FORMAT_INTEL_HWC_NV12:    /*NV12*/
        mOverlayBackBuffer->OBUF_0Y = 0;
        mOverlayBackBuffer->OBUF_0U = yStride * align_to(h, 32);
        mOverlayBackBuffer->OBUF_0V = 0;
        mOverlayBackBuffer->OCMD |= OVERLAY_FORMAT_PLANAR_NV12_2;
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_YUY2:    /*YUY2*/
        mOverlayBackBuffer->OBUF_0Y = 0;
        mOverlayBackBuffer->OBUF_0U = 0;
        mOverlayBackBuffer->OBUF_0V = 0;
        mOverlayBackBuffer->OCMD |= OVERLAY_FORMAT_PACKED_YUV422;
        mOverlayBackBuffer->OCMD |= OVERLAY_PACKED_ORDER_YUY2;
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_UYVY:    /*UYVY*/
        mOverlayBackBuffer->OBUF_0Y = 0;
        mOverlayBackBuffer->OBUF_0U = 0;
        mOverlayBackBuffer->OBUF_0V = 0;
        mOverlayBackBuffer->OCMD |= OVERLAY_FORMAT_PACKED_YUV422;
        mOverlayBackBuffer->OCMD |= OVERLAY_PACKED_ORDER_UYVY;
        break;
    default:
        LOGE("%s: unsupported format %d\n", __func__, format);
        return false;
    }

    mOverlayBackBuffer->OBUF_0Y += srcY * yStride + srcX;
    mOverlayBackBuffer->OBUF_0V += (srcY / 2) * uvStride + srcX;
    mOverlayBackBuffer->OBUF_0U += (srcY / 2) * uvStride + srcX;
    mOverlayBackBuffer->OBUF_1Y = mOverlayBackBuffer->OBUF_0Y;
    mOverlayBackBuffer->OBUF_1U = mOverlayBackBuffer->OBUF_0U;
    mOverlayBackBuffer->OBUF_1V = mOverlayBackBuffer->OBUF_0V;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done. offset (%d, %d, %d)\n", __func__,
                      mOverlayBackBuffer->OBUF_0Y,
                      mOverlayBackBuffer->OBUF_0U,
                      mOverlayBackBuffer->OBUF_0V);
    return true;
}

uint32_t IntelOverlayContext::calculateSWidthSW(uint32_t offset, uint32_t width)
{
    LOGD_IF(ALLOW_OVERLAY_PRINT,
            "%s: calculating SWidthSW...offset %d, width %d\n", __func__,
                                                             offset,
                                                             width);

    uint32_t swidth = ((offset + width + 0x3F) >> 6) - (offset >> 6);

    swidth <<= 1;
    swidth -= 1;

    return swidth;
}

bool IntelOverlayContext::coordinateSetup(IntelDisplayDataBuffer& buf)
{
    uint32_t swidthy = 0;
    uint32_t swidthuv = 0;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: setting up coordinates...\n", __func__);

    if (buf.isFlags(IntelDisplayDataBuffer::SIZE_CHANGE) == false)
        return true;

    uint32_t format = buf.getFormat();
    uint32_t width = buf.getSrcWidth();
    uint32_t height = buf.getSrcHeight();
    uint32_t yStride = buf.getYStride();
    uint32_t uvStride = buf.getUVStride();
    uint32_t offsety = mOverlayBackBuffer->OBUF_0Y;
    uint32_t offsetu = mOverlayBackBuffer->OBUF_0U;

    switch (format) {
    case HAL_PIXEL_FORMAT_YV12:              /*YV12*/
    case HAL_PIXEL_FORMAT_INTEL_HWC_I420:    /*I420*/
    case HAL_PIXEL_FORMAT_INTEL_HWC_NV12:    /*NV12*/
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_YUY2:    /*YUY2*/
    case HAL_PIXEL_FORMAT_INTEL_HWC_UYVY:    /*UYVY*/
        width <<= 1;
        break;
    default:
        LOGE("%s: unsupported format %d\n", __func__, format);
        return false;
    }

    if (width <= 0 || height <= 0) {
        LOGE("%s: invalid src dim\n", __func__);
        return false;
    }

    if (yStride <=0 && uvStride <= 0) {
        LOGE("%s: invalid source stride\n", __func__);
        return false;
    }

    mOverlayBackBuffer->SWIDTH = width | ((width / 2) << 16);
    swidthy = calculateSWidthSW(offsety, width);
    swidthuv = calculateSWidthSW(offsetu, width / 2);
    mOverlayBackBuffer->SWIDTHSW = (swidthy << 2) | (swidthuv << 18);
    mOverlayBackBuffer->SHEIGHT = height | ((height / 2) << 16);
    mOverlayBackBuffer->OSTRIDE = (yStride & (~0x3f)) |
                                  ((uvStride & (~0x3f)) << 16);

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: finished\n", __func__);

    return true;
}

bool IntelOverlayContext::setCoeffRegs(double *coeff, int mantSize,
                                       coeffPtr pCoeff, int pos)
{
    int maxVal, icoeff, res;
    int sign;
    double c;

    sign = 0;
    maxVal = 1 << mantSize;
    c = *coeff;
    if (c < 0.0) {
        sign = 1;
        c = -c;
    }

    res = 12 - mantSize;
    if ((icoeff = (int)(c * 4 * maxVal + 0.5)) < maxVal) {
        pCoeff[pos].exponent = 3;
        pCoeff[pos].mantissa = icoeff << res;
        *coeff = (double)icoeff / (double)(4 * maxVal);
    } else if ((icoeff = (int)(c * 2 * maxVal + 0.5)) < maxVal) {
        pCoeff[pos].exponent = 2;
        pCoeff[pos].mantissa = icoeff << res;
        *coeff = (double)icoeff / (double)(2 * maxVal);
    } else if ((icoeff = (int)(c * maxVal + 0.5)) < maxVal) {
        pCoeff[pos].exponent = 1;
        pCoeff[pos].mantissa = icoeff << res;
        *coeff = (double)icoeff / (double)(maxVal);
    } else if ((icoeff = (int)(c * maxVal * 0.5 + 0.5)) < maxVal) {
        pCoeff[pos].exponent = 0;
        pCoeff[pos].mantissa = icoeff << res;
        *coeff = (double)icoeff / (double)(maxVal / 2);
    } else {
        /* Coeff out of range */
        return false;
    }

    pCoeff[pos].sign = sign;
    if (sign)
        *coeff = -(*coeff);
    return true;
}

void IntelOverlayContext::updateCoeff(int taps, double fCutoff,
                                      bool isHoriz, bool isY,
                                      coeffPtr pCoeff)
{
    int i, j, j1, num, pos, mantSize;
    double pi = 3.1415926535, val, sinc, window, sum;
    double rawCoeff[MAX_TAPS * 32], coeffs[N_PHASES][MAX_TAPS];
    double diff;
    int tapAdjust[MAX_TAPS], tap2Fix;
    bool isVertAndUV;

    if (isHoriz)
        mantSize = 7;
    else
        mantSize = 6;

    isVertAndUV = !isHoriz && !isY;
    num = taps * 16;
    for (i = 0; i < num  * 2; i++) {
        val = (1.0 / fCutoff) * taps * pi * (i - num) / (2 * num);
        if (val == 0.0)
            sinc = 1.0;
        else
            sinc = sin(val) / val;

        /* Hamming window */
        window = (0.54 - 0.46 * cos(2 * i * pi / (2 * num - 1)));
        rawCoeff[i] = sinc * window;
    }

    for (i = 0; i < N_PHASES; i++) {
        /* Normalise the coefficients. */
        sum = 0.0;
        for (j = 0; j < taps; j++) {
            pos = i + j * 32;
            sum += rawCoeff[pos];
        }
        for (j = 0; j < taps; j++) {
            pos = i + j * 32;
            coeffs[i][j] = rawCoeff[pos] / sum;
        }

        /* Set the register values. */
        for (j = 0; j < taps; j++) {
            pos = j + i * taps;
            if ((j == (taps - 1) / 2) && !isVertAndUV)
                setCoeffRegs(&coeffs[i][j], mantSize + 2, pCoeff, pos);
            else
                setCoeffRegs(&coeffs[i][j], mantSize, pCoeff, pos);
        }

        tapAdjust[0] = (taps - 1) / 2;
        for (j = 1, j1 = 1; j <= tapAdjust[0]; j++, j1++) {
            tapAdjust[j1] = tapAdjust[0] - j;
            tapAdjust[++j1] = tapAdjust[0] + j;
        }

        /* Adjust the coefficients. */
        sum = 0.0;
        for (j = 0; j < taps; j++)
            sum += coeffs[i][j];
        if (sum != 1.0) {
            for (j1 = 0; j1 < taps; j1++) {
                tap2Fix = tapAdjust[j1];
                diff = 1.0 - sum;
                coeffs[i][tap2Fix] += diff;
                pos = tap2Fix + i * taps;
                if ((tap2Fix == (taps - 1) / 2) && !isVertAndUV)
                    setCoeffRegs(&coeffs[i][tap2Fix], mantSize + 2, pCoeff, pos);
                else
                    setCoeffRegs(&coeffs[i][tap2Fix], mantSize, pCoeff, pos);

                sum = 0.0;
                for (j = 0; j < taps; j++)
                    sum += coeffs[i][j];
                if (sum == 1.0)
                    break;
            }
        }
    }
}

bool IntelOverlayContext::scalingSetup(IntelDisplayDataBuffer& buffer)
{
    int xscaleInt, xscaleFract, yscaleInt, yscaleFract;
    int xscaleIntUV, xscaleFractUV;
    int yscaleIntUV, yscaleFractUV;
    int deinterlace_factor;
    /* UV is half the size of Y -- YUV420 */
    int uvratio = 2;
    uint32_t newval;
    coeffRec xcoeffY[N_HORIZ_Y_TAPS * N_PHASES];
    coeffRec xcoeffUV[N_HORIZ_UV_TAPS * N_PHASES];
    int i, j, pos;
    bool scaleChanged = false;
    int x, y, w, h;
    if (buffer.mBobDeinterlace) {
        deinterlace_factor = 2;
    } else {
        deinterlace_factor = 1;
    }

    if ((buffer.isFlags(IntelDisplayDataBuffer::SIZE_CHANGE) == false) &&
        (mContext->position_changed == false) &&
        (mContext->is_interlaced == buffer.mBobDeinterlace))
        return true;

    mContext->is_interlaced = buffer.mBobDeinterlace;

    x = mContext->position.x;
    y = mContext->position.y;
    w = mContext->position.w;
    h = mContext->position.h;

    // check position
    checkPosition(x, y, w, h, buffer);
    LOGD_IF(ALLOW_OVERLAY_PRINT,
           "%s: Final position (%d, %d, %d, %d)", __func__, x, y, w, h);

    if ((w <= 0) || (h <= 0)) {
         LOGE("%s: Invalid dst width/height", __func__);
         return false;
    }

    // setup dst position
    mOverlayBackBuffer->DWINPOS = (y << 16) | x;
    mOverlayBackBuffer->DWINSZ = (h << 16) | w;

    uint32_t srcWidth = buffer.getSrcWidth();
    uint32_t srcHeight = buffer.getSrcHeight();
    uint32_t dstWidth = w;
    uint32_t dstHeight = h;

    LOGD_IF(ALLOW_OVERLAY_PRINT,
            "%s: src (%dx%d) v.s. (%dx%d)\n", __func__,
                                           srcWidth, srcHeight,
                                           dstWidth, dstHeight);
    /*
     * Y down-scale factor as a multiple of 4096.
     */
    if (srcWidth == dstWidth && srcHeight == dstHeight) {
        xscaleFract = (1 << 12);
        yscaleFract = (1 << 12)/deinterlace_factor;
    } else {
        xscaleFract = ((srcWidth - 1) << 12) / dstWidth;
        yscaleFract = ((srcHeight - 1) << 12) / (dstHeight * deinterlace_factor);
    }

    /* Calculate the UV scaling factor. */
    xscaleFractUV = xscaleFract / uvratio;
    yscaleFractUV = yscaleFract / uvratio;

    /*
     * To keep the relative Y and UV ratios exact, round the Y scales
     * to a multiple of the Y/UV ratio.
     */
    xscaleFract = xscaleFractUV * uvratio;
    yscaleFract = yscaleFractUV * uvratio;

    /* Integer (un-multiplied) values. */
    xscaleInt = xscaleFract >> 12;
    yscaleInt = yscaleFract >> 12;

    xscaleIntUV = xscaleFractUV >> 12;
    yscaleIntUV = yscaleFractUV >> 12;

    /* Check scaling ratio */
    if (xscaleInt > PVR_OVERLAY_MAX_SCALING_RATIO) {
        LOGE("%s: xscaleInt > %d\n", __func__, PVR_OVERLAY_MAX_SCALING_RATIO);
        return false;
    }

    /* shouldn't get here */
    if (xscaleIntUV > PVR_OVERLAY_MAX_SCALING_RATIO) {
        LOGE("%s: xscaleIntUV > %d\n", __func__, PVR_OVERLAY_MAX_SCALING_RATIO);
        return false;
    }

    newval = (xscaleInt << 15) |
    ((xscaleFract & 0xFFF) << 3) | ((yscaleFract & 0xFFF) << 20);
    if (newval != mOverlayBackBuffer->YRGBSCALE) {
        scaleChanged = true;
        mOverlayBackBuffer->YRGBSCALE = newval;
    }

    newval = (xscaleIntUV << 15) | ((xscaleFractUV & 0xFFF) << 3) |
    ((yscaleFractUV & 0xFFF) << 20);
    if (newval != mOverlayBackBuffer->UVSCALE) {
        scaleChanged = true;
        mOverlayBackBuffer->UVSCALE = newval;
    }

    newval = yscaleInt << 16 | yscaleIntUV;
    if (newval != mOverlayBackBuffer->UVSCALEV) {
        scaleChanged = true;
        mOverlayBackBuffer->UVSCALEV = newval;
    }

    /* Recalculate coefficients if the scaling changed. */
    /*
     * Only Horizontal coefficients so far.
     */
    if (scaleChanged) {
        double fCutoffY;
        double fCutoffUV;

        fCutoffY = xscaleFract / 4096.0;
        fCutoffUV = xscaleFractUV / 4096.0;

        /* Limit to between 1.0 and 3.0. */
        if (fCutoffY < MIN_CUTOFF_FREQ)
            fCutoffY = MIN_CUTOFF_FREQ;
        if (fCutoffY > MAX_CUTOFF_FREQ)
            fCutoffY = MAX_CUTOFF_FREQ;
        if (fCutoffUV < MIN_CUTOFF_FREQ)
            fCutoffUV = MIN_CUTOFF_FREQ;
        if (fCutoffUV > MAX_CUTOFF_FREQ)
            fCutoffUV = MAX_CUTOFF_FREQ;

        updateCoeff(N_HORIZ_Y_TAPS, fCutoffY, true, true, xcoeffY);
        updateCoeff(N_HORIZ_UV_TAPS, fCutoffUV, true, false, xcoeffUV);

        for (i = 0; i < N_PHASES; i++) {
            for (j = 0; j < N_HORIZ_Y_TAPS; j++) {
                pos = i * N_HORIZ_Y_TAPS + j;
                mOverlayBackBuffer->Y_HCOEFS[pos] =
                        (xcoeffY[pos].sign << 15 |
                          xcoeffY[pos].exponent << 12 |
                          xcoeffY[pos].mantissa);
            }
        }
        for (i = 0; i < N_PHASES; i++) {
            for (j = 0; j < N_HORIZ_UV_TAPS; j++) {
                pos = i * N_HORIZ_UV_TAPS + j;
                mOverlayBackBuffer->UV_HCOEFS[pos] =
                         (xcoeffUV[pos].sign << 15 |
                          xcoeffUV[pos].exponent << 12 |
                          xcoeffUV[pos].mantissa);
            }
        }
    }

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done\n", __func__);

    return true;
}

bool IntelOverlayContext::setDataBuffer(IntelDisplayDataBuffer& buffer)
{
    if (!mContext || !mOverlayBackBuffer) {
        LOGE("%s: no overlay context\n", __func__);
        return false;
    }

    lock();

    bool ret = bufferOffsetSetup(buffer);
    if (ret == false) {
        LOGE("%s: failed to set up buffer offsets\n", __func__);
        unlock();
        return false;
    }

    ret = coordinateSetup(buffer);
    if (ret == false) {
        LOGE("%s: failed to set up overlay coordinates\n", __func__);
        unlock();
        return false;
    }

    ret = scalingSetup(buffer);
    if (ret == false) {
        LOGE("%s: failed to set up scaling parameters\n", __func__);
        unlock();
        return false;
    }

    mOverlayBackBuffer->OCMD |= 0x1;

    if (buffer.mBobDeinterlace) {
        mOverlayBackBuffer->OCMD |= BUF_TYPE_FIELD;
        mOverlayBackBuffer->OCMD &= ~FIELD_SELECT;
        mOverlayBackBuffer->OCMD |= FIELD0;
        mOverlayBackBuffer->OCMD &= ~(BUFFER_SELECT);
        mOverlayBackBuffer->OCMD |= BUFFER0;
    } else {
        mOverlayBackBuffer->OCMD |= BUF_TYPE_FRAME;
        mOverlayBackBuffer->OCMD &= ~BUF_TYPE_FIELD;
        mOverlayBackBuffer->OCMD &= ~FIELD_SELECT;
        mOverlayBackBuffer->OCMD &= ~(BUFFER_SELECT);
        mOverlayBackBuffer->OCMD |= BUFFER0;
    }
    buffer.clearFlags();
    mContext->position_changed = false;

    unlock();

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done\n", __func__);

    return true;
}

intel_overlay_state_t IntelOverlayContext::getOverlayState() const
{
    if (mContext) {
        return mContext->state;
    }

    return OVERLAY_INIT;
}

void IntelOverlayContext::setOverlayState(intel_overlay_state_t state)
{
    if (mContext) {
        mContext->state = state;
    }
}

void IntelOverlayContext::setRotation(int rotation)
{
    if (!mContext)
        return;

    /*lock shared context*/
    lock();

#ifdef INTEL_OVERLAY_ROTATION_SUPPORT
    intel_overlay_rotation_t newRotation;
    intel_overlay_orientation_t oldOrientation =
        mContext->orientation;
    intel_overlay_orientation_t newOrientation;

    switch (rotation) {
    case OVERLAY_TRANSFORM_ROT_90:
        newRotation = OVERLAY_ROTATE_90;
        break;
    case OVERLAY_TRANSFORM_ROT_180:
        newRotation = OVERLAY_ROTATE_180;
        break;
    case OVERLAY_TRANSFORM_ROT_270:
        newRotation = OVERLAY_ROTATE_270;
        break;
    case OVERLAY_TRANSFORM_FLIP_H:
    case OVERLAY_TRANSFORM_FLIP_V:
    default:
        newRotation = OVERLAY_ROTATE_0;
        break;
    }

    if (newRotation == OVERLAY_ROTATE_90 ||
        newRotation == OVERLAY_ROTATE_270)
        newOrientation = OVERLAY_ORIENTATION_LANDSCAPE;
    else
        newOrientation = OVERLAY_ORIENTATION_PORTRAINT;

    if (newOrientation != oldOrientation)
       mContext->is_rotated = true;

    mContext->rotation = newRotation;
    mContext->orientation = newOrientation;
#else
    mContext->rotation = OVERLAY_ROTATE_0;
    mContext->orientation = OVERLAY_ORIENTATION_PORTRAINT;
#endif
    unlock();
}

void IntelOverlayContext::checkPosition(int& x, int& y, int& w, int& h,
                                        IntelDisplayDataBuffer& buffer)
{
    if (!mContext)
        return;

    int output;
    drmModeModeInfoPtr mode;
    intel_overlay_mode_t displayMode;
    drmModeConnection connection = DRM_MODE_DISCONNECTED;

    displayMode = IntelHWComposerDrm::getInstance().getDisplayMode();

    /*display full screen size overlay when HDMI is connected*/
    if (displayMode == OVERLAY_EXTEND)
        output = OUTPUT_HDMI;
    else
        output = OUTPUT_MIPI0;

    connection = IntelHWComposerDrm::getInstance().getOutputConnection(output);

    if (connection != DRM_MODE_CONNECTED) {
        LOGE("%s: failed to detect connected state of output %d\n", __func__, output);
        return;
    }

    mode = IntelHWComposerDrm::getInstance().getOutputMode(output);

    if (!mode || !mode->hdisplay || !mode->vdisplay) {
        LOGE("%s: failed to detect mode of output %d\n", __func__, output);
        return;
    }

    if (displayMode == OVERLAY_EXTEND) {
        uint32_t _destw, _desth;
        int _pos_x, _pos_y;
        float _slope_xy;
        uint32_t srcWidth = buffer.getSrcWidth();
        uint32_t srcHeight = buffer.getSrcHeight();

        _slope_xy = (float)srcHeight / srcWidth;
        _destw = (short)(mode->vdisplay / _slope_xy);
        _desth = (short)(mode->hdisplay * _slope_xy);
        if (_destw <= mode->hdisplay) {
            _desth = mode->vdisplay;
            _pos_x = (mode->hdisplay - _destw) >> 1;
            _pos_y = 0;
        } else {
            _destw = mode->hdisplay;
            _pos_x = 0;
            _pos_y = (mode->vdisplay - _desth) >> 1;
        }
        x = _pos_x;
        y = _pos_y;
        w = _destw;
        h = _desth;
    } else {
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if ((x + w) > mode->hdisplay)
            w = mode->hdisplay - x;
        if ((y + h) > mode->vdisplay)
            h = mode->vdisplay - y;
    }
}

void IntelOverlayContext::setPosition(int x, int y, int w, int h)
{
    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: %d, %d, %d, %d\n", __func__, x, y, w, h);

    if (!mContext)
        return;

    // update context and overlay back buffer
    lock();

    if ((x != mContext->position.x) || (y != mContext->position.y) ||
        (w != mContext->position.w) || (h != mContext->position.h))
        mContext->position_changed = true;

    mContext->position.x = x;
    mContext->position.y = y;
    mContext->position.w = w;
    mContext->position.h = h;

    unlock();
}

bool IntelOverlayContext::enable()
{
    if (!mContext)
        return false;

    lock();

    mOverlayBackBuffer->OCMD |= OVERLAY_ENABLE;
    bool ret = flush(IntelDisplayPlane::FLASH_NEEDED | IntelDisplayPlane::WAIT_VBLANK);
    if (ret == false) {
        LOGE("%s: failed to enable overlay\n", __func__);
        unlock();
        return false;
    }

    unlock();
    return true;
}

bool IntelOverlayContext::disable()
{
    if (!mContext)
        return false;

    lock();

    if (!(mOverlayBackBuffer->OCMD & OVERLAY_ENABLE)) {
        unlock();
        return true;
    }

    LOGD("%s: disable overlay...\n", __func__);
    mOverlayBackBuffer->OCMD &= ~OVERLAY_ENABLE;
    bool ret = flush((IntelDisplayPlane::FLASH_NEEDED |
                      IntelDisplayPlane::WAIT_VBLANK));
    if (ret == false) {
        LOGE("%s: failed to disable overlay\n", __func__);
        unlock();
        return false;
    }

    unlock();

    return true;
}

bool IntelOverlayContext::reset()
{
    if (!mContext)
        return false;

    lock();

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: reset overlay...\n", __func__);
    backBufferInit();
    bool ret = flush((IntelDisplayPlane::FLASH_NEEDED |
                      IntelDisplayPlane::WAIT_VBLANK));
    if (ret == false) {
        LOGE("%s: failed to reset overlay\n", __func__);
        unlock();
        return false;
    }

    unlock();

    return true;
}

void IntelOverlayContext::setPipe(intel_display_pipe_t pipe)
{
    if (!mContext)
        return;

    lock();

    // clear pipe bits, this will use MIPI0 by default
    mContext->pipe &= ~(0x3 << 6);

    // disable color key by default
    mOverlayBackBuffer->DCLRKM = ~((0x1 << 31) | 0xffffff);

    switch (pipe) {
    case PIPE_MIPI0:
        break;
    case PIPE_MIPI1:
        mContext->pipe = (0x1 << 6);
        break;
    case PIPE_HDMI:
        mContext->pipe = (0x2 << 6);
        // make sure overlay is on the top of sprite plane
        mOverlayBackBuffer->DCLRKM |= (0x1 << 31);
        mOverlayBackBuffer->DCLRKM |= 0xffffff;
        break;
    default:
	LOGW("%s: invalid display pipe %d\n", __func__, pipe);
	unlock();
	return;
    }

    // need check position
    mContext->position_changed = true;

    unlock();
}

void IntelOverlayContext::setPipeByMode(intel_overlay_mode_t displayMode)
{
    if (!mContext)
        return;

    intel_display_pipe_t pipe;

    switch (displayMode) {
    case OVERLAY_UNKNOWN:
    case OVERLAY_MIPI0:
    case OVERLAY_CLONE_MIPI0:
        pipe = PIPE_MIPI0;
        break;
    case OVERLAY_CLONE_MIPI1:
        pipe = PIPE_MIPI1;
        break;
    case OVERLAY_EXTEND:
        pipe = PIPE_HDMI;
        break;
    case OVERLAY_CLONE_DUAL:
    default:
        LOGW("%s: Unsupported display mode %d\n", __func__, displayMode);
        return;
    }

    setPipe(pipe);
}

uint32_t IntelOverlayContext::getPipe()
{
    uint32_t pipe = 0;

    if (!mContext)
        return 0;

    lock();
        pipe = mContext->pipe;
    unlock();

    return pipe;
}

void IntelOverlayContext::forceBottom(bool bottom)
{
    if (!mContext)
        return;

    lock();

    // this setting only available for overlay A
    if (mContext->pipe) {
        unlock();
        return;
    }

    if (bottom)
        mOverlayBackBuffer->OCONFIG |= (1 << 15);
    else
        mOverlayBackBuffer->OCONFIG &= ~(1 << 15);

    unlock();
}

// DRM mode change handle
intel_overlay_mode_t
IntelOverlayContext::onDrmModeChange()
{
    intel_overlay_mode_t oldDisplayMode;
    intel_overlay_mode_t newDisplayMode = OVERLAY_UNKNOWN;
    struct drm_psb_register_rw_arg arg;
    uint32_t overlayAPipe = 0;
    bool ret = true;

    oldDisplayMode = IntelHWComposerDrm::getInstance().getOldDisplayMode();

    /*get new drm mode*/
    newDisplayMode = IntelHWComposerDrm::getInstance().getDisplayMode();

    LOGD_IF(ALLOW_OVERLAY_PRINT,
           "%s: old %d, new %d\n", __func__, oldDisplayMode, newDisplayMode);

    if (oldDisplayMode == newDisplayMode && !IntelHWComposerDrm::getInstance().isOverlayOff()) {
        goto mode_change_done;
    }

    /*disable overlay*/
    setPipeByMode(oldDisplayMode);
    disable();
    /*switch pipe*/
    setPipeByMode(newDisplayMode);
mode_change_done:
    return newDisplayMode;
}

bool IntelOverlayContextMfld::flush_bottom_field(uint32_t flags)
{
    mOverlayBackBuffer->OCMD |= FIELD1;
    mOverlayBackBuffer->OBUF_0Y = mOverlayBackBuffer->OBUF_0Y - yStride;
    mOverlayBackBuffer->OBUF_0V = mOverlayBackBuffer->OBUF_0V - uvStride;
    mOverlayBackBuffer->OBUF_0U = mOverlayBackBuffer->OBUF_0U - uvStride;
    mOverlayBackBuffer->OBUF_1Y = mOverlayBackBuffer->OBUF_1Y - yStride;
    mOverlayBackBuffer->OBUF_1U = mOverlayBackBuffer->OBUF_1U - uvStride;
    mOverlayBackBuffer->OBUF_1V = mOverlayBackBuffer->OBUF_1V - uvStride;

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done. offset (%d, %d, %d)\n", __func__,
                      mOverlayBackBuffer->OBUF_0Y,
                      mOverlayBackBuffer->OBUF_0U,
                      mOverlayBackBuffer->OBUF_0V);

    if (!flags || mDrmFd <= 0)
        return false;

    if (!mContext || !(flags & IntelDisplayPlane::FLASH_NEEDED))
        return false;

    if (!mContext->gtt_offset_in_page) {
        LOGE("%s: invalid gtt offset\n", __func__);
        return false;
    }

    struct drm_psb_register_rw_arg arg;

    memset(&arg, 0, sizeof(struct drm_psb_register_rw_arg));
    arg.overlay_write_mask = 1;
    arg.overlay_read_mask = 0;
    /*will not wait vblank, otherwise, wait too long will lead to intermittent issue*/
    arg.overlay.b_wait_vblank = 0; //(flags & IntelDisplayPlane::WAIT_VBLANK) ? 1 : 0;
    arg.overlay.OVADD = (mContext->gtt_offset_in_page << 12);
    // pipe select
    arg.overlay.OVADD |= mContext->pipe;
    if (flags & IntelDisplayPlane::UPDATE_COEF)
        arg.overlay.OVADD |= 1;
    int ret = drmCommandWriteRead(mDrmFd,
                                  DRM_PSB_REGISTER_RW,
                                  &arg, sizeof(arg));
    if (ret) {
        LOGW("%s: overlay update failed with error code in bottom %d\n",
             __func__, ret);
        return false;
    }

    return true;
}

bool IntelOverlayContextMfld::flush_frame_or_top_field(uint32_t flags)
{
    if (!flags || mDrmFd <= 0)
        return false;

    if (!mContext || !(flags & IntelDisplayPlane::FLASH_NEEDED))
        return false;

    if (!mContext->gtt_offset_in_page) {
        LOGE("%s: invalid gtt offset\n", __func__);
        return false;
    }

    mOverlayBackBuffer->OCMD |= OVERLAY_ENABLE;

    struct drm_psb_register_rw_arg arg;

    memset(&arg, 0, sizeof(struct drm_psb_register_rw_arg));
    arg.overlay_write_mask = 1;
    arg.overlay_read_mask = 0;
    arg.overlay.b_wms = (flags & IntelDisplayPlane::WMS_NEEDED) ? 1 : 0;
    arg.overlay.b_wait_vblank = (flags & IntelDisplayPlane::WAIT_VBLANK) ? 1 : 0;
    arg.overlay.OVADD = (mContext->gtt_offset_in_page << 12);
    // pipe select
    arg.overlay.OVADD |= mContext->pipe;
    if (flags & IntelDisplayPlane::UPDATE_COEF)
        arg.overlay.OVADD |= 1;
    int ret = drmCommandWriteRead(mDrmFd,
                                  DRM_PSB_REGISTER_RW,
                                  &arg, sizeof(arg));
    if (ret) {
        LOGW("%s: overlay update failed with error code %d\n",
             __func__, ret);
        return false;
    }
    if (flags & IntelDisplayPlane::BOB_DEINTERLACE)
        flush_bottom_field(flags);
    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: done\n", __func__);
    return true;
}

IntelOverlayPlane::IntelOverlayPlane(int fd, int index, IntelBufferManager *bm)
    : IntelDisplayPlane(fd, IntelDisplayPlane::DISPLAY_PLANE_OVERLAY, index, bm)
{
    bool ret;
    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s\n", __func__);

    // create data buffer, pixel format set to NV12 by default
    IntelDisplayBuffer *dataBuffer =
        new IntelDisplayDataBuffer(HAL_PIXEL_FORMAT_INTEL_HWC_NV12, 0, 0);
    if (!dataBuffer) {
        LOGE("%s: Failed to create overlay data buffer\n", __func__);
        return;
    }

    // create overlay context
    IntelOverlayContextMfld *overlayContext = new IntelOverlayContextMfld(fd, bm);
    if (!overlayContext) {
        LOGE("%s: Failed to create overlay context\n", __func__);
        goto overlay_create_err;
    }

    // initialize overlay context
    ret = overlayContext->create();
    if (ret == false) {
        LOGE("%s: Failed to initialize overlay context\n", __func__);
        goto overlay_init_err;
    }

    // clear up overlay buffers
    memset(mDataBuffers, 0, sizeof(mDataBuffers));
    mNextBuffer = 0;

    // initialized successfully
    mDataBuffer = dataBuffer;
    mContext = overlayContext;
    mWidiPlane = NULL;
    mInitialized = true;
    return;
overlay_init_err:
    delete overlayContext;
overlay_create_err:
    delete dataBuffer;
}

IntelOverlayPlane::~IntelOverlayPlane()
{
    if (initCheck()) {
	IntelOverlayContext *overlayContext =
	    reinterpret_cast<IntelOverlayContext*>(mContext);
	// flush context
	overlayContext->flush(IntelDisplayPlane::FLASH_NEEDED);

        // disable overlay
        disable();

        // destroy overlay context
        overlayContext->destroy();

        // delete overlay context;
        delete overlayContext;

        // destroy overlay data buffer;
        delete mDataBuffer;

        mContext = 0;
        mInitialized = false;
    }
}

void IntelOverlayPlane::setPosition(int left, int top, int right, int bottom)
{
    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        overlayContext->setPosition(left, top, (right - left), (bottom - top));
    }
}

bool IntelOverlayPlane::setDataBuffer(uint32_t handle, uint32_t flags,
                                      intel_gralloc_buffer_handle_t* nHandle)
{
    unsigned long long ui64Stamp = 0ULL;
    IntelDisplayBuffer *buffer = 0;
    uint32_t bufferType;

    if (!initCheck()) {
        LOGE("%s: overlay plane wasn't initialized\n", __func__);
        return false;
    }

    if (handle == 0 || nHandle == NULL) {
        LOGE("%s: invalid handle on %d\n", __func__, __LINE__);
        return false;
    }

    ui64Stamp = nHandle->ui64Stamp;

    // verify if HW overlay capable for this data buffer
    IntelDisplayDataBuffer *overlayDataBuffer =
        reinterpret_cast<IntelDisplayDataBuffer*>(mDataBuffer);
    uint32_t yStride, uvStride;
    bool isYUVPacked = false;
    int format = overlayDataBuffer->getFormat();
    uint32_t grallocStride = overlayDataBuffer->getStride();
    uint32_t grallocHeight = overlayDataBuffer->getSrcHeight();
    uint32_t grallocWidth = overlayDataBuffer->getSrcWidth();

    // calculate YUV stride, HW overlay requires 64 bytes alignment.
    switch (format) {
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_INTEL_HWC_I420:
        //
        yStride = align_to(grallocStride, 64);
        uvStride = align_to(yStride >> 1, 64);
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_NV12:
        yStride = align_to(grallocStride, 64);
        uvStride = yStride;
        break;
    case HAL_PIXEL_FORMAT_INTEL_HWC_YUY2:
    case HAL_PIXEL_FORMAT_INTEL_HWC_UYVY:
        yStride = align_to(grallocStride << 1, 64);
        uvStride = 0;
        isYUVPacked = true;
        break;
    default:
        LOGE("%s: unsupported YUV format 0x%x\n", __func__, format);
        return false;
    }

    // check the minimum and maximun stride restriction of HW overlay
    if (yStride < INTEL_OVERLAY_MIN_STRIDE) {
        LOGW("%s: yStride %d is too small, switch to ST", __func__, yStride);
        return false;
    }

    if (isYUVPacked && (yStride > INTEL_OVERLAY_MAX_STRIDE_PACKED)) {
        LOGW("%s: packed YUV stride %d is too big, switch to ST",
             __func__, yStride);
        return false;
    } else if (!isYUVPacked && (yStride > INTEL_OVERLAY_MAX_STRIDE_LINEAR)) {
        LOGW("%s: planar YUV stride %d is too big, switch to ST",
             __func__, yStride);
        return false;
    } else if ((grallocHeight > 2047) || (grallocWidth > 2047)) {
        LOGW("%s: source width or height (%dx%d) is too big, switch to ST",
             __func__, grallocWidth, grallocHeight);
        return false;
    }

    // update data buffer's yuv strides and continue
    overlayDataBuffer->setStride(yStride, uvStride);

    int grallocBuffFd = nHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_NV12 ?
        nHandle->fd[GRALLOC_SUB_BUFFER1] : 0;

    if (flags)
        bufferType = IntelBufferManager::TTM_BUFFER;
    else
        bufferType = IntelBufferManager::GRALLOC_BUFFER;

    for (int i = 0; i < OVERLAY_DATA_BUFFER_NUM_MAX; i++) {
        if (mDataBuffers[i].ui64Stamp == ui64Stamp &&
            mDataBuffers[i].handle == handle &&
            mDataBuffers[i].bufferType == bufferType) {
            buffer = mDataBuffers[i].buffer;
            mNextBuffer = (i + 1) % OVERLAY_DATA_BUFFER_NUM_MAX;
            break;
        }
    }

    LOGD_IF(ALLOW_OVERLAY_PRINT, "%s: next buffer %d\n", __func__, mNextBuffer);

    /** Map the handle if no buffer found **/
    int tryMapTimes = 0;
    while (buffer == NULL) {
        int index = tryMapTimes == 0 ? mNextBuffer : 0;

        if (mDataBuffers[index].buffer ||
            mDataBuffers[index].handle ||
            mDataBuffers[index].ui64Stamp)
        {
            LOGD_IF(ALLOW_OVERLAY_PRINT,
                    "%s: releasing buffer %d...\n", __func__, mNextBuffer);
            if (mDataBuffers[index].bufferType ==
                IntelBufferManager::TTM_BUFFER)
                mBufferManager->unwrap(mDataBuffers[index].buffer);
            else
                mBufferManager->unmap(mDataBuffers[index].buffer);

            memset(&mDataBuffers[index], 0, sizeof(mDataBuffers[index]));
        }

        if (bufferType == IntelBufferManager::TTM_BUFFER)
            buffer = mBufferManager->wrap((void *)handle, 0);
        else
            buffer = mBufferManager->map(handle);

        mNextBuffer = index;
        if (++tryMapTimes > 1) {
            LOGW("%s: Avail memory is low...", __func__);
            break;
        }
    }

    if (buffer == NULL) {
        LOGE("%s: failed to map handle %x\n", __func__, handle);
        return false;
    }

    if (tryMapTimes > 0) {
        LOGD_IF(ALLOW_OVERLAY_PRINT,
               "%s: mapping buffer at %d...\n", __func__, mNextBuffer);
        mDataBuffers[mNextBuffer].ui64Stamp = ui64Stamp;
        mDataBuffers[mNextBuffer].handle = handle;
        mDataBuffers[mNextBuffer].buffer = buffer;
        mDataBuffers[mNextBuffer].bufferType = bufferType;
        mDataBuffers[mNextBuffer].grallocBuffFd = grallocBuffFd;

        // move mNextBuffer pointer
        mNextBuffer = (mNextBuffer + 1) % OVERLAY_DATA_BUFFER_NUM_MAX;
    }

    overlayDataBuffer->setBuffer(buffer);

    mDataBufferHandle = (uint32_t)nHandle;

    // set data buffer :-)
    return setDataBuffer(*overlayDataBuffer);
}

bool IntelOverlayPlane::setDataBuffer(IntelDisplayBuffer& buffer)
{
    bool ret = true;

    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        IntelDisplayBuffer *bufferPtr = &buffer;
        IntelDisplayDataBuffer *overlayDataBuffer =
            reinterpret_cast<IntelDisplayDataBuffer*>(bufferPtr);

        ret = overlayContext->setDataBuffer(*overlayDataBuffer);
        if (ret == false)
            LOGE("%s: failed to set overlay data buffer\n", __func__);
    }

    return ret;
}

bool IntelOverlayPlane::invalidateDataBuffer()
{
    // unmap all BCD buffers from all devices
    if (!initCheck())
        return false;

    for (int i = 0; i < OVERLAY_DATA_BUFFER_NUM_MAX; i++) {
        if (mDataBuffers[i].bufferType == IntelBufferManager::TTM_BUFFER)
            mBufferManager->unwrap(mDataBuffers[i].buffer);
        else
            mBufferManager->unmap(mDataBuffers[i].buffer);
    }

    // clear data buffers
    memset(mDataBuffers, 0, sizeof(mDataBuffers));
    memset(mDataBuffer, 0, sizeof(*mDataBuffer));
    mNextBuffer = 0;
    return true;
}

bool IntelOverlayPlane::flip(void *contexts, uint32_t flags)
{
    bool ret = true;
    if (initCheck()) {
        IntelOverlayContextMfld *overlayContext =
            reinterpret_cast<IntelOverlayContextMfld*>(mContext);

        if ((mWidiPlane && mWidiPlane->isActive()) ||
	    (flags & IntelDisplayPlane::DELAY_DISABLE) ||
            IntelHWComposerDrm::getInstance().isOverlayOff()){
            LOGD_IF(ALLOW_OVERLAY_PRINT,
                    "%s: disable overlay context!\n", __func__);
            ret = disable();
            if (ret == false)
                LOGE("%s: failed to disable overlay\n", __func__);
            // return false as overlay context flip below is bypassed.
            ret = false;
        } else {
            flags |= IntelDisplayPlane::UPDATE_COEF;

            mdfld_plane_contexts_t *planeContexts;
            planeContexts = (mdfld_plane_contexts_t*)contexts;
            if (!planeContexts) {
                LOGE("%s: invalid plane contexts\n", __func__);
                return false;
            }

            planeContexts->overlay_contexts[mIndex].ovadd = 0x0;
            planeContexts->overlay_contexts[mIndex].ovadd =
                (overlayContext->getGttOffsetInPage() << 12);
            planeContexts->overlay_contexts[mIndex].index = mIndex;
            planeContexts->overlay_contexts[mIndex].pipe =
                overlayContext->getPipe();
            planeContexts->active_overlays |= (1 << mIndex);

            if (flags & IntelDisplayPlane::UPDATE_COEF)
                planeContexts->overlay_contexts[mIndex].ovadd |= 0x1;

            LOGD_IF(ALLOW_OVERLAY_PRINT,
                    "%s: overlay context ovadd: 0x%x, pipe:0x%x, index: 0x%x\n",
                        __func__,
                        planeContexts->overlay_contexts[mIndex].ovadd,
                        planeContexts->overlay_contexts[mIndex].pipe,
                        planeContexts->overlay_contexts[mIndex].index);
        }
    }

    return ret;
}

void IntelOverlayPlane::waitForFlipCompletion()
{
    bool ret = true;
    if (initCheck()) {
        IntelOverlayContextMfld *overlayContext =
            reinterpret_cast<IntelOverlayContextMfld*>(mContext);
            ret = overlayContext->waitForFlip();
            if (ret == false)
                LOGE("%s: failed to do overlay flip\n", __func__);
    }
}

bool IntelOverlayPlane::reset()
{
    bool ret = true;

    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        ret = overlayContext->reset();
        if (ret == false)
            LOGE("%s: failed to reset overlay\n", __func__);
    }

    return ret;
}

bool IntelOverlayPlane::disable()
{
    bool ret = true;

    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        ret = overlayContext->disable();
        if (ret == false)
            LOGE("%s: failed to disable overlay\n", __func__);
    }

    return ret;
}

void IntelOverlayPlane::setPipe(intel_display_pipe_t pipe)
{
    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        overlayContext->setPipe(pipe);
    }
}

void IntelOverlayPlane::setPipeByMode(intel_overlay_mode_t displayMode)
{
    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        overlayContext->setPipeByMode(displayMode);
    }
}

void IntelOverlayPlane::forceBottom(bool bottom)
{
    if (initCheck()) {
        mForceBottom = bottom;
    }
}

uint32_t IntelOverlayPlane::onDrmModeChange()
{
    if (initCheck()) {
        IntelOverlayContext *overlayContext =
            reinterpret_cast<IntelOverlayContext*>(mContext);
        return (uint32_t)overlayContext->onDrmModeChange();
    }

    return 0;
}

bool IntelOverlayPlane::setWidiPlane(IntelDisplayPlane* wplane) {

    if(wplane != NULL)
        mWidiPlane = (IntelWidiPlane*)wplane;

    return true;
}

