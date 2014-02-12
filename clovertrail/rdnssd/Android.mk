# Copyright 2006 The Android Open Source Project
ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)

etc_dir := $(TARGET_OUT)/etc/rdnssd
hooks_dir := rdnssd-hooks
hooks_target := $(etc_dir)/$(hooks_dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := icmp.c netlink.c rdnssd.c logger.c ppoll.c
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE = rdnssd
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

endif
