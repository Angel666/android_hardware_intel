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

#ifndef ANDROID_SF_IPAGE_FLIP_LISTENER_H
#define ANDROID_SF_IPAGE_FLIP_LISTENER_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>


class IPageFlipListener : public android::IInterface
{
public:
    DECLARE_META_INTERFACE(PageFlipListener);

    virtual void pageFlipped(int64_t time, uint32_t orientation) = 0;
};

class BnPageFlipListener : public android::BnInterface<IPageFlipListener>
{
public:
    enum {
        PAGE_FLIPPED = IBinder::FIRST_CALL_TRANSACTION,
    };

    virtual android::status_t onTransact(   uint32_t code,
                                            const android::Parcel& data,
                                            android::Parcel* reply,
                                            uint32_t flags = 0);
};


#endif // ANDROID_SF_IPAGE_FLIP_LISTENER_H
