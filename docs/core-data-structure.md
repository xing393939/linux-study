### linux核心数据结构

#### struct sk_buff
* [Linux内核中sk_buff结构详解](https://www.jianshu.com/p/3738da62f5f6)
* [Printing sk_buff data](https://olegkutkov.me/2019/10/17/printing-sk_buff-data)
* 在include/linux/skbuff.h定义了一个打印函数pkt_hex_dump，然后[Hex Packet Decoder](https://hpd.gasmi.net/)
* 线性数据：head - end。
* 实际线性数据：data - tail，即ip包。
* skb->data_len: skb中的分片数据（非线性数据）的长度。
* skb->len: skb中的数据块的总长度。其中：skb->len = (data - tail) + skb->data_len

#### struct socket
```
struct socket {
    socket_state           state; // socket state (SS_CONNECTED, etc)
    short                  type;  // socket type (SOCK_STREAM, etc)
    unsigned long          flags; // socket flags (SOCK_NOSPACE, etc)
    struct socket_wq       *wq;   // wait queue for several uses
    struct file            *file; // File back pointer for gc
    struct sock            *sk;   // internal networking protocol agnostic socket representation
    const struct proto_ops *ops;  // protocol specific socket operations
};

typedef enum {
	SS_FREE = 0,            /* not allocated		*/
	SS_UNCONNECTED,         /* unconnected to any socket	*/
	SS_CONNECTING,          /* in process of connecting	*/
	SS_CONNECTED,           /* connected to socket		*/
	SS_DISCONNECTING        /* in process of disconnecting	*/
} socket_state;

enum sock_type {
	SOCK_STREAM    = 1,     /* tcp */
	SOCK_DGRAM     = 2,     /* udp */
	SOCK_RAW       = 3,
	SOCK_RDM       = 4,
	SOCK_SEQPACKET = 5,
	SOCK_DCCP      = 6,
	SOCK_PACKET    = 10,
};

// flags
#define SOCKWQ_ASYNC_NOSPACE   0
#define SOCKWQ_ASYNC_WAITDATA  1
#define SOCK_NOSPACE           2
#define SOCK_PASSCRED          3
#define SOCK_PASSSEC           4
```

#### struct sock
![img](../images/struct_sock.jpg)
```
// [include/linux/sock.h]
struct sock {
    int                 sk_rcvbuf;          // theorical "max" size of the receive buffer
    int                 sk_sndbuf;          // theorical "max" size of the send buffer
    atomic_t            sk_rmem_alloc;      // "current" size of the receive buffer
    atomic_t            sk_wmem_alloc;      // "current" size of the send buffer
    struct sk_buff_head sk_receive_queue;   // head of doubly-linked list，sk_buf缓冲区
    struct sk_buff_head sk_write_queue;     // head of doubly-linked list
    struct socket       *sk_socket;
}
```

