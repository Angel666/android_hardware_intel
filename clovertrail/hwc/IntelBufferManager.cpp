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
#include <IntelBufferManager.h>
#include <IntelHWComposerDrm.h>
#include <IntelHWComposerCfg.h>
#include <IntelOverlayUtil.h>
#include <IntelOverlayHW.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/ashmem.h>
#include <sys/mman.h>
#include <pvr2d.h>

IntelDisplayDataBuffer::IntelDisplayDataBuffer(uint32_t format,
                                               uint32_t w,
                                               uint32_t h)
        : mFormat(format), mWidth(w), mHeight(h), mBuffer(0), mBobDeinterlace(0)
{
    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: width %d, format 0x%x\n", __func__, w, format);


    mRawStride = 0;
    mYStride = 0;
    mUVStride = 0;
    mSrcX = 0;
    mSrcY = 0;
    mSrcWidth = w;
    mSrcHeight = h;
    mUpdateFlags = (BUFFER_CHANGE | SIZE_CHANGE);
}

void IntelDisplayDataBuffer::setBuffer(IntelDisplayBuffer *buffer)
{
    mBuffer = buffer;

    if (!buffer) return;

    if (mGttOffsetInPage != buffer->getGttOffsetInPage())
        mUpdateFlags |= BUFFER_CHANGE;

    mBufferObject = buffer->getBufferObject();
    mGttOffsetInPage = buffer->getGttOffsetInPage();
    mSize = buffer->getSize();
    mVirtAddr = buffer->getCpuAddr();
}

void IntelDisplayDataBuffer::setStride(uint32_t stride)
{
    if (stride == mRawStride)
        return;

    mRawStride = stride;
    mUpdateFlags |= SIZE_CHANGE;
}

void IntelDisplayDataBuffer::setStride(uint32_t yStride, uint32_t uvStride)
{
    if ((yStride == mYStride) && (uvStride == mUVStride))
        return;

    mYStride = yStride;
    mUVStride = uvStride;
    mUpdateFlags |= SIZE_CHANGE;
}

void IntelDisplayDataBuffer::setWidth(uint32_t w)
{
    if (w == mWidth)
        return;

    mWidth = w;
    mUpdateFlags |= SIZE_CHANGE;
}

void IntelDisplayDataBuffer::setHeight(uint32_t h)
{
    if (h == mHeight)
        return;

    mHeight = h;
    mUpdateFlags |= SIZE_CHANGE;
}

void IntelDisplayDataBuffer::setCrop(int x, int y, int w, int h)
{
    if ((x == (int)mSrcX) && (y == (int)mSrcY) &&
        (w == (int)mSrcWidth) && (h == (int)mSrcHeight))
        return;

    mSrcX = x;
    mSrcY = y;
    mSrcWidth = w;
    mSrcHeight = h;
    mUpdateFlags |= SIZE_CHANGE;
}

void IntelDisplayDataBuffer::setDeinterlaceType(uint32_t bob_deinterlace)
{
    mBobDeinterlace = bob_deinterlace;
}

