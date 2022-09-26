### 第3章-内核是如何与用户进程协作的

####  socket的直接创建
```
socket的系统调用如下
inet_create(struct net * net, struct socket * sock, int protocol, int kern) (\home\ubuntu\linux-4.6.2\net\ipv4\af_inet.c:263)
__sock_create(struct net * net, int family, int type, int protocol, struct socket ** res, int kern) (\home\ubuntu\linux-4.6.2\net\socket.c:1160)
sock_create(int family, int type, int protocol, struct socket ** res) (\home\ubuntu\linux-4.6.2\net\socket.c:1200)

socket(AF_INET, SOCK_DGRAM, 0)                      // 创建UDP的socket
|-SYSC_socket(family = 2, type = 2, protocol = 0)
  |-sock_create(family, type, protocol, &sock)
    |-__sock_create(net, family, type, protocol, res, 0)
      |-sock = sock_alloc()                         // 创建socket对象
      |-pf = rcu_dereference(net_families[family])  // 通过family获取对应的协议族
      |-pf->create(net, sock, protocol, kern)       // 调用协议族的create函数，即inet_create
        |-inet_create(net, sock, protocol, kern)
          |-sock->ops = answer->ops;                // 设置socket->ops
          |-answer_prot = answer->prot;             // answer_prot=udp_prot
          |-sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot, kern)  // 创建sk对象，并设置sk.sk_prot=answer_prot
          |-sock_init_data(sock, sk)
            |-sk->sk_data_ready	= sock_def_readable // 软中断收到数据包会调用此函数唤醒等待的进程
```

#### 内核和用户进程协作之阻塞方式
```
read(fd, buf, 1024)        // 读取tcp socket上的数据
|-SYSC_read(fd, buf, 1024)
  |-vfs_read(f.file, buf, count, &pos)
    |-__vfs_read(file, buf, count, pos)
      |-new_sync_read(file, buf, count, pos)
        |-filp->f_op->read_iter(&kiocb, &iter)
          |-sock_recvmsg(sock, &msg, iov_iter_count(to), msg.msg_flags)
            |-sock_recvmsg_nosec(sock, msg, size, flags)
              |-sock->ops->recvmsg(sock, msg, size, flags) // 这里即是inet_recvmsg
                |-inet_recvmsg(sock, msg, size, flags)
                  |-sk->sk_prot->recvmsg(sk, msg, size, flags & MSG_DONTWAIT, flags & ~MSG_DONTWAIT, &addr_len)
                    |-tcp_recvmsg(sk, msg, size, noblock, flags, &addr_len) 
                      |-sk_wait_data(sk, &timeo, last)     // 没有收到足够数据，阻塞当前进程

sk_wait_data是如何阻塞当前进程的
#define DEFINE_WAIT_FUNC(name,function) wait_queue_t name = { 
    .private = current, 
    .func = function, 
    .task_list = LIST_HEAD_INIT((name).task_list) 
}

sk_wait_data(sk, &timeo, skb)
|-DEFINE_WAIT(wait) // 展开为：DEFINE_WAIT_FUNC(wait, autoremove_wake_function)，创建一个等待对象wait
|-sk_sleep(sk)      // 获取当前socket的等待列表
|-prepare_to_wait(sk_sleep(sk), &wait, TASK_INTERRUPTIBLE) // 把wait插入等待列表
|-sk_wait_event(sk, timeo, condition)                      // 让出CPU，进程将进入睡眠状态
  |-schedule_timeout(timeo)
    |-schedule()
```

#### 软中断模块
```
tcp的软中断在tcp_v4_rcv后会执行tcp_queue_rcv和sock_def_readable
tcp_queue_rcv(sk, skb, tcp_header_len, &fragstolen)
|-__skb_queue_tail(&sk->sk_receive_queue, skb)      // 把skb放在sk的接收链表的尾部

sock_def_readable(sk)
|-wq = rcu_dereference(sk->sk_wq)                   // 获取sk->sk_wq
|-wake_up_interruptible_sync_poll(&wq->wait, key)   // 即__wake_up_sync_key
  |-__wake_up_sync_key(&wq->wait, mode=1, nr_exclusive=1, key)
    |-__wake_up_common(q, mode, nr_exclusive, wake_flags, key)
      |-curr->func(curr, mode, wake_flags, key)     // 在define DEFINE_WAIT语句中，curr->func设置为了autoremove_wake_function

autoremove_wake_function(wait, mode, sync, key)
|-default_wake_function(wait, mode, sync, key)
  |-try_to_wake_up(curr->private, mode, wake_flags) // 唤醒因为等待而被阻塞的进程
```

#### 内核和用户进程协作之epoll

#### epoll内核对象的创建

#### 为epoll添加socket

#### epoll_wait之等待接收

#### 数据来了