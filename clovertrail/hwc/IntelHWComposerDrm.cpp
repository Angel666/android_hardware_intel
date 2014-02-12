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

#ifdef TARGET_HAS_MULTIPLE_DISPLAY
#include "MultiDisplayType.h"
#endif

IntelHWComposerDrm *IntelHWComposerDrm::mInstance(0);

IntelHWComposerDrm:: ~IntelHWComposerDrm() {
    LOGD_IF(ALLOW_MONITOR_PRINT, "%s: destroying overlay HAL...\n", __func__);

    drmDestroy();
    mInstance = NULL;
}

bool IntelHWComposerDrm::drmInit()
{
    int fd = open("/dev/card0", O_RDWR, 0);
    if (fd < 0) {
        LOGE("%s: drmOpen failed. %s\n", __func__, strerror(errno));
        return false;
    }

    mDrmFd = fd;
    LOGD("%s: successfully. mDrmFd %d\n", __func__, fd);
    return true;
}

void IntelHWComposerDrm::drmDestroy()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "%s: destroying...\n", __func__);

    if(mDrmFd > 0) {
        drmClose(mDrmFd);
        mDrmFd = -1;
    }
}

void IntelHWComposerDrm::setOutputConnection(const int output,
                                             drmModeConnection connection)
{
    if (output < 0 || output >= OUTPUT_MAX)
        return;

    mDrmOutputsState.connections[output] = connection;
}

drmModeConnection
IntelHWComposerDrm:: getOutputConnection(const int output)
{
    drmModeConnection connection = DRM_MODE_DISCONNECTED;

    if (output < 0 || output >= OUTPUT_MAX)
        return connection;

    connection = mDrmOutputsState.connections[output];

    return connection;
}

void IntelHWComposerDrm::setOutputMode(const int output,
                                       drmModeModeInfoPtr mode,
                                       int valid)
{
    if (output < 0 || output >= OUTPUT_MAX || !mode)
        return;

    if (valid) {
        memcpy(&mDrmOutputsState.modes[output],
               mode, sizeof(drmModeModeInfo));
        mDrmOutputsState.mode_valid[output] = true;
    } else
        mDrmOutputsState.mode_valid[output] = false;
}

drmModeModeInfoPtr IntelHWComposerDrm::getOutputMode(const int output)
{
    if (output < 0 || output >= OUTPUT_MAX)
        return 0;

    return &mDrmOutputsState.modes[output];
}

void IntelHWComposerDrm::setOutputFBInfo(const int output,
                                       drmModeFBPtr fbInfo)
{
    if (output < 0 || output >= OUTPUT_MAX || !fbInfo)
        return;

    memcpy(&mDrmOutputsState.fbInfos[output], fbInfo, sizeof(drmModeFB));
}

drmModeFBPtr IntelHWComposerDrm::getOutputFBInfo(const int output)
{
    if (output < 0 || output >= OUTPUT_MAX)
        return 0;

    return &mDrmOutputsState.fbInfos[output];
}

bool IntelHWComposerDrm::isValidOutputMode(const int output)
{
    if (output < 0 || output >= OUTPUT_MAX)
        return false;

    return mDrmOutputsState.mode_valid[output];
}

void IntelHWComposerDrm::setDisplayMode(intel_overlay_mode_t displayMode)
{
    mDrmOutputsState.old_display_mode = mDrmOutputsState.display_mode;
    mDrmOutputsState.display_mode = displayMode;
}

intel_overlay_mode_t IntelHWComposerDrm::getDisplayMode()
{
    return mDrmOutputsState.display_mode;
}

bool IntelHWComposerDrm::isVideoPlaying()
{
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    if (mMonitor != NULL)
        return mMonitor->isVideoPlaying();
#endif
    return false;
}

bool IntelHWComposerDrm::isOverlayOff()
{
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    if (mMonitor != NULL)
        return mMonitor->isOverlayOff();
#endif
    return false;
}

bool IntelHWComposerDrm::notifyWidi(bool on)
{
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    if (mMonitor != NULL)
        return mMonitor->notifyWidi(on);
#endif
    return false;
}

bool IntelHWComposerDrm::notifyMipi(bool on)
{
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    if (mMonitor != NULL)
        return mMonitor->notifyMipi(on);
#endif
    return false;
}

bool IntelHWComposerDrm::getVideoInfo(int *displayW, int *displayH, int *fps, int *isinterlace)
{
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    if (mMonitor != NULL)
        return mMonitor->getVideoInfo(displayW, displayH, fps, isinterlace);
#endif
    return false;
}

intel_overlay_mode_t IntelHWComposerDrm::getOldDisplayMode()
{
    return mDrmOutputsState.old_display_mode;
}

