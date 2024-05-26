from bcc import BPF

bpf_source = """
TRACEPOINT_PROBE(syscalls, sys_enter_fchmodat)
{
    bpf_trace_printk("%d %s\\n", args->dfd, args->filename);
    return 0;
}
"""

bpf = BPF(text = bpf_source)
bpf.trace_print()