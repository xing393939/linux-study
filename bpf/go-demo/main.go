package main

import (
	"fmt"
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
	pid := syscall.Gettid()
	fmt.Printf("thread-%d %x %d \n", pid, g, *p)
	return *p
}

func main() {
	for i := 0; i < 10; i++ {
		go GetGoId()
	}
	time.Sleep(time.Millisecond)
	GetGoId()
}
