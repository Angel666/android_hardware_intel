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

#ifndef INTEL_HWWIDIPLANE_H
#define INTEL_HWWIDIPLANE_H

#include <utils/Errors.h>  // for status_t
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include "IPageFlipListener.h"

namespace intel {
namespace widi {

class IWirelessDisplay;

// This is the interface for Widi HW Plane  service. This interface is used for
// Enabling the pseudo  HW-Composer plane in the HWC HAL Module

class IHwWidiPlane: public android::IInterface
{
public:
    DECLARE_META_INTERFACE(HwWidiPlane);

    virtual android::status_t  enablePlane(android::sp<android::IBinder> display) = 0;
    virtual void disablePlane(bool isConnected) = 0;
    virtual void allowExtVideoMode(bool allow) = 0;
    virtual android::status_t  registerFlipListener(android::sp<IPageFlipListener> listener) = 0;
    virtual void returnBuffer(int index) = 0;
};

// ----------------------------------------------------------------------------

class BnHwWidiPlane: public android::BnInterface<IHwWidiPlane>
{
public:
    virtual android::status_t    onTransact( uint32_t code,
                                    const android::Parcel& data,
                                    android::Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace widi
}; // namespace intel

#endif // INTEL_HWWIDIPLANE_H
