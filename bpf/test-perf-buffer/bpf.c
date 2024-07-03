#define __TARGET_ARCH_x86 1
#include <linux/bpf.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_tracing.h"

char __license[] SEC("license") = "Dual MIT/GPL";

struct event {
    int pid;
    int ppid;
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

SEC("uprobe/go_test_greet")
int bpf_prog(struct pt_regs *ctx) {
    __u64 id;
    int pid, tgid;
    struct event event={0};
    struct task_struct *task;

    int uid = (__u32)bpf_get_current_uid_gid();
    id = bpf_get_current_pid_tgid();
    tgid = id >> 32;

    event.pid = tgid;
    event.uid = uid;
    // task = (struct task_struct*)bpf_get_current_task(); // linux 4.4.0 不支持

    bpf_get_current_comm(&event.comm, sizeof(event.comm));
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    return 0;
}
