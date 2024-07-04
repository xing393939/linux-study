### eBPF开发总结

#### 参考资料
1. [eBPF Docs](https://ebpf-docs.dylanreimerink.nl/)，介绍常用概念，函数的可用版本
1. [BPF 可移植性和 CO-RE](https://arthurchiao.art/blog/bpf-portability-and-co-re-zh/#%E6%96%B9%E5%BC%8F%E4%BA%8Clibbpf--bpf_prog_type_tracing%E4%B8%8D%E5%8F%AF%E7%A7%BB%E6%A4%8D)，5.5开始，若ebpf程序是BTF_PROG_TYPE_TRACING类型，内核可以直接利用BTF读取task->real_parent->pid
1. [Hello eBPF: Tail calls and your first eBPF application](https://mostlynerdless.de/blog/2024/02/12/hello-ebpf-tail-calls-and-your-first-ebpf-application-4/)，ebpf栈空间只有512B，利用尾调用缓解
1. libbpf开发，vmlinux.h没有宏定义，需要的话就引用bpf/bpf_helpers.h

#### 低版本生成内核BTF
1. 编译内核，生成带调试信息的vmlinux，编译前先`make clean && make mrproper`
1. `file vmlinux`可看到`with debug_info, not stripped`字样
1. 给vmlinux添加BTF段：`pahole -J vmlinux`
1. 生成btf：`pahole --btf_encode_detached external.btf vmlinux`
1. 查看btf：`pahole external.btf |grep 'struct task_struct {' -A 10`

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
1. BPF_CORE_READ，级联读取结构体字段
1. 其他CO-RE宏
  * bpf_core_read_str()：可以直接替换 Non-CO-RE 的 bpf_probe_read_str()
  * bpf_core_field_exists()：判断字段是否存在
  * bpf_core_field_size()：判断字段大小，同一字段在不同版本的内核中大小可能会发生变化
  * BPF_CORE_READ_STR_INTO()：与 BPF_CORE_READ_INTO() 类似，但会对最后一个字段执行 bpf_probe_read_str()
  * BPF_CORE_READ_BITFIELD()：通过直接内存读取（direct memory read）方式，读取比特位字段
  * BPF_CORE_READ_BITFIELD_PROBED()：底层会调用 bpf_probe_read()

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

// 3. BPF_CORE_READ用法
struct task_struct *task;
struct mm_struct *mm;
struct file *exe_file;
struct dentry *dentry;
const char *name;
bpf_core_read(&mm, 8, &task->mm);
bpf_core_read(&exe_file, 8, &mm->exe_file);
bpf_core_read(&dentry, 8, &exe_file->path.dentry);
bpf_core_read(&name, 8, &dentry->d_name.name);
name = BPF_CORE_READ(task, mm, exe_file, fpath.dentry, d_name.name); // 一行代码完成上述功能
```

#### BPF CO-RE原理
1. 四个组件：
  * BTF类型信息：用于获取内核、BPF 程序类型及 BPF 代码的关键信息
  * 编译器（clang/llvm）：给 BPF C 代码提供了表达能力和记录重定位（relocation）信息的能力
  * 用户态程序(libbpf)：将内核的 BTF 与 BPF 程序联系起来， 将编译之后的 BPF 代码适配到目标机器的特定内核
  * 内核：虽然对 BPF CO-RE 完全不感知，但提供了一些 BPF 高级特性，使某些高级场景成为可能。


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