package main

import (
	"fmt"
	"os"
	"syscall"
	"time"
	"unsafe"
)

const gGoIdOffset = 152

func getg() uintptr

//go:noinline
func GetGoId() int64 {
	g := getg()
	p := (*int64)(unsafe.Pointer(g + gGoIdOffset))
	tid := syscall.Gettid()
	pid := os.Getpid()
	fmt.Printf("pid-%d thread-%d %x %d \n", pid, tid, g, *p)
	return *p
}

//go:noinline
func Add(a, b int32) int32 {
	c := a + b
	return c
}

func main() {
	Add(1, 2)
	for i := 0; i < 10; i++ {
		go GetGoId()
	}
	GetGoId()
	time.Sleep(time.Minute)
}
