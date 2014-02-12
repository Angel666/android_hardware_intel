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
#include <hardware/hardware.h>

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <IntelHWComposer.h>
#include <IntelHWComposerCfg.h>

/* global hwcomposer cfg info */
hwc_cfg cfg;

static void dump_layer(hwc_layer_t const* l)
{

}

static int hwc_prepare(hwc_composer_device_t *dev, hwc_layer_list_t* list)
{
    int status = 0;

    LOGV("%s\n", __func__);

    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    if (!hwc) {
        LOGE("%s: Invalid HWC device\n", __func__);
        status = -EINVAL;
        goto prepare_out;
    }

#ifdef INTEL_RGB_OVERLAY
    {
        IntelHWCWrapper* wrapper = hwc->getWrapper();
        wrapper->pre_prepare(list);
    }
#endif

    if (hwc->prepare(list) == false) {
        status = -EINVAL;
        goto prepare_out;
    }
#ifdef INTEL_RGB_OVERLAY
#ifdef SKIP_DISPLAY_SYS_LAYER
    {
        IntelHWCWrapper* wrapper = hwc->getWrapper();
        wrapper->post_prepare(list);
    }
#endif
#endif

prepare_out:
    return 0;
}

static int hwc_set(hwc_composer_device_t *dev,
                   hwc_display_t dpy,
                   hwc_surface_t sur,
                   hwc_layer_list_t* list)
{
    int status = 0;

    LOGV("%s\n", __func__);

    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    if (!hwc) {
        LOGE("%s: Invalid HWC device\n", __func__);
        status = -EINVAL;
        goto set_out;
    }

#ifdef INTEL_RGB_OVERLAY
#ifdef SKIP_DISPLAY_SYS_LAYER
    if (dpy && sur && list)
    {
        IntelHWCWrapper* wrapper = hwc->getWrapper();
        wrapper->pre_commit(dpy, sur, list);
    }
#endif
#endif

    if (!dpy && !sur && !list) {
        if (hwc->release() == false) {
            LOGD("%s: failed to release\n", __func__);
            status = HWC_EGL_ERROR;
            goto set_out;
        }
    } else if (hwc->commit(dpy, sur, list) == false) {
        LOGE("%s: failed to commit\n", __func__);
        status = HWC_EGL_ERROR;
        goto set_out;
    }

#ifdef INTEL_RGB_OVERLAY
#ifdef SKIP_DISPLAY_SYS_LAYER
    if (dpy && sur && list)
    {
        IntelHWCWrapper* wrapper = hwc->getWrapper();
        wrapper->post_commit(dpy, sur, list);
    }
#endif
#endif

set_out:
    return 0;
}

static void hwc_dump(struct hwc_composer_device *dev, char *buff, int buff_len)
{
    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    if (hwc)
       hwc->dump(buff, buff_len, 0);
}

void hwc_registerProcs(struct hwc_composer_device* dev,
                       hwc_procs_t const* procs)
{
    LOGV("%s\n", __func__);

    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    if (!hwc) {
        LOGE("%s: Invalid HWC device\n", __func__);
        return;
    }

    hwc->registerProcs(procs);
}

static int hwc_device_close(struct hw_device_t *dev)
{
#if 0
    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    LOGD("%s\n", __func__);

    delete hwc;
#endif
    return 0;
}

