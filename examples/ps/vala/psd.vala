/******************************************************************************
 * @file psd.vala
 *
 * @brief obus ps server daemon written in vala
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

/**
 * Result of parsing process stat file.
 */
[Compact]
[CCode (has_type_id = false)]
private struct ProcessStat {
    public uint32 pid;          /* Pid of process */
    public string name;         /* Process name */
    public string exe;          /* Executable file path */
    public uint32 pcpu;         /* %CPU usage */

    public uint32 ppid;         /* Pid of parent process */
    public char state;          /* Single-char code for process state */

    public uint64 utime;        /* User-mode CPU time accumulated by process */
    public uint64 stime;        /* Kernel-mode CPU time accumulated by process */
    public uint64 cutime;       /* Cumulative utime of process and reaped children */
    public uint64 cstime;       /* Cumulative stime of process and reaped children */

    public ulong flags;         /* Kernel flags for the process */
    public ulong minflt;        /* Number of minor page faults since process start */
    public ulong majflt;        /* Number of major page faults since process start */
    public ulong cminflt;       /* Cumulative min_flt of process and child processes */
    public ulong cmajflt;       /* Cumulative maj_flt of process and child processes */

    public int pgrp;            /* Process group id */
    public int tpgid;           /* Terminal process group id */
    public int session;         /* Session id */
    public int tty_nr;          /* Full device number of controlling terminal */
}

/**
 * This structure stores one piece of 'history' information
  */
[Compact]
[CCode (has_type_id = false)]
private struct HistEntry {
    public uint32 pid;
    public uint64 tics;
}

/**
 This structure stores a frame's cpu tics used in history calculations.
*/
[Compact]
[CCode (has_type_id = false)]
private struct Cpu {
    /* Current values */
    public uint64 utime;        /* User mode */
    public uint64 ntime;        /* User mode with nice priority */
    public uint64 stime;        /* System mode */
    public uint64 itime;        /* Idle task */

    /* Saved values */
    public uint64 sav_utime;    /* User mode */
    public uint64 sav_ntime;    /* User mode with nice priority */
    public uint64 sav_stime;    /* System mode */
    public uint64 sav_itime;    /* Idle task */

    public uint id;             /* The CPU ID number */
    public uint32 pcpu;         /* %CPU usage */
}

/**
 * Context structure to compute %CPU
 */
[Compact]
[CCode (has_type_id = false)]
private struct PcpuContext {
    public uint32 task_total;
    public uint32 task_running;
    public uint32 task_sleeping;
    public uint32 task_stopped;
    public uint32 task_zombie;

    public Ps.Summary.Mode mode;
    public uint32 refresh_rate;

    public uint pcpu_max_value;
    public uint32 hertz;
    public uint32 cpu_tot;
    public Cpu[] cpus;

    private int64 et;
    private Posix.timeval oldtimev;
    private HistEntry[] _hist0;      /* History table */
    private HistEntry[] _hist1;      /* History table */
    private int _hist_new;           /* Index of table to use for new samples */
    private int _hist_old;           /* Index of table to use for old samples */
    private int _hist_size;          /* Allocated size of history table */

    /**
     */
    private unowned HistEntry[] hist_new {
        get {
            return this._hist_new == 0 ? this._hist0 : this._hist1;
        }
    }

    /**
     */
    private unowned HistEntry[] hist_old {
        get {
            return this._hist_old == 0 ? this._hist0 : this._hist1;
        }
    }

    /**
     */
    private void swap_hist() {
        var hist_tmp = this._hist_new;
        this._hist_new = this._hist_old;
        this._hist_old = hist_tmp;
    }

    /**
     */
    private void resize_hist(int size) {
        if (size > this._hist_size) {
            this._hist_size = (size + 31) & ~31;
            this._hist0.resize(this._hist_size);
            this._hist1.resize(this._hist_size);
        }
    }

    /**
     */
    private void sort_hist_old() {
        Posix.qsort(this.hist_old, this._hist_size, sizeof(HistEntry), (key1, key2) => {
            return ((int32)((HistEntry*)key1).pid) - (int32)(((HistEntry*)key2).pid);
        });
    }

