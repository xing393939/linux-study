### 一台机器最多能支持多少条TCP连接

#### 最大打开文件数

| 级别    | 配置文件 | 查看 | 临时修改 | 永久修改 |
| --     | ----    | --  | ----    | ----   |
| 系统 | /etc/sysctl.conf          | sysctl -a | echo 9 > /proc/sys/fs/file-max | sysctl -w fs.file-max = 9 <br/>sysctl -p |
| 用户 | /etc/security/limits.conf | ulimit -a | ulimit -n 1024                  | 修改配置文件：<br/>\* hard nofile 65536<br/>\* soft nofile 65536|
| 进程 | /etc/sysctl.conf          | sysctl -a | echo 9 > /proc/sys/fs/nr_open | sysctl -w fs.nr_open = 9 <br/>sysctl -p |


