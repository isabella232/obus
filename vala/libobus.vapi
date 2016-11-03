/******************************************************************************
 * obus-vala - obus vala binding.
 *
 * @file libobus.vapi
 *
 * @brief obus vala binding
 *
 * @author yves-marie.morgan@parrot.com
 *
 * Copyright (c) 2013 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

[CCode (cheader_filename = "libobus.h")]
namespace Obus {

    [CCode (cname = "OBUS_INVALID_HANDLE")]
    public const Handle INVALID_HANDLE;

    [CCode (cname = "OBUS_INVALID_UID")]
    public const uint16 INVALID_UID;

    [CCode (cname = "enum obus_log_level", cprefix = "OBUS_LOG_", has_type_id = false)]
    public enum LogLevel {
        CRITICAL,
        ERROR,
        WARNING,
        NOTICE,
        INFO,
        DEBUG;
    }

    [CCode (cname = "enum obus_method_state", cprefix = "OBUS_METHOD_", has_type_id = false)]
    public enum MethodState {
        NOT_SUPPORTED,
        ENABLED,
        DISABLED;
        [CCode (cname = "obus_method_state_str")]
        public unowned string to_string();
    }

    [CCode (cname = "enum obus_call_status", cprefix = "OBUS_CALL_", has_type_id = false)]
    public enum CallStatus {
        INVALID,
        ACKED,
        ABORTED,
        METHOD_DISABLED,
        METHOD_NOT_SUPPORTED,
        INVALID_ARGUMENTS,
        REFUSED;
        [CCode (cname = "obus_call_status_str")]
        public unowned string to_string();
    }

    [CCode (cname = "enum obus_peer_event", cprefix = "OBUS_PEER_EVENT_", has_type_id = false)]
    public enum PeerEvent {
        CONNECTING,
        CONNECTED,
        DISCONNECTED;
        [CCode (cname = "obus_peer_event_str")]
        public unowned string to_string();
    }

    [SimpleType]
    [IntegerType (rank = 5, min = 0, max = 65535)]
    [CCode (cname = "obus_handle_t", default_value = "0", has_type_id = false)]
    public struct Handle : uint16 {
    }

    [Compact]
    [CCode (cname = "struct obus_bus_desc", has_type_id = false)]
    public struct BusDesc {
    }

    [Compact]
    [CCode (cname = "struct obus_bus_event", has_type_id = false)]
    public struct BusEvent {
    }

    [Compact]
    [CCode (cname = "struct obus_peer", has_type_id = false)]
    public class Peer {
        public unowned string name {get;}
        public unowned string address {get;}
        public void* user_data {get; set;}
        public void refuse_connection();
    }

    [CCode (cname = "obus_log_cb_t", has_target = false)]
    public delegate void LogCb(LogLevel level, string fmt, va_list args);

    [CCode (cname = "obus_peer_connection_cb_t")]
    public delegate void PeerConnectionCb(PeerEvent event, Peer peer);

    [CCode (cname = "obus_bus_event_cb_t")]
    public delegate void BusEventCb(BusEvent event);

    [CCode (cname = "obus_log_set_cb")]
    public static int log_set_cb(LogCb cb);

    [PrintfFormat]
    [CCode (cname = "obus_log")]
    public static void log(LogLevel level, string fmt, ...);

    [Compact]
    [CCode (cname = "struct obus_client", free_function = "obus_client_destroy", has_type_id = false)]
    public class Client {
        public Client(string name, BusDesc *desc, BusEventCb cb);
        public int start(string address);
        public bool is_connected();
        public int fd();
        public int process_fd();
        public int commit_bus_event(BusEvent event);
    }

    [Compact]
    [CCode (cname = "struct obus_server", free_function = "obus_server_destroy", has_type_id = false)]
    public class Server {
        public Server(BusDesc *desc);
        public int start(string[] addrs);
        public int fd();
        public int process_fd();
        public int send_ack(Handle handle, CallStatus status);
        public void set_peer_connection_cb(PeerConnectionCb cb);
        public unowned Peer get_call_peer(Handle handle);
    }
}

