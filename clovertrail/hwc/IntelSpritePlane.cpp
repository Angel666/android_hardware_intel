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
#include <IntelDisplayPlaneManager.h>
#include <IntelOverlayUtil.h>

IntelSpriteContext::~IntelSpriteContext()
{

}

IntelSpritePlane::IntelSpritePlane(int fd, int index, IntelBufferManager *bm)
    : IntelDisplayPlane(fd, IntelDisplayPlane::DISPLAY_PLANE_SPRITE, index, bm)
{
    bool ret;
    LOGD_IF(ALLOW_SPRITE_PRINT, "%s\n", __func__);

    // create data buffer
    IntelDisplayBuffer *dataBuffer = new IntelDisplayDataBuffer(0, 0, 0);
    if (!dataBuffer) {
        LOGE("%s: Failed to create sprite data buffer\n", __func__);
        return;
    }

    // create sprite context
    IntelSpriteContext *spriteContext = new IntelSpriteContext();
    if (!spriteContext) {
        LOGE("%s: Failed to create sprite context\n", __func__);
        goto sprite_create_err;
    }

    // initialized successfully
    mDataBuffer = dataBuffer;
    mContext = spriteContext;
    mInitialized = true;
    return;
sprite_create_err:
    delete dataBuffer;
}

IntelSpritePlane::~IntelSpritePlane()
{
    if (mContext) {
	// flush context
        //flip(IntelDisplayPlane::FLASH_NEEDED);

        // disable sprite
        disable();

        // delete sprite context;
        delete mContext;

        // destroy overlay data buffer;
        delete mDataBuffer;

        mContext = 0;
        mInitialized = false;
    }
}

void IntelSpritePlane::setPosition(int left, int top, int right, int bottom)
{
    if (initCheck()) {
        // TODO: check position

	int x = left;
        int y = top;
        int w = right - left;
        int h = bottom - top;

        IntelSpriteContext *spriteContext =
            reinterpret_cast<IntelSpriteContext*>(mContext);
        intel_sprite_context_t *context = spriteContext->getContext();

        // update dst position
        context->pos = (y & 0xfff) << 16 | (x & 0xfff);
        context->size = ((h - 1) & 0xfff) << 16 | ((w - 1) & 0xfff);
    }
}

bool IntelSpritePlane::setDataBuffer(IntelDisplayBuffer& buffer)
{
    if (initCheck()) {
        IntelDisplayBuffer *bufferPtr = &buffer;
        IntelDisplayDataBuffer *spriteDataBuffer =
	            reinterpret_cast<IntelDisplayDataBuffer*>(bufferPtr);
        IntelSpriteContext *spriteContext =
            reinterpret_cast<IntelSpriteContext*>(mContext);
        intel_sprite_context_t *context = spriteContext->getContext();

        uint32_t format = spriteDataBuffer->getFormat();
        uint32_t spriteFormat;
        int bpp;

        switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            spriteFormat = INTEL_SPRITE_PIXEL_FORMAT_RGBA8888;
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            spriteFormat = INTEL_SPRITE_PIXEL_FORMAT_RGBX8888;
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            spriteFormat = INTEL_SPRITE_PIXEL_FORMAT_BGRA8888;
            bpp = 4;
            break;
        default:
            LOGE("%s: unsupported format 0x%x\n", __func__, format);
            return false;
        }

        // set offset;
        int srcX = spriteDataBuffer->getSrcX();
        int srcY = spriteDataBuffer->getSrcY();
        int srcWidth = spriteDataBuffer->getSrcWidth();
        int srcHeight = spriteDataBuffer->getSrcHeight();
        uint32_t stride = align_to(bpp * srcWidth, 64);
        uint32_t linoff = srcY * stride + srcX * bpp;

        // gtt
        uint32_t gttOffsetInPage = spriteDataBuffer->getGttOffsetInPage();

        // update context
        context->cntr = spriteFormat | 0x80000000;
        context->linoff = linoff;
        context->stride = stride;
        context->surf = gttOffsetInPage << 12;
        context->update_mask = SPRITE_UPDATE_ALL;

        return true;
    }
    LOGE("%s: sprite plane was not initialized\n", __func__);
    return false;
}

bool IntelSpritePlane::flip(void *context, uint32_t flags)
{
    return true;
}

bool IntelSpritePlane::reset()
{
    return true;
}

bool IntelSpritePlane::disable()
{
    return true;
}

bool IntelSpritePlane::invalidateDataBuffer()
{
    LOGD_IF(ALLOW_SPRITE_PRINT, "%s\n", __func__);
    if (initCheck()) {
	 mBufferManager->unmap(mDataBuffer);
	 delete mDataBuffer;
	 mDataBuffer = new IntelDisplayDataBuffer(0, 0, 0);
	 return true;
    }

    return false;
}