bool IntelTTMBufferManager::getVideoBridgeIoctl()
{
    union drm_psb_extension_arg arg;
    /*video bridge ioctl = lnc_video_getparam + 1, I know it's ugly!!*/
    const char lncExt[] = "lnc_video_getparam";
    int ret = 0;

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: get video bridge ioctl num...\n", __func__);

    if(mDrmFd <= 0) {
        LOGE("%s: invalid drm fd %d\n", __func__, mDrmFd);
        return false;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: DRM_PSB_EXTENSION %d\n", __func__, DRM_PSB_EXTENSION);

    /*get devOffset via drm IOCTL*/
    strncpy(arg.extension, lncExt, sizeof(lncExt));

    ret = drmCommandWriteRead(mDrmFd, DRM_PSB_EXTENSION, &arg, sizeof(arg));
    if(ret || !arg.rep.exists) {
        LOGE("%s: get device offset failed with error code %d\n",
                  __func__, ret);
        return false;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: video ioctl offset 0x%x\n",
              __func__,
              arg.rep.driver_ioctl_offset + 1);

    mVideoBridgeIoctl = arg.rep.driver_ioctl_offset + 1;

    return true;
}

IntelTTMBufferManager::~IntelTTMBufferManager()
{
    mVideoBridgeIoctl = 0;
    delete mWsbm;
}

bool IntelTTMBufferManager::initialize()
{
    bool ret;

    if (mDrmFd <= 0) {
        LOGE("%s: invalid drm FD\n", __func__);
        return false;
    }

    /*get video bridge ioctl offset for external YUV buffer*/
    if (!mVideoBridgeIoctl) {
        ret = getVideoBridgeIoctl();
        if (ret == false) {
            LOGE("%s: failed to video bridge ioctl\n", __func__);
            return ret;
        }
    }

    IntelWsbm *wsbm = new IntelWsbm(mDrmFd);
    if (!wsbm) {
        LOGE("%s: failed to create wsbm object\n", __func__);
        return false;
    }

    ret = wsbm->initialize();
    if (ret == false) {
        LOGE("%s: failed to initialize wsbm\n", __func__);
        delete wsbm;
        return false;
    }

    mWsbm = wsbm;

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: done\n", __func__);
    return true;
}


IntelDisplayBuffer* IntelTTMBufferManager::map(uint32_t handle)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return 0;
    }

    void *wsbmBufferObject;
    bool ret = mWsbm->wrapTTMBuffer(handle, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: wrap ttm buffer failed\n", __func__);
        return 0;
    }

    void *virtAddr = mWsbm->getCPUAddress(wsbmBufferObject);
    uint32_t gttOffsetInPage = mWsbm->getGttOffset(wsbmBufferObject);
    // FIXME: set the real size
    uint32_t size = 0;

    IntelDisplayBuffer *buf = new IntelDisplayBuffer(wsbmBufferObject,
                                                     virtAddr,
                                                     gttOffsetInPage,
                                                     size);

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: mapped TTM overlay buffer. cpu %p, gtt %d\n",
         __func__, virtAddr, gttOffsetInPage);

    return buf;
}

void IntelTTMBufferManager::unmap(IntelDisplayBuffer *buffer)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return;
    }
    mWsbm->unreferenceTTMBuffer(buffer->getBufferObject());
    // destroy it
    delete buffer;
}

IntelDisplayBuffer* IntelTTMBufferManager::get(int size, int alignment)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return NULL;
    }

    void *wsbmBufferObject = NULL;
    bool ret = mWsbm->allocateTTMBuffer(size, alignment, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: failed to allocate buffer. size %d, alignment %d\n",
            __func__, size, alignment);
        return NULL;
    }

    void *virtAddr = mWsbm->getCPUAddress(wsbmBufferObject);
    uint32_t gttOffsetInPage = mWsbm->getGttOffset(wsbmBufferObject);
    uint32_t handle = mWsbm->getKBufHandle(wsbmBufferObject);
    // create new buffer
    IntelDisplayBuffer *buffer = new IntelDisplayBuffer(wsbmBufferObject,
                                                        virtAddr,
                                                        gttOffsetInPage,
                                                        size,
                                                        handle);
    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: created TTM overlay buffer. cpu %p, gtt %d\n",
         __func__, virtAddr, gttOffsetInPage);
    return buffer;
}

void IntelTTMBufferManager::put(IntelDisplayBuffer* buf)
{
    if (!buf || !mWsbm) {
        LOGE("%s: Invalid parameter\n", __func__);
        return;
    }

    void *wsbmBufferObject = buf->getBufferObject();
    bool ret = mWsbm->destroyTTMBuffer(wsbmBufferObject);
    if (ret == false)
        LOGW("%s: failed to free wsbmBO\n", __func__);

    // free overlay buffer
    delete buf;
}

IntelPVRBufferManager::~IntelPVRBufferManager()
{
    pvr2DDestroy();
}

bool IntelPVRBufferManager::initialize()
{
    bool ret = pvr2DInit();
    if (ret == false) {
        LOGE("%s: failed to init PVR2D\n", __func__);
        return false;
    }
    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: done\n", __func__);
    return true;
}

