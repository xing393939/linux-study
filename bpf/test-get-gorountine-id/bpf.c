#define __TARGET_ARCH_x86 1
#include <linux/bpf.h>
#include <linux/ptrace.h>
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_tracing.h"

char __license[] SEC("license") = "Dual MIT/GPL";

#if defined(__TARGET_ARCH_x86)
// https://go.googlesource.com/go/+/refs/heads/dev.regabi/src/cmd/compile/internal-abi.md#amd64-architecture
#define GOROUTINE_PTR(x) ((void*)(x)->r14)
#elif defined(__TARGET_ARCH_arm64)
// https://github.com/golang/go/blob/master/src/cmd/compile/abi-internal.md#arm64-architecture
#define GOROUTINE_PTR(x) ((void*)((PT_REGS_ARM64 *)(x))->regs[28])
#endif

SEC("uprobe/go_test_greet")
int bpf_prog(struct pt_regs *ctx) {
    void *goroutine_addr = GOROUTINE_PTR(ctx);
    long long int goroutine_id;
    __u64 arr = bpf_get_current_pid_tgid();
    __u32 pid = arr >> 32;

    bpf_probe_read(&goroutine_id, sizeof(goroutine_id), goroutine_addr+152);
    char buf[] = "thread-%d %lx %ld \n";
    bpf_trace_printk(buf, sizeof(buf), pid, goroutine_addr, goroutine_id);
    return 0;
}
