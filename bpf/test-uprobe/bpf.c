#define __TARGET_ARCH_x86 1
#include <linux/bpf.h>
#include <linux/ptrace.h>
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_tracing.h"

char __license[] SEC("license") = "Dual MIT/GPL";

SEC("uprobe/go_test_greet")
int bpf_prog(struct pt_regs *ctx) {
    char buf[] = "Hello %d %d \n";
    bpf_trace_printk(buf, sizeof(buf), PT_REGS_PARM1(ctx), PT_REGS_PARM2(ctx));
    return 0;
}
