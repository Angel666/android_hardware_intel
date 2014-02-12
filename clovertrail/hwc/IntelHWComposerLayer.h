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
#ifndef __INTEL_HWCOMPOSER_LAYER_H__
#define __INTEL_HWCOMPOSER_LAYER_H__

#include <string.h>
#include <hardware/hwcomposer.h>
#include <IntelDisplayPlaneManager.h>

class IntelHWComposerLayer {
public:
    enum {
        LAYER_TYPE_INVALID,
        LAYER_TYPE_RGB,
        LAYER_TYPE_YUV,
    };

private:
    hwc_layer_t *mHWCLayer;
    IntelDisplayPlane *mPlane;
    int mFlags;
    bool mForceOverlay;
    bool mNeedClearup;

    // layer info
    int mLayerType;
    int mFormat;
    bool mIsProtected;
public:
    IntelHWComposerLayer();
    IntelHWComposerLayer(hwc_layer_t *layer,
                         IntelDisplayPlane *plane,
                         int flags);
    ~IntelHWComposerLayer();

friend class IntelHWComposerLayerList;
};

class IntelHWComposerLayerList {
private:
    IntelHWComposerLayer *mLayerList;
    IntelDisplayPlaneManager *mPlaneManager;
    int mNumLayers;
    int mNumRGBLayers;
    int mNumYUVLayers;
    int mAttachedSpritePlanes;
    int mAttachedOverlayPlanes;
    int mNumAttachedPlanes;
    bool mInitialized;
public:
    IntelHWComposerLayerList(IntelDisplayPlaneManager *pm);
    ~IntelHWComposerLayerList();
    bool initCheck() const { return mInitialized; }
    void updateLayerList(hwc_layer_list_t *layerList);
    bool invalidatePlanes();
    void attachPlane(int index, IntelDisplayPlane *plane, int flags);
    void detachPlane(int index, IntelDisplayPlane *plane);
    IntelDisplayPlane* getPlane(int index);
    void setFlags(int index, int flags);
    int getFlags(int index);
    void setForceOverlay(int index, bool isForceOverlay);
    bool getForceOverlay(int index);
    void setNeedClearup(int index, bool needClearup);
    bool getNeedClearup(int index);
    int getLayerType(int index) const;
    int getLayerFormat(int index) const;
    bool isProtectedLayer(int index) const;
    int getLayersCount() const { return mNumLayers; }
    int getRGBLayerCount() const;
    int getYUVLayerCount() const;
    int getAttachedPlanesCount() const { return mNumAttachedPlanes; }
    int getAttachedSpriteCount() const { return mAttachedSpritePlanes; }
    int getAttachedOverlayCount() const { return mAttachedOverlayPlanes; }
    void clearWithOpenGL() const;
};

#endif /*__INTEL_HWCOMPOSER_LAYER_H__*/
