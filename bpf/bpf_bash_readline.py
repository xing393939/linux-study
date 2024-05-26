#!/usr/bin/env python2
from bcc import BPF
from time import strftime

chmod_prog = """
#include <uapi/linux/ptrace.h>

// perf event map (sharing data to user space)
struct data_t {
	u32 uid;
	char command[50];
};
BPF_PERF_OUTPUT(events);

int bash_readline(struct pt_regs *ctx)
{
	struct data_t data = {};
	data.uid = bpf_get_current_uid_gid();
	bpf_probe_read(data.command, 50, (void *)PT_REGS_RC(ctx));
	events.perf_submit(ctx, &data, sizeof(data));
	return 0;
}
"""

b = BPF(text=chmod_prog)
b.attach_uretprobe(name="/bin/bash", sym="readline", fn_name="bash_readline")
print("%-9s %-6s %s" % ("TIME", "UID", "COMMAND"))

def print_event(cpu, data, size):
    # event data struct is generated from "struct data_t" by bcc
    event = b["events"].event(data)
    print("%-9s %-6d %s" % (strftime("%H:%M:%S"), event.uid, event.command.decode("utf-8")))

b["events"].open_perf_buffer(print_event)
while 1:
    try:
        b.perf_buffer_poll()
    except KeyboardInterrupt:
        exit()