    /**
     */
    private HistEntry* find_hist_old(uint32 pid) {
        var tmp = HistEntry() {pid = pid};
        HistEntry* ptr = Posix.bsearch(&tmp, this.hist_old, this._hist_size, sizeof(HistEntry), (key1, key2) => {
                return ((int32)((HistEntry*)key1).pid) - (int32)(((HistEntry*)key2).pid);
            });
        return ptr;
    }

    /**
     */
    public PcpuContext() {
        /* Mode :
          SOLARIS : %CPU can never go above 100 event if SMP
          IRIX    : %CPU can go up to ncpus * 100 */
        this.mode = Ps.Summary.Mode.IRIX;

        /* Refresh rate in seconds */
        this.refresh_rate = 3;

        this._hist_new = 0;
        this._hist_old = 1;
        this._hist_size = 0;
    }

    /**
     */
    public void adjust_pcpu_max_value() {
        /* Adjust max %CPU value */
        this.pcpu_max_value = 100;
        if (this.mode == Ps.Summary.Mode.IRIX)
            this.pcpu_max_value = 100 * this.cpu_tot;
    }

    /**
     */
    public void begin() {
        /* Elapsed time in us */
        var timev = Posix.timeval();
        timev.get_time_of_day();
        this.et = (timev.tv_sec - this.oldtimev.tv_sec) * 1000000LL;
        this.et += (timev.tv_usec - this.oldtimev.tv_usec);
        if (this.et < 0)
            this.et = 0;

        this.oldtimev.tv_sec = timev.tv_sec;
        this.oldtimev.tv_usec = timev.tv_usec;

        this.task_total = 0;
        this.task_running = 0;
        this.task_sleeping = 0;
        this.task_stopped = 0;
        this.task_zombie = 0;

        /* Swap history tables, sort old table for quick search in it */
        this.swap_hist();
        this.sort_hist_old();
    }

    /**
     */
    public void update(ref ProcessStat pst) {
        /* Update task by state */
        switch (pst.state) {
        case 'R':
            this.task_running++;
            break;
        case 'S':
        case 'D':
        case 'W':
            this.task_sleeping++;
            break;
        case 't':
        case 'T':
            this.task_stopped++;
            break;
        case 'Z':
            this.task_zombie++;
            break;
        default:
            GLib.warning("unknown task state %d", pst.state);
            break;
        }

        /* Increase array size if needed */
        this.resize_hist((int)this.task_total + 1);

        /* Calculate time in this process; the sum of user time (utime) and
           system time (stime) */
        uint64 tics = (pst.utime + pst.stime);
        this.hist_new[this.task_total].pid  = pst.pid;
        this.hist_new[this.task_total].tics = tics;

        /* Delta tics since last snapshot */
        var entry = this.find_hist_old(pst.pid);
        if (entry != null)
            tics -= entry.tics;

        /* Convert tics to percent */
        pst.pcpu = (this.et == 0) ? 0 : (uint32)(tics * 100 * 1000000ULL / (uint64)(this.hertz * this.et));
        if (this.mode == Ps.Summary.Mode.SOLARIS)
            pst.pcpu /= this.cpu_tot;
        if (pst.pcpu >= this.pcpu_max_value)
            pst.pcpu = this.pcpu_max_value;

        /* One more task in context */
        this.task_total++;
    }

    /**
     */
    public void finish() {
        char buf[512];

        /* Open '/proc/stat' file */
        var path = "/proc/stat";
        var fp = Posix.FILE.open(path, "r");
        if (fp == null) {
            GLib.warning("open(%s) : err=%d (%s)", path,
                    Posix.errno, Posix.strerror(Posix.errno));
            return;
        }

        /* First line is for all cpus */
        var cpu = &this.cpus[this.cpu_tot];
        if (fp.gets(buf) != null) {
            ((string)buf).scanf("cpu %llu %llu %llu %llu",
                    out cpu.utime, out cpu.ntime,
                    out cpu.stime, out cpu.itime);
            cpu.id = 0;
        }

        /* Per cpu data */
        for (uint32 i = 0; i < this.cpu_tot; i++) {
            cpu = &this.cpus[i];
            if (fp.gets(buf) != null) {
                ((string) buf).scanf("cpu%d %llu %llu %llu %llu",
                        out cpu.id,
                        out cpu.utime, out cpu.ntime,
                        out cpu.stime, out cpu.itime);
            }
        }

        /* For all cpus and summary */
        for (uint32 i = 0; i < this.cpu_tot + 1; i++) {
            uint64 tot, sav_tot, per_tot, per_itime;
            cpu = &this.cpus[i];
            tot = cpu.utime + cpu.ntime + cpu.stime + cpu.itime;
            sav_tot = cpu.sav_utime + cpu.sav_ntime + cpu.sav_stime + cpu.sav_itime;
            per_itime = cpu.itime - cpu.sav_itime;
            per_tot = tot - sav_tot;
            if (per_tot != 0)
                cpu.pcpu = (uint32)(100 - (100 * per_itime) / per_tot);

            /* Save values for next iteration */
            cpu.sav_utime = cpu.utime;
            cpu.sav_ntime = cpu.ntime;
            cpu.sav_stime = cpu.stime;
            cpu.sav_itime = cpu.itime;
        }

        //fp.close();
    }
}

