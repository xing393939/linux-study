### 系统调用入口

#### 参考资料
* 内核版本：5.19.0
* [深入理解Linux系统调用与API](https://heapdump.cn/article/4424650)

对于64位进程汇编指令是syscall，对于32位可能是int 80、syscall(32)、sysenter
```c
// 对于syscall、syscall(32)、sysenter，在syscall_init初始化
void syscall_init(void)
{
  wrmsrl(MSR_LSTAR, (unsigned long)entry_SYSCALL_64);
  wrmsrl(MSR_CSTAR, (unsigned long)entry_SYSCALL_compat);
  wrmsrl_safe(MSR_IA32_SYSENTER_EIP, (u64)entry_SYSENTER_compat);
}
// 对于int 80，需要在中断向量表里面添加处理函数
tatic const __initconst struct idt_data def_idts[] = {
  SYSG(IA32_SYSCALL_VECTOR,  entry_INT80_compat),
}
```

entry_SYSCALL_64的代码如下：
```c
SYM_CODE_START(entry_SYSCALL_64)
  pushq  $__USER_DS                         /* pt_regs->ss */
  pushq  PER_CPU_VAR(cpu_tss_rw + TSS_sp2)  /* pt_regs->sp */
  pushq  %r11                               /* pt_regs->flags */
  pushq  $__USER_CS                         /* pt_regs->cs */
  pushq  %rcx                               /* pt_regs->ip */
  pushq  %rax                               /* pt_regs->orig_ax */
  call  do_syscall_64                       /* returns with IRQs disabled */
  sysretq
SYM_CODE_END(entry_SYSCALL_64)
```

do_syscall_64的代码如下：
```
__visible noinstr void do_syscall_64(struct pt_regs *regs, int nr)
{
	add_random_kstack_offset();
	nr = syscall_enter_from_user_mode(regs, nr);

	instrumentation_begin();

	if (!do_syscall_x64(regs, nr) && !do_syscall_x32(regs, nr) && nr != -1) {
		/* Invalid system call, but still a system call. */
		regs->ax = __x64_sys_ni_syscall(regs);
	}

	instrumentation_end();
	syscall_exit_to_user_mode(regs);
}
```

do_syscall_x64的代码如下
```c
static __always_inline bool do_syscall_x64(struct pt_regs *regs, int nr)
{
	unsigned int unr = nr;

	if (likely(unr < NR_syscalls)) {
		unr = array_index_nospec(unr, NR_syscalls);
		regs->ax = sys_call_table[unr](regs);
		return true;
	}
	return false;
}
```

sys_call_table是一个全局变量，/arch/x86/include/generated/asm/syscalls_64.h定义：
```
__SYSCALL(0, sys_read)   // 宏展开是__x64_sys_read
__SYSCALL(1, sys_write)  // 宏展开是__x64_sys_write
__SYSCALL(2, sys_open)   // 宏展开是__x64_sys_open
__SYSCALL(3, sys_close)  // 宏展开是__x64_sys_close
...
```

假设现在要看__x64_sys_close的代码定义，需要搜索`SYSCALL_DEFINE1(close`得到
```
// 这里宏展开后即是__x64_sys_close的函数定义
SYSCALL_DEFINE1(close, unsigned int, fd)
{
	int retval = close_fd(fd);

	/* can't restart close syscall because file table entry was cleared */
	if (unlikely(retval == -ERESTARTSYS ||
		     retval == -ERESTARTNOINTR ||
		     retval == -ERESTARTNOHAND ||
		     retval == -ERESTART_RESTARTBLOCK))
		retval = -EINTR;

	return retval;
}
```
