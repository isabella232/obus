/******************************************************************************
* @file ps.vapi
*
* @brief ps bus vala interface file
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/

namespace Ps {
    [CCode (cheader_filename = "ps_bus.h", cname = "ps_bus_desc", ctype = "const struct obus_bus_desc *")]
    public Obus.BusDesc *desc;

    [Compact]
    [CCode (cheader_filename = "ps_bus.h", cname = "struct ps_bus_event", free_function = "ps_bus_event_destroy")]
    public class BusEvent {
        [CCode (cheader_filename = "ps_bus.h", cname = "enum ps_bus_event_type", cprefix = "PS_BUS_EVENT_")]
        public enum Type {
            CONNECTED,
            DISCONNECTED,
            CONNECTION_REFUSED,
            UPDATED,
            COUNT;
            [CCode (cname = "ps_bus_event_type_str")]
            public unowned string to_string();
        }

        public BusEvent(Type type);
        [CCode (instance_pos = 1.2)]
        public int send(Obus.Server srv);

        public int add_process_event(Process object, Process.EventType type, ref Process.Info info);
        public int register_process(Process object);
        public int unregister_process(Process object);

        public int add_summary_event(Summary object, Summary.EventType type, ref Summary.Info info);
        public int register_summary(Summary object);
        public int unregister_summary(Summary object);
    }

    [Compact]
    [CCode (cheader_filename = "ps_process.h", cname = "struct ps_process", free_function = "ps_process_destroy")]
    public class Process {

        [CCode (cheader_filename = "ps_process.h", cname = "enum ps_process_state", cprefix = "PS_PROCESS_STATE_")]
        public enum State {
            UNKNOWN,
            RUNNING,
            SLEEPING,
            STOPPED,
            ZOMBIE,
            __LAST__;
            [CCode (cname = "ps_process_state_str")]
            public unowned string to_string();
            public bool is_valid();
        }

        [CCode (cheader_filename = "ps_process.h", cname = "enum ps_process_event_type", cprefix = "PS_PROCESS_EVENT_")]
        public enum EventType {
            UPDATED,
            COUNT;
            [CCode (cname = "ps_process_event_type_str")]
            public unowned string to_string();
        }

        [Compact]
        [CCode (cheader_filename = "ps_process.h", cname = "struct ps_process_info_fields")]
        public struct InfoFields {
            public bool pid;
            public bool ppid;
            public bool name;
            public bool exe;
            public bool pcpu;
            public bool state;
        }

        [Compact]
        [CCode (cheader_filename = "ps_process.h", cname = "struct ps_process_info", construct_function = "ps_process_info_init")]
        public struct Info {
            public InfoFields fields;

            [CCode (cname = "pid")]
            private uint32 _pid;
            [CCode (cname = "ppid")]
            private uint32 _ppid;
            [CCode (cname = "name")]
            private unowned string _name;
            [CCode (cname = "exe")]
            private unowned string _exe;
            [CCode (cname = "pcpu")]
            private uint32 _pcpu;
            [CCode (cname = "state")]
            private State _state;

            public uint32 pid {get {return this._pid;} set {this.fields.pid = true; this._pid = value;}}
            public uint32 ppid {get {return this._ppid;} set {this.fields.ppid = true; this._ppid = value;}}
            public unowned string name {get {return this._name;} set {this.fields.name = true; this._name = value;}}
            public unowned string exe {get {return this._exe;} set {this.fields.exe = true; this._exe = value;}}
            public uint32 pcpu {get {return this._pcpu;} set {this.fields.pcpu = true; this._pcpu = value;}}
            public State state {get {return this._state;} set {this.fields.state = true; this._state = value;}}

            public Info();
            public bool is_empty();
        }

        public uint32 pid {get {return this.info.pid;}}
        public uint32 ppid {get {return this.info.ppid;}}
        public unowned string name {get {return this.info.name;}}
        public unowned string exe {get {return this.info.exe;}}
        public uint32 pcpu {get {return this.info.pcpu;}}
        public State state {get {return this.info.state;}}

        private unowned Info* info {get;}
        public Obus.Handle handle {get;}
        public void* user_data {get; set;}

        public static unowned Process @new(Obus.Server srv, ref Info info);
        public bool is_registered();
        [CCode (instance_pos = 1.2)]
        public int register(Obus.Server srv);
        [CCode (instance_pos = 1.2)]
        public int unregister(Obus.Server srv);
        [CCode (instance_pos = 1.2)]
        public int send_event(Obus.Server srv, EventType type, ref Info info);
        public static unowned Process? from_handle(Obus.Server srv, Obus.Handle handle);
        public static unowned Process? next(Obus.Server srv, Process? prev);
    }

    [Compact]
    [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary", free_function = "ps_summary_destroy")]
    public class Summary {

        [CCode (cheader_filename = "ps_summary.h", cname = "enum ps_summary_mode", cprefix = "PS_SUMMARY_MODE_")]
        public enum Mode {
            SOLARIS,
            IRIX,
            __LAST__;
            [CCode (cname = "ps_summary_mode_str")]
            public unowned string to_string();
            public bool is_valid();
        }

        [CCode (cheader_filename = "ps_summary.h", cname = "enum ps_summary_event_type", cprefix = "PS_SUMMARY_EVENT_")]
        public enum EventType {
            UPDATED,
            REFRESH_RATE_CHANGED,
            MODE_CHANGED,
            COUNT;
            [CCode (cname = "ps_summary_event_type_str")]
            public unowned string to_string();
        }

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_info_fields")]
        public struct InfoFields {
            public bool pcpus;
            public bool task_total;
            public bool task_running;
            public bool task_sleeping;
            public bool task_stopped;
            public bool task_zombie;
            public bool refresh_rate;
            public bool mode;
            public bool method_set_refresh_rate;
            public bool method_set_mode;
        }

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_info", construct_function = "ps_summary_info_init")]
        public struct Info {
            public InfoFields fields;

            [CCode (array_length_cname = "n_pcpus", cname = "pcpus")]
            private unowned uint32[] _pcpus;
            [CCode (cname = "task_total")]
            private uint32 _task_total;
            [CCode (cname = "task_running")]
            private uint32 _task_running;
            [CCode (cname = "task_sleeping")]
            private uint32 _task_sleeping;
            [CCode (cname = "task_stopped")]
            private uint32 _task_stopped;
            [CCode (cname = "task_zombie")]
            private uint32 _task_zombie;
            [CCode (cname = "refresh_rate")]
            private uint32 _refresh_rate;
            [CCode (cname = "mode")]
            private Mode _mode;
            [CCode (cname = "method_set_refresh_rate")]
            private Obus.MethodState _method_set_refresh_rate;
            [CCode (cname = "method_set_mode")]
            private Obus.MethodState _method_set_mode;

            public unowned uint32[] pcpus {get {return this._pcpus;} set {this.fields.pcpus = true; this._pcpus = value;}}
            public uint32 task_total {get {return this._task_total;} set {this.fields.task_total = true; this._task_total = value;}}
            public uint32 task_running {get {return this._task_running;} set {this.fields.task_running = true; this._task_running = value;}}
            public uint32 task_sleeping {get {return this._task_sleeping;} set {this.fields.task_sleeping = true; this._task_sleeping = value;}}
            public uint32 task_stopped {get {return this._task_stopped;} set {this.fields.task_stopped = true; this._task_stopped = value;}}
            public uint32 task_zombie {get {return this._task_zombie;} set {this.fields.task_zombie = true; this._task_zombie = value;}}
            public uint32 refresh_rate {get {return this._refresh_rate;} set {this.fields.refresh_rate = true; this._refresh_rate = value;}}
            public Mode mode {get {return this._mode;} set {this.fields.mode = true; this._mode = value;}}
            public Obus.MethodState method_set_refresh_rate {get {return this._method_set_refresh_rate;} set {this.fields.method_set_refresh_rate = true; this._method_set_refresh_rate = value;}}
            public Obus.MethodState method_set_mode {get {return this._method_set_mode;} set {this.fields.method_set_mode = true; this._method_set_mode = value;}}

            public Info();
            public bool is_empty();
        }

        public unowned uint32[] pcpus {get {return this.info.pcpus;}}
        public uint32 task_total {get {return this.info.task_total;}}
        public uint32 task_running {get {return this.info.task_running;}}
        public uint32 task_sleeping {get {return this.info.task_sleeping;}}
        public uint32 task_stopped {get {return this.info.task_stopped;}}
        public uint32 task_zombie {get {return this.info.task_zombie;}}
        public uint32 refresh_rate {get {return this.info.refresh_rate;}}
        public Mode mode {get {return this.info.mode;}}
        public Obus.MethodState method_set_refresh_rate {get {return this.info.method_set_refresh_rate;}}
        public Obus.MethodState method_set_mode {get {return this.info.method_set_mode;}}

        private unowned Info* info {get;}
        public Obus.Handle handle {get;}
        public void* user_data {get; set;}

        public static unowned Summary @new(Obus.Server srv, ref Info info, Handlers handlers);
        public bool is_registered();
        [CCode (instance_pos = 1.2)]
        public int register(Obus.Server srv);
        [CCode (instance_pos = 1.2)]
        public int unregister(Obus.Server srv);
        [CCode (instance_pos = 1.2)]
        public int send_event(Obus.Server srv, EventType type, ref Info info);
        public static unowned Summary? from_handle(Obus.Server srv, Obus.Handle handle);
        public static unowned Summary? next(Obus.Server srv, Summary? prev);

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_set_refresh_rate_args_fields")]
        public struct SetRefreshRateArgsFields {
            public bool refresh_rate;
        }

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_set_refresh_rate_args")]
        public struct SetRefreshRateArgs {
            public SetRefreshRateArgsFields fields;

            [CCode (cname = "refresh_rate")]
            private uint32 _refresh_rate;

            public uint32 refresh_rate {get {return this._refresh_rate;}}
        }

        [CCode (cheader_filename = "ps_summary.h", cname = "ps_summary_method_set_refresh_rate_handler_t", has_target = false)]
        public delegate void SetRefreshRateHandler(Summary object, Obus.Handle handle, SetRefreshRateArgs args);

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_set_mode_args_fields")]
        public struct SetModeArgsFields {
            public bool mode;
        }

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_set_mode_args")]
        public struct SetModeArgs {
            public SetModeArgsFields fields;

            [CCode (cname = "mode")]
            private Mode _mode;

            public Mode mode {get {return this._mode;}}
        }

        [CCode (cheader_filename = "ps_summary.h", cname = "ps_summary_method_set_mode_handler_t", has_target = false)]
        public delegate void SetModeHandler(Summary object, Obus.Handle handle, SetModeArgs args);

        [Compact]
        [CCode (cheader_filename = "ps_summary.h", cname = "struct ps_summary_method_handlers")]
        public struct Handlers {
            public SetRefreshRateHandler method_set_refresh_rate;
            public SetModeHandler method_set_mode;
        }
    }
}
