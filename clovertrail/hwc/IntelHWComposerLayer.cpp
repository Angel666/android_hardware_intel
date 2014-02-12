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
#include <IntelHWComposerLayer.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <IntelHWComposerCfg.h>

IntelHWComposerLayer::IntelHWComposerLayer()
    : mHWCLayer(0), mPlane(0), mFlags(0)
{

}

IntelHWComposerLayer::IntelHWComposerLayer(hwc_layer_t *layer,
                                           IntelDisplayPlane *plane,
                                           int flags)
    : mHWCLayer(layer), mPlane(plane), mFlags(flags), mForceOverlay(false),
      mLayerType(0), mFormat(0), mIsProtected(false)
{

}

IntelHWComposerLayer::~IntelHWComposerLayer()
{

}

IntelHWComposerLayerList::IntelHWComposerLayerList(IntelDisplayPlaneManager *pm)
    : mLayerList(0),
      mPlaneManager(pm),
      mNumLayers(0),
      mNumRGBLayers(0),
      mNumYUVLayers(0),
      mAttachedSpritePlanes(0),
      mAttachedOverlayPlanes(0),
      mNumAttachedPlanes(0)
{
    if (!mPlaneManager)
        mInitialized = false;
    else
        mInitialized = true;
}

IntelHWComposerLayerList::~IntelHWComposerLayerList()
{
    if (!initCheck())
        return;

    // delete list
    mPlaneManager = 0;
    delete[] mLayerList;
    mNumLayers = 0;
    mNumRGBLayers = 0;
    mNumYUVLayers= 0;
    mAttachedSpritePlanes = 0;
    mAttachedOverlayPlanes = 0;
    mInitialized = false;
}

void IntelHWComposerLayerList::updateLayerList(hwc_layer_list_t *layerList)
{
    int numLayers;
    int numRGBLayers = 0;
    int numYUVLayers = 0;

    if (!layerList) {
        mNumLayers = 0;
        mNumRGBLayers = 0;
        mNumYUVLayers = 0;
        mAttachedSpritePlanes = 0;
        mAttachedOverlayPlanes = 0;
        mNumAttachedPlanes = 0;

        delete [] mLayerList;
        mLayerList = 0;
        return;
    }

    numLayers = layerList->numHwLayers;

    if (numLayers <= 0 || !initCheck())
        return;

    if (mNumLayers < numLayers) {
        delete [] mLayerList;
        mLayerList = new IntelHWComposerLayer[numLayers];
        if (!mLayerList) {
            LOGE("%s: failed to create layer list\n", __func__);
            return;
        }
    }

    for (int i = 0; i < numLayers; i++) {
        mLayerList[i].mHWCLayer = &layerList->hwLayers[i];
        mLayerList[i].mPlane = 0;
        mLayerList[i].mFlags = 0;
        mLayerList[i].mForceOverlay = false;
        mLayerList[i].mNeedClearup = false;
        mLayerList[i].mLayerType = IntelHWComposerLayer::LAYER_TYPE_INVALID;
        mLayerList[i].mFormat = 0;
        mLayerList[i].mIsProtected = false;

        // update layer format
        intel_gralloc_buffer_handle_t *grallocHandle =
            (intel_gralloc_buffer_handle_t*)layerList->hwLayers[i].handle;

        if (!grallocHandle)
            continue;

        mLayerList[i].mFormat = grallocHandle->format;

        if (grallocHandle->format == HAL_PIXEL_FORMAT_YV12 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_NV12 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_YUY2 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_UYVY ||
            grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_I420) {
            mLayerList[i].mLayerType = IntelHWComposerLayer::LAYER_TYPE_YUV;
            numYUVLayers++;
        } else if (grallocHandle->format == HAL_PIXEL_FORMAT_RGB_565 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_BGRA_8888 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_BGRX_8888 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_RGBX_8888 ||
            grallocHandle->format == HAL_PIXEL_FORMAT_RGBA_8888) {
            mLayerList[i].mLayerType = IntelHWComposerLayer::LAYER_TYPE_RGB;
            numRGBLayers++;
        } else
            LOGW("updateLayerList: unknown format 0x%x", grallocHandle->format);

        // check if a protected layer
        if (grallocHandle->usage & GRALLOC_USAGE_PROTECTED)
            mLayerList[i].mIsProtected = true;
    }

    mNumLayers = numLayers;
    mNumRGBLayers = numRGBLayers;
    mNumYUVLayers = numYUVLayers;
    mNumAttachedPlanes = 0;
}

bool IntelHWComposerLayerList::invalidatePlanes()
{
    if (!initCheck())
        return false;

    for (int i = 0; i < mNumLayers; i++) {
        if (mLayerList[i].mPlane) {
            mPlaneManager->reclaimPlane(mLayerList[i].mPlane);
            mLayerList[i].mPlane = 0;
        }
    }

    mAttachedSpritePlanes = 0;
    mAttachedOverlayPlanes = 0;
    mNumAttachedPlanes = 0;
    return true;
}

