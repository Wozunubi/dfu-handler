#! /usr/bin/bash
set -e

west build -p -b nucleo_h743zi image-1

rm temp-data/*
split -b 256 --numeric-suffixes --suffix-length=4 build/zephyr/zephyr.signed.confirmed.bin temp-data/uart_data

for i in $(seq -f "%04g" 0 1023)
do
	echo uart_data$i
	cat temp-data/uart_data$i > /dev/ttyACM0
	DATA=$(head -c 1 /dev/ttyACM0)
	echo -n $DATA > output.txt
	echo done
	sleep 10
done
