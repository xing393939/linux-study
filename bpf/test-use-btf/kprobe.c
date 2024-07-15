//go:build ignore
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#define BPF_F_CURRENT_CPU		0xffffffffULL
enum bpf_map_type {
        BPF_MAP_TYPE_UNSPEC,
        BPF_MAP_TYPE_HASH,
        BPF_MAP_TYPE_ARRAY,
        BPF_MAP_TYPE_PROG_ARRAY,
        BPF_MAP_TYPE_PERF_EVENT_ARRAY,
};

char __license[] SEC("license") = "Dual MIT/GPL";

struct event {
    int pid;
    int tgid;
    int uid;
    int retval;
    __u8 comm[16];
};

struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

SEC("kprobe/sys_execve")
int bpf_prog(struct pt_regs *ctx) {
	struct event event={0};
    struct task_struct *task;
    task = (struct task_struct *) bpf_get_current_task();
    event.tgid = BPF_CORE_READ(task, tgid);
    event.pid = BPF_CORE_READ(task, pid);
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));

	return 0;
}
