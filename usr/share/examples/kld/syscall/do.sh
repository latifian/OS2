#!/bin/bash
sync
make clean
make
kldunload ./syscall.ko
kldload ./syscall.ko
