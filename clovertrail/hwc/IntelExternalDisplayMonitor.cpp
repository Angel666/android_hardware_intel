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
//#define LOG_NDEBUG 0
#include <IntelHWComposerCfg.h>
#include <binder/IServiceManager.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#include <linux/netlink.h>
#include <pthread.h>

#include "IntelExternalDisplayMonitor.h"
#include "IntelHWComposer.h"

using namespace android;

#define UNKNOWN_MDS_MODE 0 // status of video/widi/HDMI/HDCP/MIPI is unknown initially, it can be set indirectly through onMdsMessage event or directly set by invoking mMDClient->getMode

IntelExternalDisplayMonitor::IntelExternalDisplayMonitor(IntelHWComposer *hwc) :
    mMDClient(NULL),
    mActiveDisplayMode(UNKNOWN_MDS_MODE),
    mWidiOn(false),
    mInitialized(false),
    mComposer(hwc)
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor created");
    initialize();
}

IntelExternalDisplayMonitor::~IntelExternalDisplayMonitor()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor Destroyed");
    mInitialized = false;
    mComposer = 0;
}

void IntelExternalDisplayMonitor::initialize()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor initialized");
    mInitialized = true;
}

void IntelExternalDisplayMonitor::onMdsMessage(int msg, int data)
{
    LOGI("onMdsMessage: External display monitor onMdsMessage, %d, 0x%x", msg, data);
    if (msg == MDS_ORIENTATION_CHANGE) {
        LOGV("onMdsMessage: MDS_ORIENTATION_CHANGE received");
        if (mWidiOn)
            mComposer->onUEvent(mUeventMessage, UEVENT_MSG_LEN - 2, MSG_TYPE_MDS_ORIENTATION_CHANGE);
    } else {
        mActiveDisplayMode = data;
        mComposer->onUEvent(mUeventMessage, UEVENT_MSG_LEN - 2, MSG_TYPE_MDS);
    }
}

int IntelExternalDisplayMonitor::getDisplayMode()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "Get display mode %d", mActiveDisplayMode);
    if (mActiveDisplayMode & MDS_HDMI_VIDEO_EXT)
        return OVERLAY_EXTEND;
    else if (mActiveDisplayMode & MDS_HDMI_CLONE)
        return OVERLAY_CLONE_MIPI0;
    else
        return OVERLAY_MIPI0;
}

bool IntelExternalDisplayMonitor::isVideoPlaying()
{
    if (mActiveDisplayMode & MDS_VIDEO_PLAYING)
       return true;
    return false;
}

bool IntelExternalDisplayMonitor::isOverlayOff()
{
    if (mActiveDisplayMode & MDS_OVERLAY_OFF) {
       LOGW("Overlay is off.");
       return true;
    }
    return false;
}

void IntelExternalDisplayMonitor::binderDied(const wp<IBinder>& who)
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor binderDied");
}

bool IntelExternalDisplayMonitor::notifyWidi(bool on)
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "Exteranal display notify the MDS widi's state");
    // TODO: remove mWideOn. MultiDisplay Service maintains the state machine.
    if ((mMDClient != NULL) && (mWidiOn != on)) {
        mWidiOn = on;
        return mMDClient->notifyWidi(on);
    }
    return false;
}


bool IntelExternalDisplayMonitor::notifyMipi(bool on)
{
    LOGD_IF(ALLOW_MONITOR_PRINT,
            "Exteranal display notify the MDS that Mipi should be turned on/off");
    if ((mMDClient != NULL) &&
        ((mActiveDisplayMode & MDS_HDMI_VIDEO_EXT) ||
         (mWidiOn))) {
        return mMDClient->notifyMipi(on);
    }
    return false;
}

bool IntelExternalDisplayMonitor::getVideoInfo(int *displayW, int *displayH, int *fps, int *isinterlace)
{
    if (mMDClient != NULL) {
        int ret = mMDClient->getVideoInfo(displayW, displayH, fps, isinterlace);
        return (ret == MDS_NO_ERROR);
    }
    return false;
}

bool IntelExternalDisplayMonitor::threadLoop()
{
    //LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor thread loop");

    if (mMDClient !=  0) {
        LOGD_IF(ALLOW_MONITOR_PRINT, "threadLoop: found MDS, threadLoop will exit.");
        requestExit();
        return true;
    }

    // if no MDS service available, fall back uevent
    struct pollfd fds;
    int nr;

    fds.fd = mUeventFd;
    fds.events = POLLIN;
    fds.revents = 0;
    nr = poll(&fds, 1, -1);

    if(nr > 0 && fds.revents == POLLIN) {
        int count = recv(mUeventFd, mUeventMessage, UEVENT_MSG_LEN - 2, 0);
        if (count > 0)
            mComposer->onUEvent(mUeventMessage,
                                UEVENT_MSG_LEN - 2,
                                MSG_TYPE_UEVENT);
    }

    return true;
}

status_t IntelExternalDisplayMonitor::readyToRun()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor ready to run");

    // get multi-display manager service, retry 10 seconds
    int retry = 2;
    do {
        const String16 name("MultiDisplay");
        sp<IBinder> service;
        sp<IServiceManager> serviceManager = defaultServiceManager();
        service = serviceManager->getService(name);
        if (service != 0) {
            mMDClient = new MultiDisplayClient();
            if (mMDClient == NULL) {
                LOGE("Fail to create a multidisplay client");
            } else {
                LOGI("Create a MultiDisplay client at HWC");
                bool bWait = true;
                mActiveDisplayMode = mMDClient->getMode(bWait);
                if (getDisplayMode() == OVERLAY_CLONE_MIPI0)
                    IntelHWComposerDrm::getInstance().setDisplayMode(OVERLAY_CLONE_MIPI0);
                else if (getDisplayMode() == OVERLAY_EXTEND) {
                    IntelHWComposerDrm::getInstance().setDisplayMode(OVERLAY_EXTEND);
                    LOGE("Impossible be here. Only clone mode or single mode is valid initialized state.");
                }
                else
                    IntelHWComposerDrm::getInstance().setDisplayMode(OVERLAY_MIPI0);
            }
            service->linkToDeath(this);
            break;
        }
        LOGW("Failed to get MDS service.try again...\n");
    } while(--retry);

    if (!retry && mMDClient == NULL) {
        LOGW("Failed to get mds service, fall back uevent\n");
        struct sockaddr_nl addr;
        int sz = 64*1024;

        memset(&addr, 0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_pid =  pthread_self() | getpid();
        addr.nl_groups = 0xffffffff;

        mUeventFd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
        if(mUeventFd < 0) {
            LOGD("%s: failed to open uevent socket\n", __func__);
            return TIMED_OUT;
        }

        setsockopt(mUeventFd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

        if(bind(mUeventFd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            close(mUeventFd);
            return TIMED_OUT;
        }

        memset(mUeventMessage, 0, UEVENT_MSG_LEN);
    } else {
        LOGI("Got MultiDisplay Service\n");
        if (mMDClient != NULL)
            mMDClient->registerModeChangeListener(this);
    }

    return NO_ERROR;
}

void IntelExternalDisplayMonitor::onFirstRef()
{
    LOGD_IF(ALLOW_MONITOR_PRINT, "External display monitor onFirstRef");
    run("HWC external display monitor", PRIORITY_URGENT_DISPLAY);
}
