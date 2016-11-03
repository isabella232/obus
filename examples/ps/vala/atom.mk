LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := psd-vala

LOCAL_LDLIBS := \
	-lglib-2.0 \
	-lgio-2.0 \
	-lgobject-2.0

LOCAL_VALAFLAGS := \
	--vapidir=$(LOCAL_PATH)/generated \
	--pkg posix \
	--pkg linux \
	--pkg libobus \
	--pkg ps

LOCAL_C_INCLUDES := $(LOCAL_PATH)/generated

LOCAL_SRC_FILES := \
	psd.vala

LOCAL_LIBRARIES := \
	glib libobus-vala libobus

LOCAL_DEPENDS_HOST_MODULES := host.vala host.obusgen

LOCAL_CUSTOM_MACROS := \
	obusgen-macro:server,vala,unused,$(LOCAL_PATH)/generated,$(LOCAL_PATH)/../ps.xml

include $(BUILD_EXECUTABLE)
