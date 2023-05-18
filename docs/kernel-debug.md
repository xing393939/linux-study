### 内核调试秘籍

#### 参考资料
* [内核调试秘籍](https://adtxl.com/index.php/archives/487.html)

#### printk
* 打印日志printk()
* 打印内存buffer的数据函数print_hex_dump()
* 打印堆栈函数dump_stack()

```
// 调试级别，若CONFIG_MESSAGE_LOGLEVEL_DEFAULT=8，则是打印所有
#define KERN_EMERG   KERN_SOH "0"    /* system is unusable */
#define KERN_ALERT   KERN_SOH "1"    /* action must be taken immediately */
#define KERN_CRIT    KERN_SOH "2"    /* critical conditions */
#define KERN_ERR     KERN_SOH "3"    /* error conditions */
#define KERN_WARNING KERN_SOH "4"    /* warning conditions */
#define KERN_NOTICE  KERN_SOH "5"    /* normal but significant condition */
#define KERN_INFO    KERN_SOH "6"    /* informational */
#define KERN_DEBUG   KERN_SOH "7"    /* debug-level messages */

// 使用方式
printk(KERN_EMERG "figo: %s, %d", __func__, __LINE__);
```

|常见类型|	printk格式符|
|---|---|
|int|	%d or %x|
|unsigned|	%u or %x|
|long|	%ld or %lx|
|long long|	%lld or %llx|
|unsigned long long|	%llu or %llx|
|size_t|	%zu or %zx|
|ssize_t|	%zd or %zx|
|函数指针|	%pf|













