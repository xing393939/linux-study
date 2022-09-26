### 核心数据结构

#### struct sk_buff
* [Linux内核中sk_buff结构详解](https://www.jianshu.com/p/3738da62f5f6)
* [Printing sk_buff data](https://olegkutkov.me/2019/10/17/printing-sk_buff-data)
* 在include/linux/skbuff.h定义了一个打印函数pkt_hex_dump，然后[Hex Packet Decoder](https://hpd.gasmi.net/)
* 线性数据：head - end。
* 实际线性数据：data - tail，即ip包。
* skb->data_len: skb中的分片数据（非线性数据）的长度。
* skb->len: skb中的数据块的总长度。其中：skb->len = (data - tail) + skb->data_len