### socket协议簇

#### socket协议簇
```
socket(int family, int type, int protocol)

(struct sock)->sk_family
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_INET6	10	/* IP version 6			*/
#define AF_NETLINK	16  /* Kernel user interface device */

AF_INET协议族的成员
static struct inet_protosw inetsw_array[] =
{
    {
        .type =       SOCK_STREAM,
        .protocol =   IPPROTO_TCP,
        .prot =       &tcp_prot,         // struct proto
        .ops =        &inet_stream_ops,  // struct proto_ops
        .flags =      INET_PROTOSW_PERMANENT | INET_PROTOSW_ICSK,
    },
    {
        .type =       SOCK_DGRAM,
        .protocol =   IPPROTO_UDP,
        .prot =       &udp_prot,         // struct proto 
        .ops =        &inet_dgram_ops,   // struct proto_ops
        .flags =      INET_PROTOSW_PERMANENT,
    },
    {
        .type =       SOCK_DGRAM,
        .protocol =   IPPROTO_ICMP,
        .prot =       &ping_prot,        // struct proto
        .ops =        &inet_dgram_ops,   // struct proto_ops
        .flags =      INET_PROTOSW_REUSE,
    },
    {
        .type =       SOCK_RAW,
        .protocol =   IPPROTO_IP,
        .prot =       &raw_prot,         // struct proto
        .ops =        &inet_sockraw_ops, // struct proto_ops
        .flags =      INET_PROTOSW_REUSE,
    }
}

(struct sock)->sk_protocol
#define IPPROTO_IP		0  /* Dummy protocol for TCP		*/
#define IPPROTO_ICMP	1  /* Internet Control Message Protocol	*/
#define IPPROTO_TCP		6  /* Dummy protocol for TCP		*/
#define IPPROTO_UDP		17 /* User Datagram Protocol		*/
```