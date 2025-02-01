# dfu-handler
cd zephyrproject/zephyr
source zephyr-env.sh
source ~/zephyrproject/.venv/bin/activate
cd ../../myapp

west build -p -b nucleo_h743zi ~/zephyrproject/bootloader/mcuboot/boot/zephyr -d build-mcuboot
west build -p -b nucleo_h743zi image-0

st-flash --reset --connect-under-reset erase
st-flash --reset --connect-under-reset write build-mcuboot/zephyr/zephyr.bin 0x8000000
st-flash --reset --connect-under-reset write build/zephyr/zephyr.signed.bin 0x8040000

west build -p -b nucleo_h743zi image-1

dos2unix env_setup.sh
bash env_setup.sh
dos2unix test_init.sh
bash test_init.sh
dos2unix test.sh
bash test.sh

dos2unix test_swap.sh
bash test_swap.sh

cat uart_data.bin > /dev/ttyACM0


Testing commands:

cat test.bin > /dev/ttyACM0
cat test2.bin > /dev/ttyACM0
echo -n -e '\xFF\xFF\xFF\xFF' >> test.bin
echo -n -e '\x31' >> uart_data.bin

cat packet1.bin > /dev/ttyACM0
cat packet2.bin > /dev/ttyACM0

usbipd list
usbipd attach --wsl --busid 2-2

sudo minicom

include/zephyr/dfu/flash_img.h (flash_img_context struct)
include/zephyr/storage/stream_flash.h (stream_flash_ctx struct)
subsys/dfu/img_util/flash_img.c (flash_img)
subsys/storage/stream/stream_flash.c (stream_flash, flash_sync)

- Beam packets of binary from computer to board with UART
- Receive and store binary in slot-0, write the binary to slot-1
- Swap active boot to slot-1

- If flashing interupted, error code -5, even re-flashing breaks due to writing over existing data

- Check if time has elapsed, exit function with error
- Should program send confirmation when a packet is received

480 MHz

DATA=$(cat "$SERIAL_PORT")
DATA=$(head -c "$NUM_BYTES" < "$SERIAL_PORT")

- Checksum
- Send on ACK
- Packet counter
- Timeout