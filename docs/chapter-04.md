### 第4章-内核是如何发送网络包的

#### 网卡启动准备
```
第2章讲过网卡启动调用e1000_open->e1000_setup_all_tx_resources，以设置传输的ringBuffer和描述符数组

struct e1000_tx_ring {
	void *desc;
	struct e1000_tx_buffer *buffer_info;
};

e1000_setup_all_tx_resources(adapter)
|-e1000_setup_tx_resources(adapter, &adapter->tx_ring[i])
  |-tx_ring->tx_buffer_info = vzalloc(size);                                           // 这个数组是内核使用的
  |-tx_ring->desc = dma_alloc_coherent(dev, tx_ring->size, &tx_ring->dma, GFP_KERNEL); // 这个数组是网卡使用的
```

#### send系统调用实现
```
若MTU=1500，IP头长度为20，TCP头长度为20，则MSS=1500-20-20=1460

SYSCALL_DEFINE4(send, int, fd, void __user *, buff, size_t, len, unsigned int, flags)
|-sys_sendto(fd, buff, len, flags, NULL, 0)
  |-SYSCALL_DEFINE6(sendto, int, fd, void __user *, buff, size_t, len, unsigned int, flags, struct sockaddr __user *, addr, int, addr_len)
    |-sock = sockfd_lookup_light(fd, &err, &fput_needed)    // 根据fd找到socket
    |-struct msghdr msg                                     // 构造msghdr
    |-struct iovec iov                                      // 构造iovec
    |-sock_sendmsg(sock, &msg)                              // 发送数据
      |-sock_sendmsg_nosec(sock, msg)
        |-sock->ops->sendmsg(sock, msg, msg_data_left(msg)) // 即是AF_INET协议族的inet_sendmsg

inet_sendmsg(sock, msg, msg_data_left(msg))
|-sk->sk_prot->sendmsg(sk, msg, size)                       // 即是tcp_sendmsg
  |-tcp_sendmsg(sk, msg, size)
    |-skb = tcp_write_queue_tail(sk)                        // 获取发送队列中的最后一个skb
    |-sk_stream_alloc_skb(sk, select_size(sk, sg), sk->sk_allocation, skb_queue_empty(&sk->sk_write_queue)) // 申请skb
    |-skb_entail(sk, skb)                                   // 把skb挂到socket的发送队列
    |-skb_add_data_nocache(sk, skb, &msg->msg_iter, copy)   // 用户态数据拷贝到内核态
    |-if (forced_push(tp)) __tcp_push_pending_frames(sk, mss_now, TCP_NAGLE_PUSH) // 未发送数据是否已占最大窗口的一半
    |-if (skb == tcp_send_head(sk)) tcp_push_one(sk, mss_now)

```

#### 传输层处理
```
__tcp_push_pending_frames和tcp_push_one都会调用tcp_write_xmit

tcp_write_xmit(sk, mss_now, TCP_NAGLE_PUSH, 1, sk->sk_allocation)
|-......                                                   // 滑动窗口、拥塞控制
|-tcp_transmit_skb(sk, skb, 1, gfp)      
  |-skb = skb_clone(skb, gfp_mask)                         // skb浅拷贝
  |-th = tcp_hdr(skb)
  |-th->source		= inet->inet_sport
  |-th->dest		= inet->inet_dport                     // 设置skb的tcp头
  |-icsk = inet_csk(sk)
  |-icsk->icsk_af_ops->queue_xmit(sk, skb, &inet->cork.fl) // 即是ip_queue_xmit
```

#### 网络层发送处理
```
ip_queue_xmit(sk, skb, &inet->cork.fl)
|-rt = ip_route_output_ports(net, fl4, sk, ...)                        // 查看路由表，并设置到skb->_skb_refdst
|-iph = ip_hdr(skb)                                                    // 设置ip头 
|-ip_local_out(net, sk, skb)
  |-__ip_local_out(net, sk, skb)
    |-nf_hook(NFPROTO_IPV4, NF_INET_LOCAL_OUT, ...)                    // 执行netfilter过滤（基于iptables配置的规则）
  |-dst_output(net, sk, skb)
    |-skb_dst(skb)->output(net, sk, skb)                               // 调用路由表的output方法，即ip_output
      |-ip_output(net, sk, skb)
        |-NF_HOOK_COND(NFPROTO_IPV4, NF_INET_POST_ROUTING, net, sk, skb, NULL, dev, ip_finish_output, ...) // 执行netfilter过滤
          |-ip_finish_output(net, sk, skb)
            |-ip_fragment(net, sk, skb, mtu, ip_finish_output2)        // >MTU，先分片再发
            |-ip_finish_output2(net, sk, skb)                          // ≤MTU，直接发
              |-neigh = __ipv4_neigh_lookup_noref(dev, nexthop)        // 根据下一跳ip查找邻居项
              |-neigh = __neigh_create(&arp_tbl, &nexthop, dev, false) // 找不到就创建一个
              |-dst_neigh_output(dst, neigh, skb)                      // 向下层传递
```

#### 邻居子系统
```
struct neigh_table {
	struct neigh_hash_table *nht;  
};

struct neigh_hash_table {
	struct neighbour **hash_buckets;
}

struct neighbour {
	struct neighbour  *next;  // 链表指针
	struct net_device *dev;   // 设备
	unsigned char     ha[32]  // mac地址
};

__ipv4_neigh_lookup_noref(dev, nexthop)
|-___neigh_lookup_noref(&arp_tbl, neigh_key_eq32, arp_hashfn, &key, dev)
  |-nht = rcu_dereference_bh(tbl->nht)
  |-hash_val = hash(pkey, dev, nht->hash_rnd) >> (32 - nht->hash_shift)
  |-for (n = rcu_dereference_bh(nht->hash_buckets[hash_val]); n != NULL; n = rcu_dereference_bh(n->next)) {
        if (n->dev == dev && key_eq(n, pkey)) return n
    }

__neigh_create(&arp_tbl, &nexthop, dev, false)
|-n = neigh_alloc(tbl, dev)                          // 创建
|-n->dev = dev                                       // 赋值
|-rcu_assign_pointer(nht->hash_buckets[hash_val], n) // 添加到nht->hash_buckets哈希表中

dst_neigh_output(dst, neigh, skb)
|-n->output(n, skb)                                  // 即是neigh_resolve_output
  |-neigh_resolve_output(neigh, skb)                    
    |-neigh_event_send(neigh, skb)                   // 这里可能发送arp请求 
    |-dev_hard_header(skb, dev, neigh->ha, ...)      // 设置mac头
    |-dev_queue_xmit(skb)                            // 向下层传递  
```

#### 网络设备子系统

#### 软中断调度

#### e1000网卡驱动发送

#### RingBuffer内存回收

