/**
 */
public class App : GLib.Object {
    private GLib.MainLoop main_loop;
    private GLib.Source refresh_timer_source;
    private Obus.Server srv;
    private unowned Ps.Summary summary;
    private PcpuContext pcpu_ctx;

    /**
     */
    static int main(string[] args) {
        return new App(args).run();
    }

    /**
     * Constructor.
     * @param args : program arguments
     */
    private App(string[] args) {
        /* Initialize pcpu_ctx */
        this.pcpu_ctx = PcpuContext();

        /* Get number of cpu and tics per second */
        this.pcpu_ctx.cpu_tot = (uint32)Posix.sysconf(Linux._SC_NPROCESSORS_ONLN);
        this.pcpu_ctx.hertz = (uint32)Posix.sysconf(Posix._SC_CLK_TCK);
        GLib.message("cpu_tot=%u hertz=%u", this.pcpu_ctx.cpu_tot, this.pcpu_ctx.hertz);

        /* Adjust max %CPU value */
        this.pcpu_ctx.adjust_pcpu_max_value();

        /* Per cpu data (1 more for cumulative values) */
        this.pcpu_ctx.cpus = new Cpu[this.pcpu_ctx.cpu_tot + 1];

        /* Create the main loop */
        this.main_loop = new GLib.MainLoop(null, false);

        /* Create obus server */
        this.srv = new Obus.Server(Ps.desc);

        /* Setup peer connection callback */
        this.srv.set_peer_connection_cb((event, peer) => {
            GLib.message("peer (%s %s) : %s",
                    peer.address, peer.name, event.to_string());
            peer.user_data = this;
        });

        /* Summary handler */
        var summary_handlers = Ps.Summary.Handlers();

        /* Handler for 'method_set_refresh_rate' */
        summary_handlers.method_set_refresh_rate = (object, handle, args) => {
            App app = (App)object.user_data;
            unowned Obus.Peer peer = app.srv.get_call_peer(handle);
            GLib.message("peer (%s %s) : set_refresh_rate",
                    peer.address, peer.name);
            /* Check parameters */
            if (!args.fields.refresh_rate || args.refresh_rate <= 0) {
                app.srv.send_ack(handle, Obus.CallStatus.INVALID_ARGUMENTS);
                return;
            }
            /* Send call ack status */
            app.srv.send_ack(handle, Obus.CallStatus.ACKED);
            /* Nothing more to do if value does not change */
            if (app.pcpu_ctx.refresh_rate == args.refresh_rate)
                return;
            /* Update state */
            var info_new = Ps.Summary.Info();
            info_new.refresh_rate = args.refresh_rate;
            app.pcpu_ctx.refresh_rate = args.refresh_rate;
            app.summary.send_event(app.srv,
                    Ps.Summary.EventType.REFRESH_RATE_CHANGED, ref info_new);
            /* Update internal timer */
            app.update_refresh_timer(app.pcpu_ctx.refresh_rate);
        };

        /* Handler for 'method_set_mode' */
        summary_handlers.method_set_mode = (object, handle, args) => {
            App app = (App)object.user_data;
            unowned Obus.Peer peer = app.srv.get_call_peer(handle);
            GLib.message("peer (%s %s) : set_mode",
                    peer.address, peer.name);
            /* Check parameters */
            if (!args.fields.mode || !args.mode.is_valid()) {
                app.srv.send_ack(handle, Obus.CallStatus.INVALID_ARGUMENTS);
                return;
            }
            /* Send call ack status */
            app.srv.send_ack(handle, Obus.CallStatus.ACKED);
            /* Nothing more to do if value does not change */
            if (app.pcpu_ctx.mode == args.mode)
                return;
            /* Update state */
            var info_new = Ps.Summary.Info();
            info_new.mode = args.mode;
            app.pcpu_ctx.mode = args.mode;
            app.summary.send_event(app.srv,
                    Ps.Summary.EventType.MODE_CHANGED, ref info_new);
            /* Adjust max %CPU value */
            app.pcpu_ctx.adjust_pcpu_max_value();
        };

        /* Create summary object */
        var summary_info = Ps.Summary.Info();
        summary_info.refresh_rate = this.pcpu_ctx.refresh_rate;
        summary_info.mode = this.pcpu_ctx.mode;
        summary_info.method_set_refresh_rate = Obus.MethodState.ENABLED;
        summary_info.method_set_mode = Obus.MethodState.ENABLED;
        this.summary = Ps.Summary.new(this.srv, ref summary_info, summary_handlers);
        this.summary.register(this.srv);
        this.summary.user_data = this;

        /* Initial scan of process list */
        scan_process();

        /* Start the server */
        unowned string[] addrs = args[1:args.length];
        this.srv.start(addrs);
    }

