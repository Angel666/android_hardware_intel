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
#ifndef __INTEL_WIDI_PLANE_H__
#define __INTEL_WIDI_PLANE_H__

#ifdef INTEL_WIDI

#include <IntelDisplayPlaneManager.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <binder/IInterface.h>
#include <binder/IServiceManager.h>
#include "IHwWidiPlane.h"

static const unsigned int EXT_VIDEO_MODE_MAX_SURFACE = 32;

typedef struct {
        intel_gralloc_payload_t *p;
        IntelDisplayBuffer      *pDB;
}widiPayloadBuffer_t;

class IntelWidiPlane : public IntelDisplayPlane , public intel::widi::BnHwWidiPlane {

public:
    IntelWidiPlane(int fd, int index, IntelBufferManager *bufferManager);
    ~IntelWidiPlane();
    virtual void setPosition(int left, int top, int right, int bottom);

    android::status_t  enablePlane(android::sp<android::IBinder> display);
    void  disablePlane(bool isConnected);
    android::status_t  registerFlipListener(android::sp<IPageFlipListener> listener);
    void allowExtVideoMode(bool allow);
    bool isExtVideoAllowed() {return mAllowExtVideoMode;};
    void setPlayerStatus(bool status, int fps);
    void setOrientation(uint32_t orientation);
    void setOverlayData(intel_gralloc_buffer_handle_t* nHandle, uint32_t width, uint32_t height);
    bool isStreaming() { return (mState == WIDI_PLANE_STATE_STREAMING); };
    bool isPlayerOn() { return mPlayerStatus; };
    void returnBuffer(int index);

    bool flip(void *contexts, uint32_t flags);
    bool isActive();
    bool isWidiStatusChanged();
    android::status_t setOrientationChanged();

protected:
    void init();
    android::status_t sendInitMode(int mode, uint32_t width, uint32_t height);
    bool mapPayloadBuffer(intel_gralloc_buffer_handle_t* gHandle, widiPayloadBuffer_t* wPayload);
    void unmapPayloadBuffer(widiPayloadBuffer_t* wPayload);
    void clearExtVideoModeContext(bool lock = true);

    class WidiInitThread: public android::Thread {
    public:
        WidiInitThread(IntelWidiPlane* me): android::Thread(false), mSelf(me) {LOGV("Widi Plane Init thread created");};
        ~WidiInitThread(){LOGV("Widi Plane Init thread destroyed");};

    private:
        bool  threadLoop();
    private:
        android::sp<IntelWidiPlane> mSelf;
    };
    typedef enum {
        WIDI_PLANE_STATE_UNINIT,
        WIDI_PLANE_STATE_INITIALIZED,
        WIDI_PLANE_STATE_ACTIVE,
        WIDI_PLANE_STATE_STREAMING
    }WidiPlaneState;

    class DeathNotifier: public android::IBinder::DeathRecipient
    {
    public:
                DeathNotifier(IntelWidiPlane* me): mSelf(me) {}
        virtual ~DeathNotifier();

        virtual void binderDied(const android::wp<android::IBinder>& who);
    private:
        android::sp<IntelWidiPlane> mSelf;
    };

    int32_t                         mAllowExtVideoMode;
    WidiPlaneState                  mState;
    bool                            mWidiStatusChanged;
    bool                            mPlayerStatus;
    bool                            mStreamingEnabled;
    int                             mExtFrameRate;

    android::Mutex                  mLock;
    android::sp<WidiInitThread>     mInitThread;
    android::sp<IPageFlipListener>  mFlipListener;
    android::sp<DeathNotifier>      mDeathNotifier;
    android::sp<IBinder>            mWirelesDisplayservice;
    android::sp<IBinder>            mWirelessDisplay;
    uint32_t                        mCurrentOrientation;

    android::Mutex                  mExtBufferMapLock;
    widiPayloadBuffer_t             mCurrExtFramePayload;
    uint32_t                        mPrevExtFrame;
    intel_widi_ext_buffer_meta_t    mExtVideoBufferMeta;
    android::KeyedVector<intel_gralloc_buffer_handle_t*, widiPayloadBuffer_t> mExtVideoBuffersMapping;
    bool                            mUseRotateHandle;
    uint32_t                        mExtWidth;
    uint32_t                        mExtHeight;
};

#else  // Stub implementation in case of widi module is not compiled

class IntelWidiPlane : public IntelDisplayPlane  {

public:
    IntelWidiPlane(int fd, int index, IntelBufferManager *bm):
        IntelDisplayPlane(fd, IntelDisplayPlane::DISPLAY_PLANE_OVERLAY, index, bm){};
    ~IntelWidiPlane(){};
    virtual void setPosition(int left, int top, int right, int bottom){return;};
    void setOverlayData(intel_gralloc_buffer_handle_t* nHandle, uint32_t width, uint32_t height){};

    void allowExtVideoMode(bool allow){return;};
    bool isExtVideoAllowed() {return true;};
    void setPlayerStatus(bool status, int fps) {return;};
    void setOrientation(uint32_t orientation){return;};

    bool flip(void *contexts, uint32_t flags){return true;};
    bool isActive(){return false;};
    bool isStreaming() { return false; };
    bool isPlayerOn() { return false; };
    bool isWidiStatusChanged(){return false;};
    android::status_t setOrientationChanged() {return android::NO_ERROR;};


};
#endif

#endif /*__INTEL_OVERLAY_PLANE_H__*/
