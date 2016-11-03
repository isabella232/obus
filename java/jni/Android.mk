###############################################################################
# libobus-jni.so makefile used by libobus-java
# 
# Follow theses step to compile this library:
# 1: get android NDK at http://developer.android.com/tools/sdk/ndk/index.html
# 2: call '<android-ndk-dir>/ndk-build' cmd in this directory 
###############################################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libobus-jni
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := socket.c
LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_SHARED_LIBRARY)
