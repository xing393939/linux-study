### 第5章-深度理解本机网络IO

#### 跨机网络IO和本机网络IO
![img](../images/network_io.jpg)

#### 本机发送之网络层路由
```
网络层发送处理以ip_queue_xmit开始

ip_queue_xmit(sk, skb, &inet->cork.fl)
|-rt = ip_route_output_ports(net, fl4, sk, ...) 
  |-ip_route_output_flow(net, fl4, sk)
    |-__ip_route_output_key(net, flp4)
      |-__ip_route_output_key_hash(net, flp, -1)
        |-fib_lookup(net, fl4, &res, 0)
```

5.3.2本机IP路由
5.3.3网络设备子系统
5.3.4“驱动”程序
5.4本机接收过程













