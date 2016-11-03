###############################################################################
# libobus atom.mk for alchemy build system
###############################################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libobus
LOCAL_DESCRIPTION := obus is a linux interprocess objects synchronisation protocol.
LOCAL_CATEGORY_PATH := libs/obus

# include libobus common makefile
include $(LOCAL_PATH)/libobus.mk

LOCAL_EXPORT_C_INCLUDES := $(addprefix $(LOCAL_PATH)/,$(sort $(dir $(LIBOBUS_PUBLIC_HEADER_FILES))))
LOCAL_CFLAGS := $(LIBOBUS_CFLAGS) -DHAVE_SYS_TIMERFD_H -DHAVE_EPOLL
LOCAL_LDFLAGS := $(LIBOBUS_LDFLAGS)
LOCAL_SRC_FILES := $(LIBOBUS_SOURCE_FILES)

include $(BUILD_SHARED_LIBRARY)
