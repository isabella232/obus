
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := netclient
LOCAL_DESCRIPTION := obus network client
LOCAL_CATEGORY_PATH := libs/obus/test
LOCAL_LIBRARIES := libobus

LOCAL_C_INCLUDES := $(LOCAL_PATH)/generated

LOCAL_CFLAGS := \
	-Wdeclaration-after-statement \
	-Wunsafe-loop-optimizations \
	-Wshadow -Wmissing-prototypes \
	-D_FORTIFY_SOURCE=2

LOCAL_SRC_FILES := \
	netclient.c

LOCAL_DEPENDS_HOST_MODULES := host.obusgen

LOCAL_CUSTOM_MACROS := \
	obusgen-macro:client,c,unused,$(LOCAL_PATH)/generated,$(LOCAL_PATH)/../net.xml

include $(BUILD_EXECUTABLE)
