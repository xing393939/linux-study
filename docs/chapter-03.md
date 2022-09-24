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
sk_wait_data(struct sock * sk, long * timeo, const struct sk_buff * skb) (\home\ubuntu\linux-4.6.2\net\core\sock.c:2028)
tcp_recvmsg(struct sock * sk, struct msghdr * msg, size_t len, int nonblock, int flags, int * addr_len) (\home\ubuntu\linux-4.6.2\net\ipv4\tcp.c:1758)
inet_recvmsg(struct socket * sock, struct msghdr * msg, size_t size, int flags) (\home\ubuntu\linux-4.6.2\net\ipv4\af_inet.c:771)
sock_recvmsg_nosec() (\home\ubuntu\linux-4.6.2\net\socket.c:714)
sock_recvmsg(struct socket * sock, struct msghdr * msg, size_t size, int flags) (\home\ubuntu\linux-4.6.2\net\socket.c:722)
sock_read_iter(struct kiocb * iocb, struct iov_iter * to) (\home\ubuntu\linux-4.6.2\net\socket.c:799)
new_sync_read(struct file * filp, char * buf, size_t len, loff_t * ppos) (\home\ubuntu\linux-4.6.2\fs\read_write.c:462)
__vfs_read(struct file * file, char * buf, size_t count, loff_t * pos) (\home\ubuntu\linux-4.6.2\fs\read_write.c:474)
vfs_read(struct file * file, char * buf, size_t count, loff_t * pos) (\home\ubuntu\linux-4.6.2\fs\read_write.c:494)

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
```

#### 等待接收消息

#### 软中断模块

#### 内核和用户进程协作之epoll

#### epoll内核对象的创建

#### 为epoll添加socket

#### epoll_wait之等待接收

#### 数据来了