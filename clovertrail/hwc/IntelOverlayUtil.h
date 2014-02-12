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
#ifndef __INTEL_OVERLAY_UTIL_H__
#define __INTEL_OVERLAY_UTIL_H__

static inline uint32_t align_to(uint32_t arg, uint32_t align)
{
    return ((arg + (align - 1)) & (~(align - 1)));
}

/* Data device parameters*/
enum {
    MRST_OVERLAY_PIPE = 0,
    MRST_OVERLAY_USAGE,
    MRST_OVERLAY_DISPLAY_BUFFER,
    MRST_OVERLAY_Y_STRIDE,
    MRST_OVERLAY_RESET,
    MRST_OVERLAY_HDMI_STATUS,
};

/*Reset operations*/
enum {
    MRST_OVERLAY_OP_RESET = 0,
    MRST_OVERLAY_OP_DISABLE,
    MRST_OVERLAY_OP_ENABLE,
};

/*HDMI status for MDFLD_OVERLAY_HDMI_STATUS parameter setting*/
enum {
    MRST_HDMI_STATUS_DISCONNECTED = 0,
    MRST_HDMI_STATUS_CONNECTED_CLONE,
    MRST_HDMI_STATUS_CONNECTED_EXTEND,
};

/*struct for MDFLD_OVERLAY_DISPLAY_BUFFER parameter setting*/
struct overlay_display_buffer_param {
    /*BCD buffer device id*/
    uint32_t device;

    /*index*/
    uint32_t handle;

    /*rotated buffer handle*/
    uint32_t kBufferHandle;
};

#endif /*__INTEL_OVERLAY_UTIL_H__*/
