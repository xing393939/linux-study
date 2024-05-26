#include <linux/bpf.h>

struct sock_key
{
__u32 sip; //源IP
__u32 dip; //目的IP
__u32 sport; //源端口
__u32 dport; //目的端口
__u32 family; //协议
};

struct bpf_map_def SEC("maps") sock_ops_map = {
.type = BPF_MAP_TYPE_SOCKHASH,
.key_size = sizeof(struct sock_key),
.value_size = sizeof(int),
.max_entries = 65535,
.map_flags = 0,
};