LOCAL_PATH:= $(call my-dir)

# Do not compile the Audience proxy if the board does not include Audience
ifeq ($(BOARD_HAVE_AUDIENCE),true)

common_c_includes := \
	$(KERNEL_HEADERS) \

common_shared_libraries := \
	libsysutils \
	libcutils

include $(CLEAR_VARS)

LOCAL_SRC_FILES := proxy_main.c ad_i2c.c ad_protocol.c ad_usb_tty.c

LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

LOCAL_MODULE := ad_proxy
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

endif

