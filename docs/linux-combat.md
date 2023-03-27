### Linux 性能优化实战

#### CPU篇
* 平均负载与cpu使用率
  * 平均负载：单位时间内，系统中处于可运行状态和不可中断状态的平均进程数
  * cpu使用率：%usr + %sys
  * 情况1：CPU密集型进程，CPU使用率和平均负载基本一致
  * 情况2：IO密集型进程，平均负载升高，CPU使用率不一定升高
  * 情况3：大量等待CPU的进程调度，平均负载升高，CPU使用率也升高

```
# 1.1 模拟一个cpu使用率100%的场景，cpu负载会达到1
stress --c 1 --timeout 600

# 1.2 -P ALL 表示监控所有CPU，后面数字5表示间隔5秒后输出一组数据
mpstat -P ALL 5 // 可以看到其中一个cpu的%usr是100%

# 1.3 每5秒输出一次，-u表示输出cpu指标
pidstat -u 5    // 可以找到是那个进程占cpu

# 2.1 模拟IO压力，cpu负载会达到1
stress -i 1 --timeout 600

# 2.2 可以看到一个cpu的%sys是24%，%iowait是68%
mpstat -P ALL 5 1

# 2.3 可以找到%CPU高的那个进程
pidstat -u 5

# 3.1 8个进程同时抢占cpu，此时cpu负载达到8
stress -c 8 --timeout 600

# 3.2 可以看到8个进程的%wait达到75%
pidstat -u 5
```