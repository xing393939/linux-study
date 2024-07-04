//go:build ignore
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

SEC("kprobe/sys_execve")
int kprobe_execve() {
	pid_t pid = bpf_get_current_pid_tgid();
	__u32 cpu;
    struct task_struct *task;
    task = (struct task_struct *) bpf_get_current_task();
    cpu = BPF_CORE_READ(task, cpu);

    char buf[] = "pid=%d cpu=%d \n";
    bpf_trace_printk(buf, sizeof(buf), pid, cpu);

	return 0;
}
