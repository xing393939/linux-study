### 第3章-内核是如何与用户进程协作的

####  socket的直接创建
```
socket的系统调用如下
inet_create(struct net * net, struct socket * sock, int protocol, int kern) (\home\ubuntu\linux-4.6.2\net\ipv4\af_inet.c:263)
__sock_create(struct net * net, int family, int type, int protocol, struct socket ** res, int kern) (\home\ubuntu\linux-4.6.2\net\socket.c:1160)
sock_create(int family, int type, int protocol, struct socket ** res) (\home\ubuntu\linux-4.6.2\net\socket.c:1200)

socket(AF_INET, SOCK_DGRAM, 0) // 创建UDP的socket
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

#### 等待接收消息

#### 软中断模块

#### 内核和用户进程协作之epoll

#### epoll内核对象的创建

#### 为epoll添加socket

#### epoll_wait之等待接收

#### 数据来了