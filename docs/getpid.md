### 系统调用之getpid

#### 参考资料
* [系统调用之getpid](https://blog.csdn.net/tiantao2012/article/details/80314731)

```c
SYSCALL_DEFINE0(getpid)
{
	#这里可以看是当前进程
	return task_tgid_vnr(current);
}
static inline pid_t task_tgid_vnr(struct task_struct *tsk)
{
	#走到这一步，就和gettid的实现一模一样
	return __task_pid_nr_ns(tsk, __PIDTYPE_TGID, NULL);
}
 
pid_t __task_pid_nr_ns(struct task_struct *task, enum pid_type type,
			struct pid_namespace *ns)
{
	pid_t nr = 0;
 
	rcu_read_lock();
	#这里我们没有指定namespace，因此用当前进程的namespace
	if (!ns)
		ns = task_active_pid_ns(current);
	#当前进程大部分情况下不为null
	if (likely(pid_alive(task))) {
	#这里形参的type就是__PIDTYPE_TGID，因此if条件成立，最终给type赋值为PIDTYPE_PID
		if (type != PIDTYPE_PID) {
			if (type == __PIDTYPE_TGID)
				type = PIDTYPE_PID;
			#得到这个线程组的领导线程
			task = task->group_leader;
		}
		#获取领导线程的pid
		nr = pid_nr_ns(rcu_dereference(task->pids[type].pid), ns);
	}
	rcu_read_unlock();
	#返回这个pid
	return nr;
}

```