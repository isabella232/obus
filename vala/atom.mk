
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libobus-vala

# Install vapi files
LOCAL_INSTALL_HEADERS := \
	libobus.vapi:$(TARGET_OUT_STAGING)/usr/share/vala/vapi/ \
	libobus.deps:$(TARGET_OUT_STAGING)/usr/share/vala/vapi/

include $(BUILD_CUSTOM)