static int hwc_get_cfg(hwc_cfg *cfg)
{
    FILE *fp = NULL;
    char cfg_string[CFG_STRING_LEN];
    char * pch;

    if (cfg == NULL) {
        LOGE("%s: pass NULL parameter!\n", __func__);
        return -1;
    } else {
        /* set default cfg parameter */
        cfg->enable = 1;
        cfg->log_level = 0;
        cfg->bypasspost = 1;
    }

    fp = fopen(HWC_CFG_PATH, "r");
    if (fp != NULL) {
        memset(cfg_string, '\0', CFG_STRING_LEN);
        fread(cfg_string, 1, CFG_STRING_LEN-1, fp);

        LOGD("%s: read config line %s!\n", __func__, cfg_string);

        pch = strstr(cfg_string, "enable");
        if (pch != NULL)
            cfg->enable = *(pch+strlen("enable=")) - '0';

        pch = strstr(cfg_string, "log_level");
        if (pch != NULL)
            cfg->log_level = atoi(pch+strlen("log_level="));

        pch = strstr(cfg_string, "bypasspost");
        if (pch != NULL)
            cfg->bypasspost = atoi(pch+strlen("bypasspost="));


        LOGD("%s:get cfg parameter enable_hwc=%d,log_level=%d,bypasspost=%d!\n",
                     __func__, cfg->enable, cfg->log_level, cfg->bypasspost);
        fclose(fp);
    }

    return 0;
}

static int hwc_query(struct hwc_composer_device* dev,
                       int what,
                       int* value)
{
    LOGD("%s: what %d\n", __func__, what);
    return -EINVAL;
}

static int hwc_event_control(struct hwc_composer_device *dev,
                                int event,
                                int enabled)
{
    int err = 0;
    bool ret;

    LOGV("%s: event %d, enabled %d\n", __func__, event, enabled);
    IntelHWComposer *hwc = static_cast<IntelHWComposer*>(dev);

    if (!hwc) {
        LOGE("%s: Invalid HWC device\n", __func__);
        return -EINVAL;
    }

    switch (event) {
    case HWC_EVENT_VSYNC:
        ret = hwc->vsyncControl(enabled);
        if (ret == false) {
            LOGE("%s: failed to enable/disable vsync\n", __func__);
            err = -EINVAL;
        }
        break;
    default:
        LOGE("%s: unsupported event %d\n", __func__, event);
    }

    return err;
}

struct hwc_methods hwc_event_methods = {
    eventControl: hwc_event_control
};

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;

    LOGV("%s: name %s\n", __func__, name);

    hwc_get_cfg(&cfg);

    if (cfg.enable == 0) {
        status = -EINVAL;
        goto hwc_init_out;
    }

    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        IntelHWComposer *hwc = new IntelHWComposer();
        if (!hwc) {
            LOGE("%s: No memory\n", __func__);
            status = -ENOMEM;
            goto hwc_init_out;
        }

        /* initialize our state here */
        if (hwc->initialize() == false) {
            LOGE("%s: failed to intialize HWCompower\n", __func__);
            status = -EINVAL;
            goto hwc_init_out;
        }

#ifdef INTEL_RGB_OVERLAY
        {
            IntelHWCWrapper* wrapper = hwc->getWrapper();
            if (wrapper->initialize() == false) {
                // Don't return error even failed to initialize wrapper.
                ALOGW("%s: failed to intialize HWCompowerWrapper\n", __func__);
            }
        }
#endif

        /* initialize the procs */
        hwc->hwc_composer_device_t::common.tag = HARDWARE_DEVICE_TAG;
        hwc->hwc_composer_device_t::common.version = HWC_DEVICE_API_VERSION_0_3;
        hwc->hwc_composer_device_t::common.module =
            const_cast<hw_module_t*>(module);
        hwc->hwc_composer_device_t::common.close = hwc_device_close;

        hwc->hwc_composer_device_t::prepare = hwc_prepare;
        hwc->hwc_composer_device_t::set = hwc_set;
        hwc->hwc_composer_device_t::dump = hwc_dump;
        hwc->hwc_composer_device_t::registerProcs = hwc_registerProcs;
        hwc->hwc_composer_device_t::query = hwc_query;
        hwc->hwc_composer_device_t::methods = &hwc_event_methods;

        *device = &hwc->hwc_composer_device_t::common;
        status = 0;
    }
hwc_init_out:
    return status;
}

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Intel Hardware Composer",
        author: "Intel UMSE",
        methods: &hwc_module_methods,
    }
};
