/*
* Copyright Â© 2012 Intel Corporation
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
*/

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <IPageFlipListener.h>

using namespace android;

class BpPageFlipListener : public BpInterface<IPageFlipListener>
{
public:
    BpPageFlipListener(const sp<IBinder>& impl)
        : BpInterface<IPageFlipListener>(impl)
    {
    }

    virtual void pageFlipped(int64_t time, uint32_t orientation)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPageFlipListener::getInterfaceDescriptor());
        data.writeInt64(time);
        data.writeInt32(orientation);
        remote()->transact(BnPageFlipListener::PAGE_FLIPPED, data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(PageFlipListener, "android.ui.IPageFlipListener");

status_t BnPageFlipListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case PAGE_FLIPPED: {
            CHECK_INTERFACE(IPageFlipListener, data, reply);
            int64_t time = data.readInt64();
            uint32_t orientation = data.readInt32();
            pageFlipped(time, orientation);
            reply->writeNoException();
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
    return NO_ERROR;
}


