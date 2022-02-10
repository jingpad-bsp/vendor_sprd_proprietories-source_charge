LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
       autosuspend.c \
       autosuspend_wakeup_count.c

LOCAL_CFLAGS := -Werror

LOCAL_STATIC_LIBRARIES += libcutils

LOCAL_MODULE := libcharge_suspend
LOCAL_VENDOR_MODULE := true

include $(BUILD_STATIC_LIBRARY)
