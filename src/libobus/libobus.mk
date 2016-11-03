###############################################################################
# libobus common makefile used by all others makefiles (autotools, android)
###############################################################################

# activate common warnings
LIBOBUS_WARNINGS := \
	-Wall \
	-Wshadow \
	-Wextra \
	-Wno-unused -Wno-unused-parameter \
	-Wunused-value -Wunused-variable -Wunused-label \
	-Wpointer-arith \
	-Wformat-nonliteral \
	-Wformat-security \
	-Winit-self \
	-Wmissing-prototypes \
	-Wdeclaration-after-statement

# set all symbols hidden by default, visiblity is restored with gcc attribute
LIBOBUS_CFLAGS := \
	$(LIBOBUS_WARNINGS) \
	-D_FORTIFY_SOURCE=2 \
	-fdiagnostics-show-option \
	-fvisibility=hidden

# enable relocation read only
LIBOBUS_LDFLAGS := \
	-Wl,--no-undefined \
	-Wl,-z,norelro,-z,now \
	-Wl,--export-dynamic

LIBOBUS_PUBLIC_HEADER_FILES := \
	include/libobus.h \
	include/libobus_private.h

LIBOBUS_HEADER_FILES := \
	src/obus_buffer.h \
	src/obus_bus_api.h \
	src/obus_bus_event.h \
	src/obus_bus.h \
	src/obus_call.h \
	src/obus_event.h \
	src/obus_field.h \
	src/obus_hash.h \
	src/obus_header.h \
	src/obus_io.h \
	src/obus_list.h \
	src/obus_log.h \
	src/obus_loop.h \
	src/obus_object.h \
	src/obus_packet.h \
	src/obus_platform.h \
	src/obus_socket.h \
	src/obus_struct.h \
	src/obus_timer.h \
	src/obus_utils.h

LIBOBUS_SOURCE_FILES := \
	src/obus_log.c \
	src/obus_utils.c \
	src/obus_loop.c \
	src/obus_loop_posix.c \
	src/obus_hash.c \
	src/obus_timer.c \
	src/obus_timer_posix.c \
	src/obus_io.c \
	src/obus_socket.c \
	src/obus_field.c \
	src/obus_struct.c \
	src/obus_object.c \
	src/obus_event.c \
	src/obus_call.c \
	src/obus_bus_event.c \
	src/obus_bus_api.c \
	src/obus_bus.c \
	src/obus_packet.c \
	src/obus_server.c \
	src/obus_client.c
