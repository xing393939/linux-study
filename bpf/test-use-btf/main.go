package main

import (
	"bytes"
	"encoding/binary"
	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/perf"
	"github.com/cilium/ebpf/rlimit"
	"log"
)
import "C"

//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -type event ebpf kprobe.c

func main() {
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	objs := ebpfObjects{}
	if err := loadEbpfObjects(&objs, nil); err != nil {
		log.Fatalf("loading objects: %v", err)
	}
	defer objs.Close()

	kp, err := link.Kprobe("sys_clone", objs.BpfProg, nil)
	if err != nil {
		log.Fatalf("opening kprobe: %s", err)
	}
	defer kp.Close()

	rd, err := perf.NewReader(objs.Events, 1024)
	if err != nil {
		log.Fatalf("opening ringbuf reader: %s", err)
	}
	defer rd.Close()
	var event ebpfEvent
	for {
		record, err := rd.Read()
		if err != nil {
			log.Printf("reading from reader: %s", err)
			return
		}

		// Parse the ringbuf event entry into a bpfEvent structure.
		if err := binary.Read(bytes.NewBuffer(record.RawSample), binary.LittleEndian, &event); err != nil {
			log.Printf("parsing ringbuf event: %s", err)
			continue
		}

		log.Printf("pid: %d\t tgid=%d\t comm: %s\n", event.Pid, event.Tgid, string(event.Comm[:]))
	}
}
