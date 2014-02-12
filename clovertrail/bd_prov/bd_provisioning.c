/*
 *  bd_provisioning.c - bluetooth device provisioning application
 *
 *  Copyright(c) 2009-2011 Intel Corporation. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#undef ALOGV
#define ALOG_NDEBUG 0

#include <cutils/log.h>
#include <cutils/properties.h>

#ifdef BUILD_WITH_CHAABI_SUPPORT
#include "umip_access.h"
#endif

#define ALOG_TAG "bd_prov"

#define BD_ADDRESS_LEN 6
#define UIM_ASCII_PARAM_LEN 22

#define NO_ERR                       0
#define ERR_WRONG_PARAM             -1

int main(int argc, char **argv)
{
	unsigned char *chaabi_bd_address = NULL;
	int res = NO_ERR;
	char uim_ascii_param[UIM_ASCII_PARAM_LEN];
	char state[PROPERTY_VALUE_MAX];

	/* Check parameters */
	if (argc != 1) {
		/* No param expected */
		return ERR_WRONG_PARAM;
	}

#ifdef BUILD_WITH_CHAABI_SUPPORT
	/* Read BD address from Chaabi */

	ALOGV("Retrieving bd address from chaabi ...");

	res = get_customer_data(ACD_BT_MAC_ADDR_FIELD_INDEX,
			(void ** const) &chaabi_bd_address);
	if ((res != BD_ADDRESS_LEN) || !chaabi_bd_address) {
		/* chaabi read error OR no chaabi support */
		if (res < 0)
			ALOGE("Error retrieving chaabi bd address, "
					"error %d", res);
		else
			ALOGE("Error retrieving chaabi bd address, "
					"wrong length");
		if (chaabi_bd_address) {
			free(chaabi_bd_address);
			chaabi_bd_address = NULL;
		}
	} else {
		ALOGV("Bd address successfully retrieved from chaabi: "
				"%02X:%02X:%02X:%02X:%02X:%02X",
				chaabi_bd_address[0], chaabi_bd_address[1],
				chaabi_bd_address[2], chaabi_bd_address[3],
				chaabi_bd_address[4], chaabi_bd_address[5]);
	}
#else
	ALOGE("Chaabi not supported, "
			"bd address diversification is not available");
#endif

	/* If uim is already running, stop it so it can be restarted */
	property_get("init.svc.uim", state, "");
	if (!strcmp("running", state)) {
		ALOGI("Stopping uim");
		property_set("ctl.stop", "uim");
	}

	if (chaabi_bd_address) {
		sprintf(uim_ascii_param, "uim:%02X:%02X:%02X:%02X:%02X:%02X",
				chaabi_bd_address[0], chaabi_bd_address[1],
				chaabi_bd_address[2], chaabi_bd_address[3],
				chaabi_bd_address[4], chaabi_bd_address[5]);
		ALOGI("Starting %s...", uim_ascii_param);
		property_set("ctl.start", uim_ascii_param);
	} else {
		ALOGI("Starting uim...");
		property_set("ctl.start", "uim");
	}

	return NO_ERR;
}
