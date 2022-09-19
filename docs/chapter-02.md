### 第2章-内核是如何接收网络包的

#### 创建ksoftirqd内核线程
```
// kernel/softirq.c
struct smp_hotplug_thread softirq_threads = {
	.store			= &ksoftirqd,
	.thread_should_run	= ksoftirqd_should_run,
	.thread_fn		= run_ksoftirqd,
	.thread_comm		= "ksoftirqd/%u",
}

early_initcall(spawn_ksoftirqd)
|-spawn_ksoftirqd(void)
  |-smpboot_register_percpu_thread(&softirq_threads)
```

#### 网络子系统初始化
```
// net/core/dev.c
subsys_initcall(net_dev_init);
|-net_dev_init(void)
  |-open_softirq(NET_RX_SOFTIRQ, net_rx_action)

open_softirq为每一种软中断注册处理函数
```

#### 协议栈注册
```
// net/ipv4/af_inet.c
fs_initcall(inet_init)
|-inet_init(void)
  |-inet_add_protocol(&udp_protocol, IPPROTO_UDP) // 注册udp协议
  |-inet_add_protocol(&tcp_protocol, IPPROTO_TCP) // 注册tcp协议
  |-dev_add_pack(&ip_packet_type)                 // 注册ip协议

全局变量struct list_head ptype_base[16]
* 软中断会通过ptype_base找到ip协议的处理函数ip_rcv

全局变量struct net_protocol *inet_protos[256]
* ip_rcv()会通过inet_protos找到TCP的处理函数tcp_v4_rcv
```

#### 网卡驱动初始化
```
// drivers/net/ethernet/intel/e100.c
struct pci_driver e1000_driver = {
	.name     = e1000_driver_name,
	.id_table = e1000_pci_tbl,
	.probe    = e1000_probe,
	.remove   = e1000_remove
}

module_init(e1000_init_module)
|-e1000_init_module(void)
  |-pci_register_driver(&e1000_driver)

pci_register_driver调用完成后，内核就知道了该驱动的相关信息，当硬件e1000网卡被识别后，就会调用该驱动的probe方法（e1000_probe）
```














