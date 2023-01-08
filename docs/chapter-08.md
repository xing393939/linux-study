### 一台机器最多能支持多少条TCP连接

#### 最大打开文件数
* fs.file-max对root用户无效
* nofile一定要小于nr_open

| 级别    | 配置文件 | 查看 | 临时修改 | 永久修改 |
| --     | ----    | --  | ----    | ----   |
| 系统级别 | /etc/sysctl.conf           | sysctl -a | echo 9 > /proc/sys/fs/file-max | sysctl -w fs.file-max = 9 <br/>sysctl -p |
| 用户进程 | /etc/security/limits.conf  | ulimit -a | ulimit -n 1024                 | 修改配置文件：<br/>\* hard nofile 65536<br/>\* soft nofile 65536 |
| 进程级别 | /etc/sysctl.conf           | sysctl -a | echo 9 > /proc/sys/fs/nr_open  | sysctl -w fs.nr_open = 9 <br/>sysctl -p |

```
100万的设置：
sysctl -w net.ipv4.ip_local_port_range="5000 65000"
sysctl -w fs.file-max=1100000
sysctl -w fs.nr_open=1100000
sysctl -w net.core.somaxconn=4096
sysctl -w net.ipv4.tcp_syncookies=1
sysctl -w net.ipv4.tcp_max_orphans=1000000
sysctl -w net.ipv4.tcp_max_syn_backlog=4096
sysctl -w net.ipv4.tcp_mem="600000 800000 1000000"
sysctl -w net.ipv4.tcp_rmem="4096 4096 6291456"
sysctl -w net.ipv4.tcp_wmem="4096 4096 6291456"
sysctl -p

ulimit -n 1000000 

vim /etc/security/limits.conf
* hard nofile 1000000
* soft nofile 1000000
在新的终端生效

sysctl net.ipv4.ip_local_port_range fs.file-max fs.nr_open \
net.ipv4.tcp_max_orphans net.ipv4.tcp_max_syn_backlog \
net.ipv4.tcp_mem net.ipv4.tcp_rmem net.ipv4.tcp_wmem \
net.core.somaxconn net.ipv4.tcp_syncookies

换c6i.large
```

#### UDP和TCP端口可以相同
```
TCP：10.0.2.0向10.0.2.2的22端口发送数据，每次收包后寻找socket的流程
tcp_v4_rcv(skb)
|-sk = __inet_lookup_skb(&tcp_hashinfo, skb, ...)  
  |-__inet_lookup(dev_net(skb_dst(skb)->dev), hashinfo, skb, ...)
    |-__inet_lookup_established(net, hashinfo, ...)
      |-INET_MATCH(sk, net, acookie, saddr, daddr, ports, dif) // 比对saddr、sport、daddr、dport、网络设备

UDP的客户端代码：
sockfd = socket(AF_INET, SOCK_DGRAM, 0)
servaddr.sin_port = 5000;
servaddr.sin_addr.s_addr = "10.0.2.2";
  sendto(sockfd, hello1, strlen(hello1), MSG_CONFIRM, &servaddr, sizeof(servaddr))
recvfrom(sockfd, buffer, strlen(buffer), MSG_WAITALL, &servaddr, &len)

UDP的服务端代码：
sockfd = socket(AF_INET, SOCK_DGRAM, 0)
servaddr.sin_addr.s_addr = INADDR_ANY;
servaddr.sin_port = 5000;
bind(sockfd, &servaddr, sizeof(servaddr))
recvfrom(sockfd, buffer, strlen(buffer), MSG_WAITALL, &cliaddr, &len)
  sendto(sockfd, hello1, strlen(hello1), MSG_CONFIRM, &cliaddr, len)

UDP：10.0.2.0向10.0.2.2的5000端口发送数据，每次收包后寻找socket的流程
udp_rcv(skb)
|-__udp4_lib_rcv(skb, &udp_table, IPPROTO_UDP)
  |-sk = __udp4_lib_lookup_skb(skb, uh->source, uh->dest, udptable)
    |-__udp4_lib_lookup(dev_net(skb_dst(skb)->dev), ...)       // 比对saddr、sport、daddr、dport、网络设备
```