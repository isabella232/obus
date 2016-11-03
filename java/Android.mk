###############################################################################
# libobus-java makefile
###############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_MODULE := libobus-java
LOCAL_REQUIRED_MODULES := libobus-jni
LOCAL_JNI_SHARED_LIBRARIES := libobus-jni

include $(BUILD_STATIC_JAVA_LIBRARY)

# include jni makefile (hidden by this makefile !)
include $(LOCAL_PATH)/jni/Android.mk