bool IntelPVRBufferManager::pvr2DInit()
{
    bool ret = false;
    int pvrDevices = 0;
    PVR2DDEVICEINFO *pvrDevInfo = NULL;
    unsigned long pvrDevID = 0;
    PVR2DERROR eResult = PVR2D_OK;

    if (mPVR2DHandle) {
        LOGW("%s: overlay HAL already has PVR2D handle %p\n",
              __func__, mPVR2DHandle);
        return true;
    }

    pvrDevices = PVR2DEnumerateDevices(0);
    if (pvrDevices <= 0) {
        if (pvrDevices == PVR2DERROR_DEVICE_UNAVAILABLE) {
            LOGE("%s: Cannot connect to PVR services\n", __func__);
            /*FIXME: should I wait here?*/
        }

        LOGE("%s: device not found\n", __FUNCTION__);
        goto pvr_init_err;
    }

    pvrDevInfo =  (PVR2DDEVICEINFO *)malloc(pvrDevices * sizeof(PVR2DDEVICEINFO));
    if (!pvrDevInfo) {
        LOGE("%s: no memory\n", __FUNCTION__);
        goto pvr_init_err;
    }

    pvrDevices = PVR2DEnumerateDevices(pvrDevInfo);
    if(pvrDevices != PVR2D_OK) {
        LOGE("%s: Enumerate device failed\n", __func__);
        goto pvr_init_err;
    }

    /*use the 1st device*/
    pvrDevID = pvrDevInfo[0].ulDevID;
    eResult = PVR2DCreateDeviceContext(pvrDevID, &mPVR2DHandle, 0);
    if (eResult != PVR2D_OK) {
        LOGE("%s: Create device context failed\n", __func__);
        goto pvr_init_err;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT,
            "%s: pvr2d context inited. handle %p\n", __func__, mPVR2DHandle);
    ret = true;

pvr_init_err:
    if(pvrDevInfo) {
        free(pvrDevInfo);
    }

    return ret;
}

void IntelPVRBufferManager::pvr2DDestroy()
{
    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: destroying...\n", __func__);

    if(mPVR2DHandle) {
        PVR2DDestroyDeviceContext(mPVR2DHandle);
        mPVR2DHandle = 0;
    }
}

