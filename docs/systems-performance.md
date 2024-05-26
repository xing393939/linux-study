### 性能之巅

#### 资料
1. [作者的博客](https://www.brendangregg.com/)
1. [安装Systemtap - UCloud](https://docs.ucloud.cn/uhost/public/systemtap)

#### systemtap安装
```
1. apt install systemtap
2. stap-prep // 检查安装是否正确，并会提示如何安装
3. echo -e "deb http://ddebs.ubuntu.com xenial main restricted universe multiverse\ndeb http://ddebs.ubuntu.com xenial-updates main restricted universe multiverse\ndeb http://ddebs.ubuntu.com xenial-proposed main restricted universe multiverse" > /etc/apt/sources.list.d/ddebs.list
4. apt install ubuntu-dbgsym-keyring 或者 apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C8CAB6595FDFF622
5. apt update
6. apt-get install linux-image-$(uname -r)-dbgsym

// 源码安装（因为ubuntu自带的systemtap版本太老了）
apt remove systemtap
wget https://sourceware.org/systemtap/ftp/releases/systemtap-4.4.tar.gz
apt install g++ make libelf-dev libdw-dev
./configure && make && make install

// 检查命令（在ubuntu 16.04，内核4.4.0-1128-aws安装成功）
stap -ve 'probe begin { log("hello world") exit() }'
stap -ve 'probe vfs.read {printf("read performed\n"); exit()}'
stap -e 'probe kernel.function("sys_open") {log("hello world") exit()}'
```

#### systemtap使用
* [SystemTap使用技巧](https://segmentfault.com/a/1190000010774974)
* [统计函数执行耗时](https://lrita.github.io/2017/09/16/get-function-elapse/)
* [动态追踪技术之SystemTap](https://www.cnblogs.com/shuqin/p/13196585.html)

```
// systemtap
// 输出进程a.out的main函数
stap -L 'process("./a.out").function("main")'
// 输出进程a.out的所有函数
stap -L 'process("./a.out").function("*")'
// 统计进程a.out函数执行次数
vim funccount.stp
global top_funcs;
probe process("./a.out").function("*"){
  top_funcs[ppfunc()]++;
}
probe end{
  printf("\n%-40s %s\n", "FUNC", "COUNT");
  foreach(func in top_funcs-) 
    printf("%-40s %d\n", func, top_funcs[func]);
}
stap funccount.stp
```

#### 系统分析工具
* perf：用于分析哪些方法调用cpu比较高、cpu cache命中率、分支预测等
* valgrind：它的helgrind工具用于分析资源竞争
* gprofile：用于分析函数调用次数和耗时等(编译时需要带上-pg)，原理就是在函数入口、出口加hook，对源码是侵入式的。
* oprofile：用于分析cpu cache命中率等
* Dtrace：不支持linux，适合Solaris、MacOS、FreeBSD
* SystemTap：可以分析内核函数、用户函数

#### eBPF 常用的两个工具：BCC和bpftrace
* BCC：利用linux的eBPF功能，因此需要使用linux 3.15+/4.1+。
* bpftrace 类似于Dtrace的linux 翻版，支持使用自定义的脚本语言
* [eBPF 入门之初体验](https://zhuanlan.zhihu.com/p/347239769)

```
# 在wsl2的ubuntu18.04和ubuntu20.04安装成功
# 参考：https://oftime.net/2021/01/16/win-bpf/
1. 先安装linux-header，需要源码安装，我的内核版本是4.19.104-microsoft-standard
2. ubuntu20.04直接apt-get install bpfcc-tools
3. ubuntu18.04官方的bpfcc-tool版本太老，需要源码安装bpfcc-tools v0.12（选择src-with-submodule版本）
4. sudo mount -t debugfs debugfs /sys/kernel/debug

// 统计进程a.out函数执行次数
/usr/share/bcc/tools/funccount ./a.out:*
```

