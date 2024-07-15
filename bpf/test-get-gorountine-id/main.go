package main

import (
	"C"
	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/rlimit"
	"io"
	"log"
	"os"
)

//go:generate go run github.com/cilium/ebpf/cmd/bpf2go bpf bpf.c

func main() {
	path := "../go-demo/go-demo.out"
	ex, err := link.OpenExecutable(path)
	if err != nil {
		log.Fatal(err)
	}

	// Allow the current process to lock memory for eBPF resources.
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	// Load pre-compiled programs and maps into the kernel.
	objs := bpfObjects{}
	if err := loadBpfObjects(&objs, nil); err != nil {
		log.Fatalf("loading objects: %v", err)
	}
	defer objs.Close()

	_, err = ex.Uprobe("main.GetGoId", objs.BpfProg, &link.UprobeOptions{})
	if err != nil {
		log.Fatal(err)
	}

	r, _ := os.Open("/sys/kernel/debug/tracing/trace_pipe")
	_, _ = io.Copy(os.Stdout, r)
}
