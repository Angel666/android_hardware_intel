/*
 **
 ** Copyright 2011 Intel Corporation
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **      http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 **
 ** Author:
 ** Zhang, Dongsheng <dongsheng.zhang@intel.com>
 ** Li, Peizhao <peizhao.li@intel.com>
 **
 */
#ifndef _AD_LOG_H_
#define _AD_LOG_H_

#define LOG_TAG "ad_proxy"
#include "cutils/log.h"

#define AD_PROXY_DEBUG

#define RERRO(f, arg...) SLOGE(f, ## arg)
#define RTRAC(f, arg...)  SLOGW(f, ## arg)

#ifdef AD_PROXY_DEBUG
#define RDBUG(f, arg...) SLOGD(f, ## arg)
#else
#define RDBUG(f, arg...)
#endif

#endif