    /**
     * Destructor.
     */
    ~App() {
        GLib.message("App::~App");
        /* This will automatically stop and destroy ths obus server */
        this.srv = null;
        this.summary = null;
    }

    /**
     */
    private GLib.Source create_signal_source(int signum) {
        var signal_source = new GLib.Unix.SignalSource(signum);
        signal_source.set_callback(() => {
            GLib.message("signal %d (%s) received !",
                    signum, GLib.strsignal(signum));
            this.main_loop.quit();
            return true;
        });
        return signal_source;
    }

    /**
     * Run the application.
     * @return application exit code.
     */
    private int run() {
        /* Create a source to catch SIGINT and SIGTERM */
        var signal_source1 = create_signal_source(Posix.SIGINT);
        var signal_source2 = create_signal_source(Posix.SIGTERM);
        signal_source1.attach(this.main_loop.get_context());
        signal_source2.attach(this.main_loop.get_context());

        /* Create a source for obus server and attach it to main loop */
        var io_channel = new GLib.IOChannel.unix_new(this.srv.fd());
        var io_source = new GLib.IOSource(io_channel, GLib.IOCondition.IN);
        io_source.set_callback((source, condition) => {
            this.srv.process_fd();
            return true;
        });
        io_source.attach(this.main_loop.get_context());

        /* Run the main loop */
        this.update_refresh_timer(this.pcpu_ctx.refresh_rate);
        this.main_loop.run();

        /* Remove refresh timer source */
        if (refresh_timer_source != null)
            refresh_timer_source.destroy();
        refresh_timer_source = null;

        /* Remove obus server source from main loop */
        io_source.destroy();

        /* Remove signal sources */
        signal_source1.destroy();
        signal_source2.destroy();
        return 0;
    }

    /**
     * Update the refresh timer.
     * @param rate : new refresh rate in seconds.
     */
    private void update_refresh_timer(uint rate) {
        /* Destroy current timer source if needed */
        if (refresh_timer_source != null)
            refresh_timer_source.destroy();
        /* Create new timer source and attach it to main loop */
        refresh_timer_source = new GLib.TimeoutSource(1000 * rate);
        refresh_timer_source.set_callback(() => {
            /* Update process list */
            this.scan_process();
            return true;
        });
        refresh_timer_source.attach(this.main_loop.get_context());
    }

