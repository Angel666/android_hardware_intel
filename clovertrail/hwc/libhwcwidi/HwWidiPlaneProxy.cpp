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

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <utils/Errors.h>  // for status_t

#include "IHwWidiPlane.h"

namespace intel {
namespace widi {

using namespace android;

enum {
    ENABLE_HW_WIDI_PLANE = IBinder::FIRST_CALL_TRANSACTION,
    DISABLE_HW_WIDI_PLANE,
    REGISTER_FLIP_LISTENER,
    ALLOW_EXT_VIDEO_MODE,
    RETURN_BUFFER
};

class BpHwWidiPlane: public BpInterface<IHwWidiPlane>
{
public:
    BpHwWidiPlane(const sp<IBinder>& impl)
        : BpInterface<IHwWidiPlane>(impl)
    {
    }

    virtual status_t  enablePlane(sp<IBinder> widiClass)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IHwWidiPlane::getInterfaceDescriptor());
        data.writeStrongBinder(widiClass);
        remote()->transact(ENABLE_HW_WIDI_PLANE, data, &reply);
        return reply.readInt32();
    }
    virtual void  disablePlane(bool isConnected)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IHwWidiPlane::getInterfaceDescriptor());
        data.writeInt32((int32_t)(isConnected ? 1 : 0));
        remote()->transact(DISABLE_HW_WIDI_PLANE, data, &reply);
        return;
    }
    virtual status_t  registerFlipListener(sp<IPageFlipListener> listener)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IHwWidiPlane::getInterfaceDescriptor());
        data.writeStrongBinder(listener->asBinder());
        remote()->transact(REGISTER_FLIP_LISTENER, data, &reply);
        return reply.readInt32();
    }
    virtual void allowExtVideoMode(bool allow) {
        Parcel data, reply;
        data.writeInterfaceToken(IHwWidiPlane::getInterfaceDescriptor());
        data.writeInt32(((int32_t) allow));
        remote()->transact(ALLOW_EXT_VIDEO_MODE, data, &reply);
        return;
    }
    virtual void returnBuffer(int index) {
        Parcel data, reply;
        data.writeInterfaceToken(IHwWidiPlane::getInterfaceDescriptor());
        data.writeInt32(((int32_t) index));
        remote()->transact(RETURN_BUFFER, data, &reply, IBinder::FLAG_ONEWAY);
        return;
    }
};

IMPLEMENT_META_INTERFACE(HwWidiPlane, "android.widi.IHwWidiPlane");

// ----------------------------------------------------------------------

status_t BnHwWidiPlane::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ENABLE_HW_WIDI_PLANE: {
            CHECK_INTERFACE(IHwWidiPlane, data, reply);
            sp<IBinder> widiClass = data.readStrongBinder();
            enablePlane(widiClass);
            reply->writeInt32(NO_ERROR);
            return NO_ERROR;
        } break;
        case DISABLE_HW_WIDI_PLANE: {
            bool isConnected =false;
            CHECK_INTERFACE(IHwWidiPlane, data, reply);
            isConnected = data.readInt32() == 1;
            disablePlane(isConnected);
            reply->writeInt32(NO_ERROR);
            return NO_ERROR;
        } break;
        case REGISTER_FLIP_LISTENER: {
            CHECK_INTERFACE(IHwWidiPlane, data, reply);
            sp<IBinder> listener = data.readStrongBinder();
            registerFlipListener(interface_cast<IPageFlipListener>(listener));
            reply->writeInt32(NO_ERROR);
            return NO_ERROR;
        } break;
        case ALLOW_EXT_VIDEO_MODE: {
            CHECK_INTERFACE(IHwWidiPlane, data, reply);
            int32_t allow = data.readInt32();
            allowExtVideoMode(allow);
            reply->writeInt32(NO_ERROR);
            return NO_ERROR;
        } break;
        case RETURN_BUFFER: {
            CHECK_INTERFACE(IHwWidiPlane, data, reply);
            int32_t index = data.readInt32();
            returnBuffer(index);
            reply->writeInt32(NO_ERROR);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace widi
}; // namespace intel
