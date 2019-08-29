#!/bin/bash

kerImg="kernel.img"
kerElf="kernel.elf"
nCoreDeb="3333"

if [ $# -ge 1 ]; then
   kerImg=$1.img
   kerElf=$1.elf
fi
if [ $# -eq 2 ]; then
   nCoreDeb=$2
fi


openocd-rpi3 -c "" -f ./toolchain/share/openocd-rpi3/scripts/interface/jlink.cfg -f ./external/openOCD/config/rpi3_64.cfg  &> /dev/null &

sleep 1
aarch64-rpi3-elf-gdb \
  -ex 'set architecture aarch64' \
  -ex "file $kerElf" \
  -ex "target extended-remote localhost:$nCoreDeb" \
  -ex 'monitor reset init' \
  -ex 'monitor targets rpi3.cpu0' \
  -ex "monitor load_image $kerImg 0x80000 bin" \
  -ex 'monitor reg pc 0x80000' \
  -ex "load $kerElf" \
  -ex 'monitor targets' \
  -x gdbinit_launch_rpi

