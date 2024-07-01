package main

import (
	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/rlimit"
	"io"
	"log"
	"os"
)
import "C"

func main() {
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	path := "/root/logs/test1/voteapi"
	ex, err := link.OpenExecutable(path)
	if err != nil {
		log.Fatal(err)
	}

	coll, err := ebpf.LoadCollection("tracker.o")
	if err != nil {
		log.Fatal("LoadCollection", err)
	}

	_, err = ex.Uprobe("main.main", coll.Programs["bpf_prog"], &link.UprobeOptions{})
	if err != nil {
		log.Fatal(err)
	}

	r, _ := os.Open("/sys/kernel/debug/tracing/trace_pipe")
	_, _ = io.Copy(os.Stdout, r)
}
