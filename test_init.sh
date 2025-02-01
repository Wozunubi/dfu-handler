#! /usr/bin/bash
set -e

st-flash --reset --connect-under-reset erase

west build -p -b nucleo_h743zi ~/zephyrproject/bootloader/mcuboot/boot/zephyr -d build-mcuboot
st-flash --reset --connect-under-reset write build-mcuboot/zephyr/zephyr.bin 0x8000000

west build -p -b nucleo_h743zi image-0
st-flash --reset --connect-under-reset write build/zephyr/zephyr.signed.bin 0x8040000
