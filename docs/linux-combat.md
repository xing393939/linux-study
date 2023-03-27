### Linux 性能优化实战

#### CPU篇
```
# 1.1 模拟一个cpu使用率100%的场景，cpu负载会达到1
stress --cpu 1 --timeout 600

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
```