bool IntelHWComposerDrm::initialize(IntelHWComposer *hwc)
{
    bool ret = false;

    LOGD_IF(ALLOW_MONITOR_PRINT, "%s: init...\n", __func__);

    if (mDrmFd < 0) {
        ret = drmInit();
        if(ret == false) {
            LOGE("%s: drmInit failed\n", __func__);
            return ret;
        }
    }

#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    // create external display monitor
    mMonitor = new IntelExternalDisplayMonitor(hwc);
    if (mMonitor == 0) {
        LOGE("%s: failed to create external display monitor\n", __func__);
        drmDestroy();
        return false;
    }
#endif

    // detect display mode
    ret = detectDrmModeInfo();
    if (ret == false)
        LOGW("%s: failed to detect DRM modes\n", __func__);
    else
        LOGD_IF(ALLOW_MONITOR_PRINT, "%s: finish successfully.\n", __func__);

    // set old display mode the same detect mode
    mDrmOutputsState.old_display_mode =  mDrmOutputsState.display_mode;
    return true;
}
bool IntelHWComposerDrm::detectDrmModeInfo()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "%s: detecting drm mode info...\n", __func__);

    if (mDrmFd < 0) {
        LOGE("%s: invalid drm FD\n", __func__);
        return false;
    }

    /*try to get drm resources*/
    drmModeResPtr resources = drmModeGetResources(mDrmFd);
    if (!resources) {
        LOGE("%s: fail to get drm resources. %s\n", __func__, strerror(errno));
        return false;
    }

    /*get mipi0 info*/
    drmModeConnectorPtr connector = NULL;
    drmModeEncoderPtr encoder = NULL;
    drmModeCrtcPtr crtc = NULL;
    drmModeConnectorPtr connectors[OUTPUT_MAX];
    drmModeModeInfoPtr mode = NULL;
    drmModeFBPtr fbInfo = NULL;

    for (int i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(mDrmFd, resources->connectors[i]);
        if (!connector) {
            LOGW("%s: fail to get drm connector\n", __func__);
            continue;
        }

        int outputIndex = -1;
        if (connector->connector_type == DRM_MODE_CONNECTOR_MIPI ||
            connector->connector_type == DRM_MODE_CONNECTOR_LVDS) {
            LOGD_IF(ALLOW_MONITOR_PRINT, "%s: got MIPI/LVDS connector\n", __func__);
            if (connector->connector_type_id == 1)
                outputIndex = OUTPUT_MIPI0;
            else if (connector->connector_type_id == 2)
                outputIndex = OUTPUT_MIPI1;
            else {
                LOGW("%s: unknown connector type\n", __func__);
                outputIndex = OUTPUT_MIPI0;
            }
        } else if (connector->connector_type == DRM_MODE_CONNECTOR_DVID) {
            LOGD_IF(ALLOW_MONITOR_PRINT, "%s: got HDMI connector\n", __func__);
            outputIndex = OUTPUT_HDMI;
        }

        /*update connection status*/
        setOutputConnection(outputIndex, connector->connection);

        /*get related encoder*/
        encoder = drmModeGetEncoder(mDrmFd, connector->encoder_id);
        if (!encoder) {
            LOGD_IF(ALLOW_MONITOR_PRINT, "%s: fail to get drm encoder\n", __func__);
            drmModeFreeConnector(connector);
            setOutputConnection(outputIndex, DRM_MODE_DISCONNECTED);
            continue;
        }

        /*get related crtc*/
        crtc = drmModeGetCrtc(mDrmFd, encoder->crtc_id);
        if (!crtc) {
            LOGD_IF(ALLOW_MONITOR_PRINT, "%s: fail to get drm crtc\n", __func__);
            drmModeFreeEncoder(encoder);
            drmModeFreeConnector(connector);
            setOutputConnection(outputIndex, DRM_MODE_DISCONNECTED);
            continue;
        }

        /*set crtc mode*/
        setOutputMode(outputIndex, &crtc->mode, crtc->mode_valid);

        // get fb info
        fbInfo = drmModeGetFB(mDrmFd, crtc->buffer_id);
        if (!fbInfo) {
            LOGD("%s: fail to get fb info\n", __func__);
            drmModeFreeCrtc(crtc);
            drmModeFreeEncoder(encoder);
            drmModeFreeConnector(connector);
            setOutputConnection(outputIndex, DRM_MODE_DISCONNECTED);
            continue;
        }

        setOutputFBInfo(outputIndex, fbInfo);

        /*free all crtc/connector/encoder*/
        drmModeFreeFB(fbInfo);
        drmModeFreeCrtc(crtc);
        drmModeFreeEncoder(encoder);
        drmModeFreeConnector(connector);
    }

    drmModeFreeResources(resources);

    drmModeConnection mipi0 = getOutputConnection(OUTPUT_MIPI0);
    drmModeConnection mipi1 = getOutputConnection(OUTPUT_MIPI1);
    drmModeConnection hdmi = getOutputConnection(OUTPUT_HDMI);

#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    int mdsMode = 0;
    if (mMonitor != 0) {
        mdsMode = mMonitor->getDisplayMode();
        LOGD("%s: getDisplayMode %d", __func__, mdsMode);
        //TODO: overlay only support OVERLAY_EXTEND and OVERLAY_MIPI0
        if (mdsMode == OVERLAY_EXTEND && hdmi == DRM_MODE_CONNECTED)
            setDisplayMode(OVERLAY_EXTEND);
        else if (mdsMode == OVERLAY_CLONE_MIPI0)
            setDisplayMode(OVERLAY_CLONE_MIPI0);
        else
            setDisplayMode(OVERLAY_MIPI0);
    } else
#endif
    {
        setDisplayMode(OVERLAY_MIPI0);
    }

    LOGD_IF(ALLOW_MONITOR_PRINT,
           "%s: mipi/lvds %s, mipi1 %s, hdmi %s, displayMode %d\n",
        __func__,
        ((mipi0 == DRM_MODE_CONNECTED) ? "connected" : "disconnected"),
        ((mipi1 == DRM_MODE_CONNECTED) ? "connected" : "disconnected"),
        ((hdmi == DRM_MODE_CONNECTED) ? "connected" : "disconnected"),
        getDisplayMode());

    return true;
}
