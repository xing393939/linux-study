### 网络性能优化建议

#### 网络请求优化
1. 减少网络io，如果能直接请求api，就不要proxy
1. 合并网络请求，把N次get改成hmget
1. 调用者和被调用者在同一个内网
1. 内网调用不要用外网域名

#### 接收过程优化
1. 调整网卡ringBuffer大小
  * ethtool -g eth0 // pre-set maxinums是允许的最大配置，current hardware settings是当前配置
  * ethtool -S eth0 // 若rx_fifo_errors不为0，说明有丢包情况
1. 多队列网卡
  * ethtool -l eth0      // Combined表示网卡队列数量
  * cat /proc/interrupts // 查看各cpu在各中断号的处理次数
1. 硬中断合并：ethtool -c eth0
  * Adapter RX：自适应硬中断合并
  * rx-usecs：多少微秒后产生一个中断
  * rx_frames：多少帧后产生一个中断
1. 软中断budget：net.core.netdev_budget，最多处理这么多包后让出CPU
1. 接收处理合并：ethtool -k eth0
  * generic-receive_offload：在内核合并包
  * large_receive_offload：在网卡就合并包，需网卡支持

#### 发送过程优化
1. 控制数据包小于MTU
1. 减少内存拷贝：mmap、sendfile
1. 推迟分片：ethtool -k eth0
  * generic-segmentation-offload：在网络设备子系统分包
  * tcp-segmentation-offload：在网卡进行分包，需网卡支持
1. 多队列网卡XPS：
  * `for i in $(ls -1 /sys/class/net/eth0/queues/rx*/rps_*); do echo -n ${i}: && cat ${i}; done`
  * rps_cpus：当前队列所绑定的cpu
  * rps_flow_cnt：当前队列发送的帧数
1. 用ebpf绕开本机IO

#### 内核与进程协作优化
1. 少用进程阻塞的方式
1. 使用IO多路复用和reactor模式
1. 使用kernel-by-pass，减少上下文切换、内存拷贝


