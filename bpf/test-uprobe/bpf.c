#include <linux/bpf.h>
#include "bpf/bpf_helpers.h"
char __license[] SEC("license") = "Dual MIT/GPL";

SEC("uprobe/go_test_greet")
int bpf_prog(void *ctx) {
    char buf[] = "Hello World!\n";
    bpf_trace_printk(buf, sizeof(buf));
    return 0;
}
