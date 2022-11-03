#!/usr/bin/env bash

if [[ $1 ]]; then
    build_dir="$1"
else
    build_dir=build/PureRecovery.elf
fi

echo "arm-none-eabi-gdb "$build_dir" -x .gdbinit-1051"
arm-none-eabi-gdb "$build_dir" -x .gdbinit-1051
