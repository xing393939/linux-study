#!/usr/bin/env python2
from bcc import BPF
from bcc.utils import printb

chmod_prog = """
#include <uapi/linux/ptrace.h>

// perf event map (sharing data to user space)
struct data_t {
	u32 pid;
	char filename[50];
};
BPF_PERF_OUTPUT(events);

TRACEPOINT_PROBE(syscalls, sys_enter_fchmodat)
{
	struct data_t data = {};
	data.pid = bpf_get_current_pid_tgid();
	bpf_probe_read(data.filename, 50, args->filename);
	events.perf_submit(args, &data, sizeof(data));
	return 0;
}
"""

b = BPF(text=chmod_prog)
print("%-6s %-16s" % ("PID", "COMM"))

def print_event(cpu, data, size):
    # event data struct is generated from "struct data_t" by bcc
    event = b["events"].event(data)
    printb(b"%-6d %-16s" % (event.pid, event.filename))

b["events"].open_perf_buffer(print_event)
while 1:
    try:
        b.perf_buffer_poll()
    except KeyboardInterrupt:
        exit()