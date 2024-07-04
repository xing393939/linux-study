### eBPF开发总结

#### 低版本生成内核BTF
1. 编译内核，生成带调试信息的vmlinux，编译前先`make clean && make mrproper`
1. `file vmlinux`可看到`with debug_info, not stripped`字样
1. 给vmlinux添加BTF段：`pahole -J vmlinux`
1. 生成btf：`pahole --btf_encode_detached external.btf vmlinux`

#### eBPF字节码学习

