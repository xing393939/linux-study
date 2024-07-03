//go:build ignore
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

SEC("kprobe/sys_execve")
int kprobe_execve() {
	__u64 id = bpf_get_current_pid_tgid();
	__u64 ppid;
    struct task_struct *task;
    task = (struct task_struct *) bpf_get_current_task();
    ppid = BPF_CORE_READ(task, tgid);

    char buf[] = "Hello %d %d \n";
    bpf_trace_printk(buf, sizeof(buf), id, ppid);

	return 0;
}
