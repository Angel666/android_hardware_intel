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
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#include <linux/netlink.h>
#include "IntelHWComposer.h"
#include "IntelFakeVsyncEvent.h"

IntelFakeVsyncEvent::IntelFakeVsyncEvent(IntelHWComposer *hwc) :
    mEnabled(false), mComposer(hwc), mNextFakeVSync(0)
{
    LOGV("Fake vsync event created");
    mRefreshPeriod = nsecs_t(1e9 / 60);
}

IntelFakeVsyncEvent::~IntelFakeVsyncEvent()
{

}

void IntelFakeVsyncEvent::setEnabled(bool enabled, nsecs_t lastVsync)
{
    android::Mutex::Autolock _l(mLock);
    mNextFakeVSync = lastVsync + mRefreshPeriod;
    mEnabled = enabled;
    mCondition.signal();
}

bool IntelFakeVsyncEvent::threadLoop()
{
    { // scope for lock
        android::Mutex::Autolock _l(mLock);
        while (!mEnabled) {
            mCondition.wait(mLock);
        }
    }

    const nsecs_t period = mRefreshPeriod;
    const nsecs_t now = systemTime(CLOCK_MONOTONIC);
    nsecs_t next_vsync = mNextFakeVSync;
    nsecs_t sleep = next_vsync - now;
    if (sleep < 0) {
        // we missed, find where the next vsync should be
        sleep = (period - ((now - next_vsync) % period));
        next_vsync = now + sleep;
    }
    mNextFakeVSync = next_vsync + period;

    struct timespec spec;
    spec.tv_sec  = next_vsync / 1000000000;
    spec.tv_nsec = next_vsync % 1000000000;

    int err;
    do {
        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
    } while (err<0 && errno == EINTR);

    if (err == 0) {
        mComposer->vsync(next_vsync, IntelHWComposer::VSYNC_SRC_FAKE);
    }

    return true;
}

android::status_t IntelFakeVsyncEvent::readyToRun()
{
    return android::NO_ERROR;
}

void IntelFakeVsyncEvent::onFirstRef()
{
    run("HWC Fake Vsync Event", android::PRIORITY_URGENT_DISPLAY);
}
