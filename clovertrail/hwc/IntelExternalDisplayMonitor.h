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
#ifndef __INTEL_EXTERNAL_DISPLAY_MONITOR_H__
#define __INTEL_EXTERNAL_DISPLAY_MONITOR_H__

#include <utils/threads.h>

#include "display/IExtendDisplayModeChangeListener.h"
#include "display/IMultiDisplayComposer.h"
#include "display/MultiDisplayClient.h"
#include "display/MultiDisplayType.h"

class IntelHWComposer;

class IntelExternalDisplayMonitor :
    public android::BnExtendDisplayModeChangeListener,
    public android::IBinder::DeathRecipient,
    protected android::Thread
{
public:
    enum {
        MSG_TYPE_UEVENT = 0,
        MSG_TYPE_MDS,
        MSG_TYPE_MDS_ORIENTATION_CHANGE,
    };

    enum {
        UEVENT_MSG_LEN = 4096,
    };

public:
    IntelExternalDisplayMonitor(IntelHWComposer *hwc);
    virtual ~IntelExternalDisplayMonitor();
    void initialize();
public:
    // onMdsMessage() interface
    void onMdsMessage(int msg, int data);
public:
    int getDisplayMode();
    bool isVideoPlaying();
    bool isOverlayOff();
    bool notifyWidi(bool);
    bool notifyMipi(bool);
    bool getVideoInfo(int *displayW, int *displayH, int *fps, int *isinterlace);
private:
    //DeathReipient interface
    virtual void binderDied(const android::wp<android::IBinder>& who);
private:
    virtual bool threadLoop();
    virtual android::status_t readyToRun();
    virtual void onFirstRef();
private:
    MultiDisplayClient* mMDClient;
    android::Mutex mLock;
    android::Condition mModeChanged;
    int mActiveDisplayMode;
    bool mWidiOn;
    bool mInitialized;
    IntelHWComposer *mComposer;
    char mUeventMessage[UEVENT_MSG_LEN];
    int mUeventFd;
}; // IntelExternalDisplayMonitor

#endif // __INTEL_EXTERNAL_DISPLAY_MONITOR_H__