bool IntelPVRBufferManager::gttMap(PVR2DMEMINFO *buf,
                                   int *offset,
                                   uint32_t virt,
                                   uint32_t size,
                                   uint32_t gttAlign)
{
    struct psb_gtt_mapping_arg arg;
    void * hKernelMemInfo = NULL;

    if (!buf || !offset) {
        LOGE("%s: invalid parameters.\n", __func__);
        return false;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: mapping to gtt. buffer %p, offset %p\n",
         __func__, buf, offset);

    if (mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    hKernelMemInfo = (void *)buf->hPrivateMapData;
    if(!hKernelMemInfo) {
        LOGE("%s: kernel meminfo handle is NULL\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_MEMINFO;
    arg.hKernelMemInfo = hKernelMemInfo;
    arg.page_align = gttAlign;

    int ret = drmCommandWriteRead(mDrmFd, DRM_PSB_GTT_MAP, &arg, sizeof(arg));
    if (ret) {
        LOGE("%s: gtt mapping failed\n", __func__);
        return false;
    }

    *offset =  arg.offset_pages;

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: mapped succussfully, gtt offset %d\n", __func__, *offset);
    return true;
}

bool IntelPVRBufferManager::gttUnmap(PVR2DMEMINFO *buf)
{
    struct psb_gtt_mapping_arg arg;
    void * hKernelMemInfo = NULL;

    if(!buf) {
        LOGE("%s: invalid parameters.\n", __func__);
        return false;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: unmapping from gtt. buffer %p\n", __func__, buf);

    if(mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    hKernelMemInfo = (void *)buf->hPrivateMapData;
    if(!hKernelMemInfo) {
        LOGE("%s: kernel meminfo handle is NULL\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_MEMINFO;
    arg.hKernelMemInfo = hKernelMemInfo;

    int ret = drmCommandWrite(mDrmFd, DRM_PSB_GTT_UNMAP, &arg, sizeof(arg));
    if(ret) {
        LOGE("%s: gtt unmapping failed\n", __func__);
        return false;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: unmapped successfully.\n", __func__);
    return true;
}

IntelDisplayBuffer*
IntelPVRBufferManager::wrap(void *virt, int size)
{
    if (!virt || size <= 0) {
        LOGE("%s: invalid parameters\n", __func__);
        return 0;
    }

    if (!mPVR2DHandle) {
        LOGE("%s: PVR wasn't initialized\n", __func__);
        return 0;
    }

    PVR2DERROR eResult = PVR2D_OK;

    PVR2DMEMINFO *pvr2dMemInfo;
    /*wrap it to a PVR2DMemInfo*/
    eResult = PVR2DMemWrap(mPVR2DHandle, virt, 0, size, NULL, &pvr2dMemInfo);
    if (eResult != PVR2D_OK) {
        LOGE("%s: failed to wrap memory\n", __func__);
        return 0;
    }

    int gttOffsetInPage = 0;
    uint32_t gttPageAlignment = 16;

    /*map it to GTT*/
    bool ret = gttMap(pvr2dMemInfo, &gttOffsetInPage,
                      (uint32_t)virt, (uint32_t)size, gttPageAlignment);
    if (ret == false) {
        LOGE("%s: Failed to map to GTT\n", __func__);
        PVR2DMemFree(mPVR2DHandle, pvr2dMemInfo);
        return 0;
    }

    IntelDisplayBuffer *buffer = new IntelDisplayBuffer(pvr2dMemInfo,
                                                        virt,
                                                        gttOffsetInPage,
                                                        size);
    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: done\n", __func__);
    return buffer;
}

void IntelPVRBufferManager::unwrap(IntelDisplayBuffer *buffer)
{
    if (!mPVR2DHandle) {
        LOGE("%s: PVR wasn't initialized\n", __func__);
        return;
    }

    PVR2DMEMINFO *pvr2dMemInfo = (PVR2DMEMINFO*)buffer->getBufferObject();

    /*unmap it from GTT*/
    bool ret = gttUnmap(pvr2dMemInfo);
    if (ret == false)
        LOGW("%s: failed to unmap %p\n", __func__, pvr2dMemInfo);

    /*unwrap this meminfo*/
    PVR2DMemFree(mPVR2DHandle, pvr2dMemInfo);

    // destroy overlay buffer
    delete buffer;

    LOGD_IF(ALLOW_BUFFER_PRINT, "%s: done\n", __func__);
}

IntelDisplayBuffer* IntelPVRBufferManager::map(uint32_t handle)
{
    if (!mPVR2DHandle) {
        LOGE("%s: PVR wasn't initialized\n", __func__);
        return 0;
    }

    if (!handle) {
        LOGE("%s: invalid buffer handle\n", __func__);
        return 0;
    }

    PVR2DMEMINFO *pvr2dMemInfo;

    PVR2DERROR err = PVR2DMemMap(mPVR2DHandle, 0, (void*)handle, &pvr2dMemInfo);
    if (err != PVR2D_OK) {
        LOGE("%s: failed to map handle 0x%x\n", __func__, handle);
        return 0;
    }
    void *virtAddr = pvr2dMemInfo->pBase;
    uint32_t size = pvr2dMemInfo->ui32MemSize;
    int gttOffsetInPage = 0;

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: virt %p, size %dB\n", __func__, virtAddr, size);

    // map it into gtt
    bool ret = gttMap(pvr2dMemInfo, &gttOffsetInPage,
                      (uint32_t)virtAddr, size, 1);
    if (!ret) {
        LOGE("%s: failed to map 0x%x to GTT\n", __func__, handle);
        PVR2DMemFree(mPVR2DHandle, pvr2dMemInfo);
        return 0;
    }

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: mapped handle 0x%x, gtt %d\n", __func__, handle,
         gttOffsetInPage);

    IntelDisplayBuffer *buffer = new IntelDisplayBuffer(pvr2dMemInfo,
                                                        virtAddr,
                                                        gttOffsetInPage,
                                                        size,
                                                        handle);
    return buffer;
}

void IntelPVRBufferManager::unmap(IntelDisplayBuffer *buffer)
{
    if (!mPVR2DHandle) {
       LOGE("%s: PVR wasn't initialized\n", __func__);
        return;
    }

    if (!buffer) {
        LOGE("%s: invalid buffer\n", __func__);
        return;
    }

    PVR2DMEMINFO *pvr2dMemInfo = (PVR2DMEMINFO*)buffer->getBufferObject();

    if (!pvr2dMemInfo)
        return;

    // unmap from gtt
    gttUnmap(pvr2dMemInfo);

    // unmap buffer
    PVR2DMemFree(mPVR2DHandle, pvr2dMemInfo);
}

IntelBCDBufferManager::IntelBCDBufferManager(int fd)
    : IntelBufferManager(fd), mWsbm(0)
{

}

IntelBCDBufferManager::~IntelBCDBufferManager()
{
    if (initCheck())
        delete mWsbm;
}

bool IntelBCDBufferManager::gttMap(uint32_t devId, uint32_t bufferId,
                                   uint32_t gttAlign,
                                   int *offset)
{
    struct psb_gtt_mapping_arg arg;

    if (!offset) {
        LOGE("%s: invalid parameters.\n", __func__);
        return false;
    }

    if (mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_BCD;
    arg.bcd_device_id = devId;
    arg.bcd_buffer_id = bufferId;
    arg.page_align = gttAlign;

    int ret = drmCommandWriteRead(mDrmFd, DRM_PSB_GTT_MAP, &arg, sizeof(arg));
    if (ret) {
        LOGE("%s: gtt mapping failed\n", __func__);
        return false;
    }

    *offset =  arg.offset_pages;
    return true;
}

bool IntelBCDBufferManager::gttUnmap(uint32_t devId, uint32_t bufferId)
{
    struct psb_gtt_mapping_arg arg;

    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: unmapping from gtt. buffer %p\n", __func__, bufferId);

    if(mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_BCD;
    arg.bcd_device_id = devId;
    arg.bcd_buffer_id = bufferId;

    int ret = drmCommandWrite(mDrmFd, DRM_PSB_GTT_UNMAP, &arg, sizeof(arg));
    if(ret) {
        LOGE("%s: gtt unmapping failed\n", __func__);
        return false;
    }

    return true;
}

bool IntelBCDBufferManager::getBCDInfo(uint32_t devId,
                                       uint32_t *count,
                                       uint32_t *stride)
{
    struct psb_gtt_mapping_arg arg;

    if (!count || !stride) {
        LOGE("%s: invalid parameters.\n", __func__);
        return false;
    }

    if (mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_BCD_INFO;
    arg.bcd_device_id = devId;

    int ret = drmCommandWriteRead(mDrmFd, DRM_PSB_GTT_MAP, &arg, sizeof(arg));
    if (ret) {
        LOGE("%s: gtt mapping failed\n", __func__);
        return false;
    }

    *count =  arg.bcd_buffer_count;
    *stride = arg.bcd_buffer_stride;
    return true;
}

bool IntelBCDBufferManager::initialize()
{
     // FIXME: remove wsbm later
    IntelWsbm *wsbm = new IntelWsbm(mDrmFd);
    if (!wsbm) {
        LOGE("%s: failed to create wsbm object\n", __func__);
        goto open_dev_err;
    }

    if (!(wsbm->initialize())) {
        LOGE("%s: failed to initialize wsbm\n", __func__);
        goto wsbm_err;
    }

    mWsbm = wsbm;
    mInitialized = true;
    return true;

wsbm_err:
    delete wsbm;
open_dev_err:
    mInitialized = false;
    return false;
}

IntelDisplayBuffer* IntelBCDBufferManager::map(uint32_t device, uint32_t handle)
{
    return 0;
}

void IntelBCDBufferManager::unmap(IntelDisplayBuffer *buffer)
{

}

IntelDisplayBuffer** IntelBCDBufferManager::map(uint32_t device,
                                                uint32_t *count)
{
    unsigned int i;
    bool ret;

    if (device >= INTEL_BCD_DEVICE_NUM_MAX || !count) {
        LOGE("%s: invalid parameters\n", __func__);
        return 0;
    }

    if (!initCheck()) {
        LOGE("%s: BCD buffer manager wasn't initialized\n", __func__);
        return 0;
    }

    // map out buffers from this device
    uint32_t bufferCount;
    uint32_t bufferStride;

    ret = getBCDInfo(device, &bufferCount, &bufferStride);
    if (!ret) {
        LOGE("%s: failed to get BCD info for device %d\n", __func__, device);
        return 0;
    }

    if (!bufferCount || !bufferStride) {
        LOGE("%s: no buffer exists in BCD device %d\n", __func__, device);
        return 0;
    }

    IntelDisplayBuffer **bufferList =
       (IntelDisplayBuffer**)malloc(bufferCount * sizeof(IntelDisplayBuffer*));
    if (!bufferList) {
        LOGE("%s: failed to allocate buffer list\n", __func__);
        return 0;
    }

    // clear up the list
    memset(bufferList, 0, sizeof(IntelDisplayBuffer*));

    int gttOffsetInPage;
    void *virtAddr = 0;
    uint32_t size = 0;

    for (i = 0; i < bufferCount; i++) {
        // map into gtt
        ret = gttMap(device, i, 0, &gttOffsetInPage);
        if (!ret) {
            LOGE("%s: failed to map to GTT\n", __func__);
            goto gtt_map_err;
        }

        LOGD_IF(ALLOW_BUFFER_PRINT,
               "%s: creating buffer, dev %d buffer %d, gtt %d\n",
             __func__, device, i, gttOffsetInPage);

        bufferList[i] = new IntelDisplayBuffer(0,//(void *)devHandle,
                                               virtAddr,
                                               gttOffsetInPage,
                                               size,
                                               (uint32_t)device);
        if (!bufferList[i]) {
            LOGE("%s: failed to create new buffer\n", __func__);
            goto buf_err;
        }

        bufferList[i]->setStride(bufferStride);
    }

    *count = bufferCount;
    return bufferList;

buf_err:
    gttUnmap(device, i);
gtt_map_err:
    for ( int j = i - 1; j >= 0; j--) {
        if (bufferList[i]) {
            gttUnmap(device, i);
            delete bufferList[i];

        }
    }
    free(bufferList);
    return 0;
}

void IntelBCDBufferManager::unmap(IntelDisplayBuffer **buffers, uint32_t count)
{
    if (!buffers || !count)
        return;

    if (!initCheck())
        return;

    IMG_HANDLE devHandle = (IMG_HANDLE)buffers[0]->getBufferObject();
    uint32_t device = (uint32_t)buffers[0]->getHandle();

    for (uint32_t i = 0; i < count; i++) {
        // unmap from gtt
        gttUnmap(device, i);

        LOGD_IF(ALLOW_BUFFER_PRINT,
                "%s: dev %d, buffer %d\n", __func__, device, i);

        // delete
        delete buffers[i];
    }

    // delete buffer list
    free(buffers);
}

IntelDisplayBuffer* IntelBCDBufferManager::get(int size, int alignment)
{
    if (!initCheck()) {
        LOGE("%s: BCD Buffer Manager wasn't initialized\n", __func__);
        return 0;
    }

    void *wsbmBufferObject = NULL;
    bool ret = mWsbm->allocateTTMBuffer(size, alignment, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: failed to allocate buffer. size %d, alignment %d\n",
            __func__, size, alignment);
        return NULL;
    }

    void *virtAddr = mWsbm->getCPUAddress(wsbmBufferObject);
    uint32_t gttOffsetInPage = mWsbm->getGttOffset(wsbmBufferObject);
    uint32_t handle = mWsbm->getKBufHandle(wsbmBufferObject);
    // create new buffer
    IntelDisplayBuffer *buffer = new IntelDisplayBuffer(wsbmBufferObject,
                                                        virtAddr,
                                                        gttOffsetInPage,
                                                        size,
                                                        handle);
    LOGD_IF(ALLOW_BUFFER_PRINT,
           "%s: created TTM overlay buffer. cpu %p, gtt %d\n",
         __func__, virtAddr, gttOffsetInPage);
    return buffer;
}

void IntelBCDBufferManager::put(IntelDisplayBuffer* buf)
{
    if (!buf || !mWsbm) {
        LOGE("%s: Invalid parameter\n", __func__);
        return;
    }

    void *wsbmBufferObject = buf->getBufferObject();
    bool ret = mWsbm->destroyTTMBuffer(wsbmBufferObject);
    if (ret == false)
        LOGW("%s: failed to free wsbmBO\n", __func__);

    // free overlay buffer
    delete buf;
}

IntelGraphicBufferManager::~IntelGraphicBufferManager()
{
    if (initCheck()) {
        // destroy device memory context
	PVRSRVDestroyDeviceMemContext(&mDevData, mDevMemContext);

        // close connection to PVR services
        PVRSRVDisconnect(mPVRSrvConnection);

        // delete wsbm
        delete mWsbm;
    }
}

bool IntelGraphicBufferManager::gttMap(PVRSRV_CLIENT_MEM_INFO *memInfo,
                                       uint32_t gttAlign,
                                       int *offset)
{
    struct psb_gtt_mapping_arg arg;

    if (!memInfo || !offset) {
        LOGE("%s: invalid parameters.\n", __func__);
        return false;
    }

    if (mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_MEMINFO;
    arg.hKernelMemInfo = memInfo->hKernelMemInfo;
    arg.page_align = gttAlign;

    int ret = drmCommandWriteRead(mDrmFd, DRM_PSB_GTT_MAP, &arg, sizeof(arg));
    if (ret) {
        LOGE("%s: gtt mapping failed\n", __func__);
        return false;
    }

    *offset =  arg.offset_pages;
    return true;
}

bool IntelGraphicBufferManager::gttUnmap(PVRSRV_CLIENT_MEM_INFO *memInfo)
{
    struct psb_gtt_mapping_arg arg;

    if (!memInfo) {
        LOGE("%s: invalid parameter\n", __func__);
        return false;
    }

    if(mDrmFd < 0) {
        LOGE("%s: drm is not ready\n", __func__);
        return false;
    }

    arg.type = PSB_GTT_MAP_TYPE_MEMINFO;
    arg.hKernelMemInfo = memInfo->hKernelMemInfo;

    int ret = drmCommandWrite(mDrmFd, DRM_PSB_GTT_UNMAP, &arg, sizeof(arg));
    if(ret) {
        LOGE("%s: gtt unmapping failed\n", __func__);
        return false;
    }

    return true;
}

bool IntelGraphicBufferManager::initialize()
{
    PVRSRV_ERROR res;
    PVRSRV_CONNECTION *pvrConnection;
    PVRSRV_DEVICE_IDENTIFIER devIDs[PVRSRV_MAX_DEVICES];
    IMG_UINT32 devNum = 0;
    PVRSRV_DEV_DATA *pvr3DDevData = &mDevData;
    IMG_UINT32 heapCount;
    PVRSRV_HEAP_INFO heapInfos[PVRSRV_MAX_CLIENT_HEAPS];

    IntelWsbm *wsbm = new IntelWsbm(mDrmFd);
    if (!wsbm) {
        LOGE("%s: failed to create wsbm object\n", __func__);
        return false;
    }

    if (!(wsbm->initialize())) {
        LOGE("%s: failed to initialize wsbm\n", __func__);
        delete wsbm;
        return false;
    }

    mWsbm = wsbm;

    // connect to PVR Service
    res = PVRSRVConnect(&pvrConnection, 0);
    if (res != PVRSRV_OK) {
        LOGE("%s: failed to connection with PVR services\n", __func__);
        goto srv_err;
    }

    // get device data
    res = PVRSRVEnumerateDevices(pvrConnection, &devNum, devIDs);
    if (res != PVRSRV_OK) {
        LOGE("%s: failed to enumerate devices\n", __func__);
        goto dev_err;
    }

    for (uint32_t i = 0; i < devNum; i++) {
        if (devIDs[i].eDeviceType == PVRSRV_DEVICE_TYPE_SGX) {
            res = PVRSRVAcquireDeviceData(pvrConnection,
                                         devIDs[i].ui32DeviceIndex,
                                         pvr3DDevData,
                                         PVRSRV_DEVICE_TYPE_UNKNOWN);
            if (res != PVRSRV_OK) {
                LOGE("%s: failed to acquire device data\n", __func__);
                goto dev_err;
            }

            // got device data, break;
            break;
        }
    }

    // create memory context
    res = PVRSRVCreateDeviceMemContext(pvr3DDevData,
                                       &mDevMemContext,
                                       &heapCount,
                                       heapInfos);
    if (res != PVRSRV_OK) {
        LOGE("%s: failed to create device memory context\n", __func__);
        goto dev_err;
    }

    for(uint32_t i = 0; i < heapCount; i++) {
        if(HEAP_IDX(heapInfos[i].ui32HeapID) == 0) {
            mGeneralHeap = heapInfos[i].hDevMemHeap;
	    break;
	}
    }

    mPVRSrvConnection = pvrConnection;
    mInitialized = true;
    return true;
dev_err:
    PVRSRVDisconnect(pvrConnection);
srv_err:
    delete mWsbm;
    mWsbm = 0;
    mInitialized = false;
    return false;
}

IntelDisplayBuffer* IntelGraphicBufferManager::map(uint32_t handle)
{
    PVRSRV_ERROR res;
    PVRSRV_CLIENT_MEM_INFO *memInfo;
    IntelDisplayBuffer* buffer;
    bool ret;

    if (!initCheck())
        return 0;

    res = PVRSRVMapDeviceMemory2(&mDevData,
                                handle,
                                mGeneralHeap,
                                PVRSRV_MEM_NO_GPU_ADDR,
                                &memInfo);
    if (res != PVRSRV_OK) {
        LOGE("%s: failed to map meminfo with handle 0x%x, err = %d",
             __func__, handle, res);
        return 0;
    }

    void *vaddr = memInfo->pvLinAddr;
    uint32_t size = memInfo->uAllocSize;
    int gttOffsetInPage;

    // map to gtt
    ret = gttMap(memInfo, 0, &gttOffsetInPage);
    if (!ret) {
        LOGE("%s: failed to map gtt\n", __func__);
        goto gtt_err;
    }

    buffer = new IntelDisplayBuffer(memInfo,
                                    vaddr,
                                    gttOffsetInPage,
                                    size,
                                    handle);
    return buffer;
gtt_err:
    PVRSRVUnmapDeviceMemory(&mDevData, memInfo);
    return 0;
}

void IntelGraphicBufferManager::unmap(IntelDisplayBuffer *buffer)
{
    PVRSRV_CLIENT_MEM_INFO *memInfo;

    if (!buffer)
        return;

    if (!initCheck())
        return;

    memInfo = (PVRSRV_CLIENT_MEM_INFO*)buffer->getBufferObject();

    if (!memInfo)
        return;

    // unmap from gtt
    gttUnmap(memInfo);

    // unmap PVR meminfo
    PVRSRVUnmapDeviceMemory(&mDevData, memInfo);

    // destroy it
    delete buffer;
}

IntelDisplayBuffer* IntelGraphicBufferManager::get(int size, int alignment)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return NULL;
    }

    void *wsbmBufferObject = NULL;
    bool ret = mWsbm->allocateTTMBuffer(size, alignment, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: failed to allocate buffer. size %d, alignment %d\n",
            __func__, size, alignment);
        return NULL;
    }

    void *virtAddr = mWsbm->getCPUAddress(wsbmBufferObject);
    uint32_t gttOffsetInPage = mWsbm->getGttOffset(wsbmBufferObject);
    uint32_t handle = mWsbm->getKBufHandle(wsbmBufferObject);
    // create new buffer
    IntelDisplayBuffer *buffer = new IntelDisplayBuffer(wsbmBufferObject,
                                                        virtAddr,
                                                        gttOffsetInPage,
                                                        size,
                                                        handle);
    LOGD_IF(ALLOW_BUFFER_PRINT,
            "%s: created TTM overlay buffer. cpu %p, gtt %d\n",
         __func__, virtAddr, gttOffsetInPage);
    return buffer;
}

void IntelGraphicBufferManager::put(IntelDisplayBuffer* buf)
{
    if (!buf || !mWsbm) {
        LOGE("%s: Invalid parameter\n", __func__);
        return;
    }

    void *wsbmBufferObject = buf->getBufferObject();
    bool ret = mWsbm->destroyTTMBuffer(wsbmBufferObject);
    if (ret == false)
        LOGW("%s: failed to free wsbmBO\n", __func__);

    // free overlay buffer
    delete buf;
}

IntelDisplayBuffer* IntelGraphicBufferManager::wrap(void *addr, int size)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return 0;
    }

    void *wsbmBufferObject;
    uint32_t handle = (uint32_t)addr;
    bool ret = mWsbm->wrapTTMBuffer(handle, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: wrap ttm buffer failed\n", __func__);
        return 0;
    }

    ret = mWsbm->waitIdleTTMBuffer(wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: wait ttm buffer idle failed\n", __func__);
        return 0;
    }

    void *virtAddr = mWsbm->getCPUAddress(wsbmBufferObject);
    uint32_t gttOffsetInPage = mWsbm->getGttOffset(wsbmBufferObject);

    if (!gttOffsetInPage || !virtAddr) {
        LOGW("GTT offset:%x Virtual addr: %p.", gttOffsetInPage, virtAddr);
        return 0;
    }

    IntelDisplayBuffer *buf = new IntelDisplayBuffer(wsbmBufferObject,
                                                     virtAddr,
                                                     gttOffsetInPage,
                                                     0);
    return buf;
}

void IntelGraphicBufferManager::unwrap(IntelDisplayBuffer *buffer)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return;
    }

    if (!buffer)
        return;

    mWsbm->unreferenceTTMBuffer(buffer->getBufferObject());
    // destroy it
    delete buffer;
}

void IntelGraphicBufferManager::waitIdle(uint32_t khandle)
{
    if (!mWsbm) {
        LOGE("%s: no wsbm found\n", __func__);
        return;
    }

    if (!khandle)
        return;

    void *wsbmBufferObject;;
    bool ret = mWsbm->wrapTTMBuffer(khandle, &wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: wrap ttm buffer failed\n", __func__);
        return;
    }

    ret = mWsbm->waitIdleTTMBuffer(wsbmBufferObject);
    if (ret == false) {
        LOGE("%s: wait ttm buffer idle failed\n", __func__);
        return;
    }

    mWsbm->unreferenceTTMBuffer(wsbmBufferObject);
    return;
}

IntelPayloadBuffer::IntelPayloadBuffer(IntelBufferManager* bufferManager, int bufferFd)
        : mBufferManager(bufferManager), mBuffer(0), mPayload(0)
{
    if (bufferFd <= 0 || !bufferManager)
        return;

    mBuffer = mBufferManager->map(bufferFd);
    if (!mBuffer) {
        LOGE("%s: failed to map payload buffer.\n", __func__);
        return;
    }
    mPayload = mBuffer->getCpuAddr();
}

IntelPayloadBuffer::~IntelPayloadBuffer()
{
    if (mBufferManager && mBuffer) {
        mBufferManager->unmap(mBuffer);
    }
}
