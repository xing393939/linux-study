#define _GNU_SOURCE
#define MAX_CHAR_SIZE 4096

#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/bpf.h>
#include <linux/version.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <errno.h>

struct ELFHeader {
    char padding1[40];
    unsigned long long int sh_off;
    char padding2[10];
    unsigned short int sh_size;
    unsigned short int sh_num;
};

struct ELFSectionHeader {
    char padding1[24];
    unsigned long long int sh_offset;
    unsigned long long int sh_size;
};

int read_file(char *file, unsigned char *buf) {
    int bfd = open(file, O_RDONLY);
    if (bfd < 0) {
        printf("open eBPF program error: %s\n", strerror(errno));
        exit(-1);
    }
    int n = read(bfd, buf, MAX_CHAR_SIZE);
    close(bfd);
    return n;
}

int read_bpf(unsigned char *buf) {
    int i, text_size;
    struct ELFHeader *elf = (struct ELFHeader *) buf;
    struct ELFSectionHeader *sec;
    for (i = 0; i < elf->sh_num; i++) {
        sec = (struct ELFSectionHeader *) (buf + elf->sh_off + i * elf->sh_size);
        if (sec->sh_offset == 64) {
            text_size = sec->sh_size;
        }
    }
    return text_size;
}

int main() {
    unsigned char buf[MAX_CHAR_SIZE] = {};
    struct bpf_insn *insn;
    union bpf_attr attr = {};
    unsigned char log_buf[MAX_CHAR_SIZE] = {};
    int ret;
    int efd;
    int pfd;
    int n;
    int i;
    struct perf_event_attr pattr = {};

    // 获取sys_enter_clone的id
    n = read_file("/sys/kernel/debug/tracing/events/syscalls/sys_enter_clone/id", buf);
    char *end = "\n";
    unsigned long long int config_id = strtoull((const char *) buf, &end, 10);

    // 获取bpf程序的text段内容
    n = read_file("./test_bpf.o", buf);
    n = read_bpf(buf);
    for (i = 64; i < 64 + n; ++i) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0)
            printf("\n");
    }

    insn = (struct bpf_insn *) (buf + 64);
    strcpy(attr.prog_name, "mymain");
    attr.prog_type = BPF_PROG_TYPE_TRACEPOINT;
    attr.insns = (unsigned long) insn;
    attr.insn_cnt = n / sizeof(struct bpf_insn);
    attr.license = (unsigned long) "GPL";
    attr.log_size = sizeof(log_buf);
    attr.log_buf = (unsigned long) log_buf;
    attr.log_level = 1;
    // see LINUX_VERSION_CODE in /usr/include/linux/version.h
    attr.kern_version = 331580;
    pfd = syscall(SYS_bpf, BPF_PROG_LOAD, &attr, sizeof(attr));
    if (pfd < 0) {
        printf("bpf syscall error: %s\n", strerror(errno));
        printf("log_buf = %s\n", log_buf);
        exit(-1);
    }

    pattr.type = PERF_TYPE_TRACEPOINT;
    pattr.sample_type = PERF_SAMPLE_RAW;
    pattr.sample_period = 1;
    pattr.wakeup_events = 1;
    pattr.config = config_id;
    pattr.size = sizeof(pattr);
    efd = syscall(SYS_perf_event_open, &pattr, -1, 0, -1, 0);
    if (efd < 0) {
        printf("perf_event_open error: %s\n", strerror(errno));
        exit(-1);
    }
    ret = ioctl(efd, PERF_EVENT_IOC_ENABLE, 0);
    if (ret < 0) {
        printf("PERF_EVENT_IOC_ENABLE error: %s\n", strerror(errno));
        exit(-1);
    }
    ret = ioctl(efd, PERF_EVENT_IOC_SET_BPF, pfd);
    if (ret < 0) {
        printf("PERF_EVENT_IOC_SET_BPF error: %s\n", strerror(errno));
        exit(-1);
    }
    system("cat /sys/kernel/debug/tracing/trace_pipe");
}