void IntelHWComposerLayerList::attachPlane(int index,
                                           IntelDisplayPlane *plane,
                                           int flags)
{
    if (index < 0 || index >= mNumLayers || !plane) {
        LOGE("%s: Invalid parameters\n", __func__);
        return;
    }

    if (initCheck()) {
        mLayerList[index].mPlane = plane;
        mLayerList[index].mFlags = flags;
        if (plane->getPlaneType() == IntelDisplayPlane::DISPLAY_PLANE_SPRITE)
            mAttachedSpritePlanes++;
        else if (plane->getPlaneType() == IntelDisplayPlane::DISPLAY_PLANE_OVERLAY)
            mAttachedOverlayPlanes++;
        mNumAttachedPlanes++;
    }
}

void IntelHWComposerLayerList::detachPlane(int index, IntelDisplayPlane *plane)
{
    if (index < 0 || index >= mNumLayers || !plane) {
        LOGE("%s: Invalid parameters\n", __func__);
        return;
    }

    if (initCheck()) {
        mPlaneManager->reclaimPlane(plane);
        mLayerList[index].mPlane = 0;
        mLayerList[index].mFlags = 0;
        if (plane->getPlaneType() == IntelDisplayPlane::DISPLAY_PLANE_SPRITE)
            mAttachedSpritePlanes--;
        else if (plane->getPlaneType() == IntelDisplayPlane::DISPLAY_PLANE_OVERLAY)
            mAttachedOverlayPlanes--;
        mNumAttachedPlanes--;
    }
}

IntelDisplayPlane* IntelHWComposerLayerList::getPlane(int index)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return 0;
    }

    if (initCheck())
        return mLayerList[index].mPlane;

    return 0;
}

void IntelHWComposerLayerList::setFlags(int index, int flags)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return;
    }

    if (initCheck())
        mLayerList[index].mFlags = flags;
}

int IntelHWComposerLayerList::getFlags(int index)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return 0;
    }

    if (initCheck())
        return mLayerList[index].mFlags;

    return 0;
}

void IntelHWComposerLayerList::setForceOverlay(int index, bool isForceOverlay)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return;
    }

    if (initCheck())
        mLayerList[index].mForceOverlay = isForceOverlay;
}

bool IntelHWComposerLayerList::getForceOverlay(int index)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return false;
    }

    if (initCheck())
        return mLayerList[index].mForceOverlay;

    return false;
}

void IntelHWComposerLayerList::setNeedClearup(int index, bool needClearup)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return;
    }

    if (initCheck())
        mLayerList[index].mNeedClearup = needClearup;
}

bool IntelHWComposerLayerList::getNeedClearup(int index)
{
    if (index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return false;
    }

    if (initCheck())
        return mLayerList[index].mNeedClearup;

    return false;
}

int IntelHWComposerLayerList::getLayerType(int index) const
{
    if (!initCheck() || index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return IntelHWComposerLayer::LAYER_TYPE_INVALID;
    }

    return mLayerList[index].mLayerType;
}

int IntelHWComposerLayerList::getLayerFormat(int index) const
{
    if (!initCheck() || index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return 0;
    }

    return mLayerList[index].mFormat;
}

bool IntelHWComposerLayerList::isProtectedLayer(int index) const
{
    if (!initCheck() || index < 0 || index >= mNumLayers) {
        LOGE("%s: Invalid parameters\n", __func__);
        return false;
    }

    return mLayerList[index].mIsProtected;
}

int IntelHWComposerLayerList::getRGBLayerCount() const
{
    if (!initCheck()) {
        LOGE("%s: Invalid parameters\n", __func__);
        return 0;
    }

    return mNumRGBLayers;
}

int IntelHWComposerLayerList::getYUVLayerCount() const
{
    if (!initCheck()) {
        LOGE("%s: Invalid parameters\n", __func__);
        return 0;
    }

    return mNumYUVLayers;
}

void IntelHWComposerLayerList::clearWithOpenGL() const
{
    drmModeModeInfoPtr mode;
    mode = IntelHWComposerDrm::getInstance().getOutputMode(OUTPUT_MIPI0);

    if (!mode || !mode->hdisplay || !mode->vdisplay) {
        LOGE("%s: failed to detect mode of output OUTPUT_MIPI0\n", __func__);
        return;
    }

    LOGD("%s: clear fb here, size %d x %d", __func__, mode->hdisplay, mode->vdisplay);
    GLfloat vertices[][2] = {
        { 0,  mode->vdisplay },
        { 0,  0 },
        { mode->hdisplay, 0 },
        { mode->hdisplay, mode->vdisplay }
    };

    glColor4f(0, 0, 0, 0);

    glDisable(GL_TEXTURE_EXTERNAL_OES);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
