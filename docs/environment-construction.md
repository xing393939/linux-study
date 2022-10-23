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

// 运行qemu，若开调试端口再加上“-s -S”
qemu-system-x86_64 -kernel ./arch/x86/boot/bzImage -initrd initramfs-busybox-x64.cpio.gz --append "console=ttyS0 nokaslr root=/dev/ram init=/init" -nographic

// vscode的launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "kernel-debug",
            "type": "cppdbg",
            "request": "launch",
            "miDebuggerServerAddress": "127.0.0.1:1234",
            "program": "${workspaceFolder}/vmlinux",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "logging": {
                "engineLogging": false
            },
            "MIMode": "gdb",
        }
    ]
}

// vscode的c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "linux-gcc-x64",
            "compileCommands": "${workspaceFolder}/compile_commands.json"
        }
    ],
    "version": 4
}
```

#### qemu相关
* [QEMU网络操作相关说明及常用命令](https://github.com/QthCN/opsguide_book/blob/master/QEMU%E7%BD%91%E7%BB%9C%E6%93%8D%E4%BD%9C%E7%9B%B8%E5%85%B3%E8%AF%B4%E6%98%8E%E5%8F%8A%E5%B8%B8%E7%94%A8%E5%91%BD%E4%BB%A4.md)
* qemu支持的网卡驱动：qemu-system-x86_64 -net nic,model=?
* qemu查看当前的网卡：(qemu) info network
* qemu快捷键：
  * ctrl-a c 切换monitor和console
  * ctrl-a x 退出qemu
  * ctrl-a h 查看帮助
  
#### busybox镜像制作
* [QEMU 启动 linux 内核和自制根文件系统](https://www.frytea.com/technology/unix-like/qemu-launch-linux-kernel-and-homemade-rootfs/)
* 也可以用buildboot制作

