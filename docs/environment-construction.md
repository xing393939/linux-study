### 环境搭建

#### 编译过程
```
参考文章：https://zhuanlan.zhihu.com/p/445453676
内核源码：https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.6.2.tar.xz
系统环境：x86-64、wsl的Ubuntu-18.04，内核4.19.104
编译环境：gcc-7.5.0、qemu-2.11.1、gdb-8.1.1

主要命令：
make menuconfig // 图形化的编译配置
make -j8        // 以8核的能力编译内核，output最后一行是Kernel: arch/x86/boot/bzImage is ready

遇到的两个问题：
1. undefined reference to ____ilog2_NaN，解决方案：https://blog.csdn.net/wlj1012/article/details/81626669
2. kernel does not support PIC mode，解决方案：https://blog.csdn.net/jasonLee_lijiaqi/article/details/84651138
```

#### debug环境配置
```
参考文章：https://wenfh2020.com/2021/12/03/ubuntu-qemu-linux/
最终目标：wsl中用qemu跑内核并打开调试端口1234，在windows的vscode打开wsl的源码目录，配置gdb远程调试
```