    /**
     */
    private void scan_process() {
        /* Open '/proc' directory */
        var path = "/proc";
        var dir = Posix.opendir(path);
        if (dir == null) {
            GLib.warning("opendir(%s) : err=%d (%s)", path,
                    Posix.errno, Posix.strerror(Posix.errno));
            return;
        }

        /* Initialize pcpu computation */
        this.pcpu_ctx.begin();

        /* Create a bus event to aggregate all updates */
        var bus_event = new Ps.BusEvent(Ps.BusEvent.Type.UPDATED);

        /* Browse entries */
        unowned Posix.DirEnt dirent;
        while ((dirent = Posix.readdir(dir)) != null) {
            /* Skip '.' and '..' */
            unowned string name = (string)dirent.d_name;
            if (name == "." || name == "..")
                continue;

            /* Only keep directories */
            if (dirent.d_type != 4/*Posix.DT_DIR*/)
                continue;

            /* Only keep numeric folder as pid */
            int pid = int.parse(name);
            if (pid < 0)
                continue;

            /* Open '/proc/pid/stat' file */
            path = "/proc/%d/stat".printf(pid);
            int fd = Posix.open(path, Posix.O_RDONLY);
            if (fd < 0) {
                /* No log, process may have already died */
                continue;
            }

            /* Read its content */
            char buf[256];
            var readlen = Posix.read(fd, buf, buf.length - 1);
            if (readlen < 0) {
                GLib.warning("read : err=%d (%s)",
                        Posix.errno, Posix.strerror(Posix.errno));
                Posix.close(fd);
                continue;
            }

            /* Parse buffer */
            buf[readlen] = '\0';
            var pst = ProcessStat();
            parse_process_stat(pid, (string)buf, ref pst);

            /* Compute %CPU */
            this.pcpu_ctx.update(ref pst);

            /* Update process and free resources */
            update_process(pid, ref pst, bus_event);
            Posix.close(fd);
        }

        /* Close '/proc' directory */
        //dir.close();

        /* Finish pcpu computation */
        this.pcpu_ctx.finish();

        /* Update summary */
        update_summary(bus_event);

        /* Cleanup dead processes */
        cleanup_process_list(bus_event);

        /* Send bus event */
        /*if (!ps_bus_event_is_empty(bus_event)) */
        bus_event.send(this.srv);
    }

    /**
     */
    private void parse_process_stat(uint32 pid, string buf, ref ProcessStat pst) {
        /* Reset structure */
        pst.pid = pid;
        pst.pcpu = 0;

        /* Get process name (between parenthesis) */
        string tmpbuf = buf;
        int idx1 = tmpbuf.index_of_char('(');
        int idx2 = tmpbuf.last_index_of_char(')');
        if (idx1 != -1 && idx2 != -1) {
            if (idx2 > idx1) {
                /* Extract name, skip ') ' */
                pst.name = tmpbuf[idx1 + 1 : idx2];
                tmpbuf = tmpbuf[idx2 + 2 : tmpbuf.length];
            }
        }
        /* Make sure name is valid */
        if (pst.name == null)
            pst.name = "";

        /* Get exe path */
        char exe[1024];
        var path = "/proc/%u/exe".printf(pid);
        var linklen = Posix.readlink(path, exe);
        if (linklen < 0) {
            if (Posix.errno != Posix.EACCES && Posix.errno != Posix.ENOENT) {
                GLib.warning("readlink('%s') err=%d (%s)", path,
                        Posix.errno, Posix.strerror(Posix.errno));
            }
            pst.exe = "";
        } else {
            if (linklen >= exe.length)
                exe[exe.length - 1] = '\0';
            else
                exe[linklen] = '\0';
            pst.exe = (string)exe;
        }

        /* Parse buffer */
        tmpbuf.scanf("%c %d %d %d %d %d %lu %lu %lu %lu %lu %llu %llu %llu %llu",
                out pst.state, out pst.ppid, out pst.pgrp, out pst.session,
                out pst.tty_nr, out pst.tpgid, out pst.flags,
                out pst.minflt, out pst.cminflt, out pst.majflt, out pst.cmajflt,
                out pst.utime, out pst.stime, out pst.cutime, out pst.cstime);
    }

    /**
     */
    private Ps.Process.State convert_process_state(char state) {
        switch (state) {
        case 'R':
            return Ps.Process.State.RUNNING;
        case 'S':
        case 'D':
        case 'W':
            return Ps.Process.State.SLEEPING;
        case 't':
        case 'T':
            return Ps.Process.State.STOPPED;
        case 'Z':
            return Ps.Process.State.ZOMBIE;
        default:
            return Ps.Process.State.UNKNOWN;
        }
    }

