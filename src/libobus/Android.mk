###############################################################################
# libobus Android.mk for Android build system
###############################################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libobus

# include libobus common makefile
include $(LOCAL_PATH)/libobus.mk

# remove _FORTIFY_SOURCE (already added by android)
LIBOBUS_CFLAGS := $(filter-out -D_FORTIFY_SOURCE=2,$(LIBOBUS_CFLAGS))

LOCAL_EXPORT_C_INCLUDES := $(addprefix $(LOCAL_PATH)/,$(sort $(dir $(LIBOBUS_PUBLIC_HEADER_FILES))))
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)
LOCAL_C_INCLUDES := $(LOCAL_EXPORT_C_INCLUDES)
LOCAL_CFLAGS := $(LIBOBUS_CFLAGS) -DANDROID -DHAVE_SYS_TIMERFD_H -DHAVE_EPOLL
LOCAL_LDFLAGS := $(LIBOBUS_LDFLAGS)
LOCAL_SRC_FILES := $(LIBOBUS_SOURCE_FILES)

include $(BUILD_SHARED_LIBRARY)
