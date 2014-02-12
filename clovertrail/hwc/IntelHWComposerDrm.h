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
#ifndef __INTEL_HWCOMPOSER_DRM_H__
#define __INTEL_HWCOMPOSER_DRM_H__

#include <IntelBufferManager.h>
#include <IntelHWCUEventObserver.h>
#include <psb_drm.h>
#include <pthread.h>
#include <pvr2d.h>
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
#include <IntelExternalDisplayMonitor.h>
#endif

extern "C" {
#include "xf86drm.h"
#include "xf86drmMode.h"
}

#define PVR_DRM_DRIVER_NAME     "pvrsrvkm"

#define DRM_PSB_GTT_MAP         0x0F
#define DRM_PSB_GTT_UNMAP       0x10

#define DRM_MODE_CONNECTOR_MIPI 15

typedef enum {
    PVR_OVERLAY_VSYNC_INIT,
    PVR_OVERLAY_VSYNC_DONE,
    PVR_OVERLAY_VSYNC_PENDING,
} eVsyncState;

typedef enum {
    OUTPUT_MIPI0 = 0,
    OUTPUT_HDMI,
    OUTPUT_MIPI1,
    OUTPUT_MAX,
} intel_drm_output_t;

typedef enum {
    OVERLAY_MIPI0 = 0,
    OVERLAY_CLONE_MIPI0,
    OVERLAY_CLONE_MIPI1,
    OVERLAY_CLONE_DUAL,
    OVERLAY_EXTEND,
    OVERLAY_UNKNOWN,
} intel_overlay_mode_t;

typedef struct {
    drmModeConnection connections[OUTPUT_MAX];
    drmModeModeInfo modes[OUTPUT_MAX];
    drmModeFB fbInfos[OUTPUT_MAX];
    bool mode_valid[OUTPUT_MAX];
    intel_overlay_mode_t display_mode;
    intel_overlay_mode_t old_display_mode;
} intel_drm_output_state_t;

class IntelHWComposer;

/**
 * Class: Overlay HAL implementation
 * This is a singleton implementation of hardware overlay.
 * this object will be shared between mutiple overlay control/data devices.
 * FIXME: overlayHAL should contact to the h/w to track the overlay h/w
 * state.
 */
class IntelHWComposerDrm {
private:
    int mDrmFd;
    intel_drm_output_state_t mDrmOutputsState;
    static IntelHWComposerDrm *mInstance;
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    android::sp<IntelExternalDisplayMonitor> mMonitor;
#endif
private:
    IntelHWComposerDrm()
        : mDrmFd(-1)
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
        , mMonitor(0)
#endif
    {
        memset(&mDrmOutputsState, 0, sizeof(intel_drm_output_state_t));
    }
    IntelHWComposerDrm(const IntelHWComposerDrm&);
    bool drmInit();
    void drmDestroy();
public:
    ~IntelHWComposerDrm();
    static IntelHWComposerDrm& getInstance() {
        IntelHWComposerDrm *instance = mInstance;
        if (instance == 0) {
            instance = new IntelHWComposerDrm();
            mInstance = instance;
        }
        return *instance;
    }
    bool initialize(IntelHWComposer *hwc);
    bool detectDrmModeInfo();
    int getDrmFd() const { return mDrmFd; }

    // DRM output states
    void setOutputConnection(const int output, drmModeConnection connection);
    drmModeConnection getOutputConnection(const int output);
    void setOutputMode(const int output, drmModeModeInfoPtr mode, int valid);
    drmModeModeInfoPtr getOutputMode(const int output);
    void setOutputFBInfo(const int output, drmModeFBPtr fbInfo);
    drmModeFBPtr getOutputFBInfo(const int output);
    bool isValidOutputMode(const int output);
    void setDisplayMode(intel_overlay_mode_t displayMode);
    intel_overlay_mode_t getDisplayMode();
    intel_overlay_mode_t getOldDisplayMode();
    bool isVideoPlaying();
    bool isOverlayOff();
    bool notifyMipi(bool);
    bool notifyWidi(bool);
    bool getVideoInfo(int *displayW, int *displayH, int *fps, int *isinterlace);
};

#endif /*__INTEL_HWCOMPOSER_DRM_H__*/
