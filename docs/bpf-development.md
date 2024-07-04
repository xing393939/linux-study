### eBPF开发总结

#### 参考资料
1. [eBPF Docs](https://ebpf-docs.dylanreimerink.nl/)，介绍常用概念，函数的可用版本
1. [BPF 可移植性和 CO-RE](https://arthurchiao.art/blog/bpf-portability-and-co-re-zh/#%E6%96%B9%E5%BC%8F%E4%BA%8Clibbpf--bpf_prog_type_tracing%E4%B8%8D%E5%8F%AF%E7%A7%BB%E6%A4%8D)，5.5开始，若ebpf程序是BTF_PROG_TYPE_TRACING类型，内核可以直接利用BTF读取task->real_parent->pid
1. [Hello eBPF: Tail calls and your first eBPF application](https://mostlynerdless.de/blog/2024/02/12/hello-ebpf-tail-calls-and-your-first-ebpf-application-4/)，ebpf栈空间只有512B，利用尾调用缓解

#### 低版本生成内核BTF
1. 编译内核，生成带调试信息的vmlinux，编译前先`make clean && make mrproper`
1. `file vmlinux`可看到`with debug_info, not stripped`字样
1. 给vmlinux添加BTF段：`pahole -J vmlinux`
1. 生成btf：`pahole --btf_encode_detached external.btf vmlinux`

#### eBPF字节码学习
1. [【Learning eBPF-3】一个 eBPF 程序的深入剖析](https://www.cnblogs.com/lianyihong/p/18120323)，详细分析hello程序字节码
1. [eBPF Loader](https://fuweid.com/post/2022-ebpf-loader/)，利用BTF重定向Map和Struct_Field
1. [Towards truly portable eBPF](https://fuweid.com/post/2022-ebpf-portable-with-btfhub/)
  * 利用`__builtin_preserve_access_index`在编译阶段把task->real_parent->pid链接符号保存在.BTF.ext段
  * libbpf在加载期间利用BTF的信息修改指令
1. [BPF 程序（BPF Prog）类型详解](https://arthurchiao.art/blog/bpf-advanced-notes-1-zh/#%E4%BC%A0%E5%85%A5%E5%8F%82%E6%95%B0struct-__sk_buff-)，struct __sk_buff是BPF校验器提供的一个屏蔽sk_buff变化的稳定接口(支持版本未知)

#### libbpf+CO-RE
1. extern Kconfig variables。解决（4.6开始，task_struct.utime的单元由jiffies变为纳秒）
1. struct flavors。解决：
  * 5.14开始，task_struct.state更名为__state
  * 4.7开始，thread_struct.fs更名为fsbase

```
// 1. 4.6开始，task_struct.utime的单元由jiffies变为纳秒
extern u32 LINUX_KERNEL_VERSION __kconfig;
extern u32 CONFIG_HZ __kconfig;
u64 utime_ns;
if (LINUX_KERNEL_VERSION >= KERNEL_VERSION(4, 11, 0))
    utime_ns = BPF_CORE_READ(task, utime);
else
    utime_ns = BPF_CORE_READ(task, utime) * (1000000000UL / CONFIG_HZ);

// 2. 5.14开始，task_struct.state更名为__state
struct task_struct___old {
    ...
}
if (bpf_core_field_exists(t->__state)) {
    state = BPF_CORE_READ(t, __state);
} else {
    state = BPF_CORE_READ((struct task_struct___old *)t, state);
}
```

#### btftool技巧
```
// 查看所有程序
bpftool prog --json --pretty

// 查看指定id程序
bpftool prog show id 147 --json --pretty

// 查看指定id程序字节码
bpftool prog dump xlated id 147

// 查看所有map
bpftool map --json --pretty

// 查看指定id的map
bpftool map show id 206 --json --pretty

// 查看指定id的map具体内容
bpftool map dump id 206
```