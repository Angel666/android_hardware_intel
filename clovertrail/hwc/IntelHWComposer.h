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
#ifndef __INTEL_HWCOMPOSER_CPP__
#define __INTEL_HWCOMPOSER_CPP__

#include <EGL/egl.h>
#include <hardware/hwcomposer.h>

#include <IntelHWComposerDrm.h>
#include <IntelBufferManager.h>
#include <IntelHWComposerLayer.h>
#include <IntelHWComposerDump.h>
#include <IntelVsyncEventHandler.h>
#include <IntelFakeVsyncEvent.h>
#ifdef INTEL_RGB_OVERLAY
#include <IntelHWCWrapper.h>
#endif

class IntelHWComposer : public hwc_composer_device_t, public IntelHWCUEventObserver, public IntelHWComposerDump  {
public:
    enum {
        VSYNC_SRC_MIPI = 0,
        VSYNC_SRC_HDMI,
        VSYNC_SRC_FAKE,
        VSYNC_SRC_NUM,
    };
private:
    IntelHWComposerDrm *mDrm;
    IntelBufferManager *mBufferManager;
    IntelBufferManager *mGrallocBufferManager;
    IntelDisplayPlaneManager *mPlaneManager;
    IntelHWComposerLayerList *mLayerList;
    hwc_procs_t const *mProcs;
    android::sp<IntelVsyncEventHandler> mVsync;
    android::sp<IntelFakeVsyncEvent> mFakeVsync;
    nsecs_t mLastVsync;
    int mMonitoringMethod;
    bool mForceSwapBuffer;
    bool mHotplugEvent;
    android::Mutex mLock;
    IMG_framebuffer_device_public_t *mFBDev;
    bool mInitialized;
    uint32_t mActiveVsyncs;
    uint32_t mVsyncsEnabled;
#ifdef INTEL_RGB_OVERLAY
    IntelHWCWrapper mWrapper;
#endif
private:
    void dumpLayerList(hwc_layer_list_t *list);
    void onGeometryChanged(hwc_layer_list_t *list);
    bool overlayPrepare(int index, hwc_layer_t *layer, int flags);
    bool spritePrepare(int index, hwc_layer_t *layer, int flags);
    bool primaryPrepare(int index, hwc_layer_t *layer, int flags);
    bool isOverlayLayer(hwc_layer_list_t *list,
                        int index,
                        hwc_layer_t *layer,
                        int& flags);
    bool isSpriteLayer(hwc_layer_list_t *list,
                       int index,
                       hwc_layer_t *layer,
                       int& flags);
    bool isPrimaryLayer(hwc_layer_list_t *list,
                       int index,
                       hwc_layer_t *layer,
                       int& flags);
    bool isScreenshotActive(hwc_layer_list_t *list);
    void revisitLayerList(hwc_layer_list_t *list, bool isGeometryChanged);
    bool useOverlayRotation(hwc_layer_t *layer, int index, uint32_t& handle,
                           int& w, int& h,
                           int& srcX, int& srcY, int& srcW, int& srcH, uint32_t& transform);
    bool updateLayersData(hwc_layer_list_t *list);
    bool isHWCUsage(int usage);
    bool isHWCFormat(int format);
    bool isHWCTransform(uint32_t transform);
    bool isHWCBlending(uint32_t blending);
    bool isHWCLayer(hwc_layer_t *layer);
    bool isBobDeinterlace(hwc_layer_t *layer);
    bool isForceOverlay(hwc_layer_t *layer);
    bool areLayersIntersecting(hwc_layer_t *top, hwc_layer_t* bottom);
    void handleHotplugEvent();
    uint32_t disableUnusedVsyncs(uint32_t target);
    uint32_t enableVsyncs(uint32_t target);
public:
    void onUEvent(const char *msg, int msgLen, int msgType);
    bool flipFramebufferContexts(void *contexts);
    void vsync(int64_t timestamp, int pipe);
public:
    bool initCheck() { return mInitialized; }
    bool initialize();
    bool prepare(hwc_layer_list_t *list);
    bool commit(hwc_display_t dpy, hwc_surface_t sur, hwc_layer_list_t *list);
    bool vsyncControl(int enabled);
    bool release();
    bool dump(char *buff, int buff_len, int *cur_len);
    void registerProcs(hwc_procs_t const *procs) { mProcs = procs; }
#ifdef INTEL_RGB_OVERLAY
    IntelHWCWrapper* getWrapper() { return &mWrapper; }
#endif

    IntelHWComposer()
        : IntelHWCUEventObserver(), IntelHWComposerDump(),
          mDrm(0), mBufferManager(0), mGrallocBufferManager(0),
          mPlaneManager(0), mLayerList(0), mProcs(0), mVsync(0), mFakeVsync(0),
          mLastVsync(0), mMonitoringMethod(0), mForceSwapBuffer(false),
          mHotplugEvent(false), mInitialized(false), mActiveVsyncs(0) {}
    ~IntelHWComposer();
};

#endif /*__INTEL_HWCOMPOSER_CPP__*/
