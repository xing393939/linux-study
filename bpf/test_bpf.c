#include <linux/bpf.h>
#include "bpf/bpf_helpers.h"
int bpf_prog(void *ctx) {
    char buf[] = "Hello World!\n";
    bpf_trace_printk(buf, sizeof(buf));
    return 0;
}