    /**
     */
    private unowned Ps.Process? find_process(uint32 pid) {
        /* Walk list of process in obus server */
        unowned Ps.Process process = Ps.Process.next(this.srv, null);
        while (process != null) {
            if (process.pid == pid)
                return process;
            process = Ps.Process.next(this.srv, process);
        }

        /* Not found */
        return null;
    }

    /**
     */
    private void update_process(uint32 pid, ref ProcessStat pst, Ps.BusEvent bus_event) {

        /* Convert process state */
        var ps_state = convert_process_state(pst.state);

        /* Get current process_info structure */
        unowned Ps.Process process = find_process(pid);

        /* Fill new structure */
        var info_new = Ps.Process.Info();

        if (process == null || pst.pid != process.pid)
            info_new.pid = pst.pid;

        if (process == null || pst.ppid != process.ppid)
            info_new.ppid = pst.ppid;

        if (process == null || pst.name != process.name)
            info_new.name = pst.name;

        if (process == null || pst.exe != process.exe)
            info_new.exe = pst.exe;

        if (process == null || pst.pcpu != process.pcpu)
            info_new.pcpu = pst.pcpu;

        if (process == null || ps_state != process.state)
            info_new.state = ps_state;

        /* Create and register new object or send update event */
        if (process == null) {
            process = Ps.Process.new(this.srv, ref info_new);
            bus_event.register_process(process);
        } else if (!info_new.is_empty()) {
            bus_event.add_process_event(process,
                    Ps.Process.EventType.UPDATED, ref info_new);
        }
    }

    /**
     */
    private void cleanup_process_list(Ps.BusEvent bus_event) {
        /* Walk list of process in obus server */
        unowned Ps.Process process = Ps.Process.next(this.srv, null);
        while (process != null) {

            /* Get next entry (before removing current from list */
            unowned Ps.Process next = Ps.Process.next(this.srv, process);

            /* Check if process is still present */
            var st = Posix.Stat();
            var path = "/proc/%u/stat".printf(process.pid);
            if (Posix.stat(path, out st) < 0 && Posix.errno == Posix.ENOENT) {
                /* Unregister it in bus event*/
                bus_event.unregister_process(process);
            }

            /* Iterate */
            process = next;
        }
    }

    /**
     */
    private void update_summary(Ps.BusEvent bus_event) {
        /* For all cpus and summary */
        var pcpus_new = new uint32[this.pcpu_ctx.cpu_tot + 1];
        for (int i = 0; i < this.pcpu_ctx.cpu_tot + 1; i++) {
            pcpus_new[i] = this.pcpu_ctx.cpus[i].pcpu;
        }

        /* Init new info, get current one */
        var info_new = Ps.Summary.Info();
        bool pcpus_changed = false;

        /* Don't give total cpu in obus */
        if (this.summary.pcpus.length != this.pcpu_ctx.cpu_tot) {
            pcpus_changed = true;
        } else {
            for (int i = 0; i < this.pcpu_ctx.cpu_tot; i++) {
                if (this.summary.pcpus[i] != pcpus_new[i])
                    pcpus_changed = true;
                }
        }
        if (pcpus_changed)
            info_new.pcpus = pcpus_new[0:this.pcpu_ctx.cpu_tot];

        if (this.pcpu_ctx.task_total != this.summary.task_total)
            info_new.task_total = this.pcpu_ctx.task_total;

        if (this.pcpu_ctx.task_running != this.summary.task_running)
            info_new.task_running = this.pcpu_ctx.task_running;

        if (this.pcpu_ctx.task_sleeping != this.summary.task_sleeping)
            info_new.task_sleeping = this.pcpu_ctx.task_sleeping;

        if (this.pcpu_ctx.task_stopped != this.summary.task_stopped)
            info_new.task_stopped = this.pcpu_ctx.task_stopped;

        if (this.pcpu_ctx.task_zombie != this.summary.task_zombie)
            info_new.task_zombie = this.pcpu_ctx.task_zombie;

        if (!info_new.is_empty()) {
            bus_event.add_summary_event(this.summary,
                    Ps.Summary.EventType.UPDATED, ref info_new);
        }
    }
}

