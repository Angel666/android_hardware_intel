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
 *    Lei Zhang <lei.zhang@intel.com>
 *
 */
#ifndef __INTEL_HWCOMPOSER_CFG_CPP__
#define __INTEL_HWCOMPOSER_CFG_CPP__

#define HWC_CFG_PATH "/etc/hwc.cfg"
#define CFG_STRING_LEN 1024

typedef struct {
    unsigned char enable;
    unsigned int log_level;
    unsigned int bypasspost;
} hwc_cfg;

enum hwc_log_level {
    NO_DEBUG = 0x00,
    HWC_DEBUG = 0x01,
    PLANE_DEBUG = 0x02,
    SPRITE_DEBUG = 0x04,
    OVERLAY_DEBUG = 0x08,
    WIDI_DEBUG = 0x10,
    LAYER_DEBUG = 0x20,
    MONITOR_DEBUG = 0x40,
    BUFFER_DEBUG = 0x80,
};

#define ALLOW_PRINT(cfg, level) ((cfg & level) == level) ? true : false

#define ALLOW_NO_PRINT         ALLOW_PRINT(cfg.log_level, NO_DEBUG)
#define ALLOW_HWC_PRINT        ALLOW_PRINT(cfg.log_level, HWC_DEBUG)
#define ALLOW_PLANE_PRINT      ALLOW_PRINT(cfg.log_level, PLANE_DEBUG)
#define ALLOW_SPRITE_PRINT     ALLOW_PRINT(cfg.log_level, SPRITE_DEBUG)
#define ALLOW_OVERLAY_PRINT    ALLOW_PRINT(cfg.log_level, OVERLAY_DEBUG)
#define ALLOW_WIDI_PRINT       ALLOW_PRINT(cfg.log_level, WIDI_DEBUG)
#define ALLOW_LAYER_PRINT      ALLOW_PRINT(cfg.log_level, LAYER_DEBUG)
#define ALLOW_MONITOR_PRINT    ALLOW_PRINT(cfg.log_level, MONITOR_DEBUG)
#define ALLOW_BUFFER_PRINT     ALLOW_PRINT(cfg.log_level, BUFFER_DEBUG)

extern hwc_cfg cfg;

#endif /*__INTEL_HWCOMPOSER_CFG_CPP__*/
