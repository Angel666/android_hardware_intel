LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/wbxml_buffers.c \
    src/wbxml_elt.c \
    src/wbxml_encoder.c \
    src/wbxml_errors.c \
    src/wbxml_lists.c \
    src/wbxml_log.c \
    src/wbxml_mem.c \
    src/wbxml_parser.c \
    src/wbxml_tables.c \
    src/wbxml_tree_clb.c \
    src/wbxml_tree.c \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/hdr

LOCAL_CFLAGS += -DREMOVE_UNUSED

LOCAL_MODULE := libwbxmlparser

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
