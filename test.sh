#! /usr/bin/bash
west build -p -b nucleo_h743zi image-1

rm temp-data/*
split -b 256 --numeric-suffixes --suffix-length=4 build/zephyr/zephyr.signed.confirmed.bin temp-data/uart_data

stty -F /dev/ttyACM0 raw

for i in $(seq 0 1023); do
  hex1_num=$(printf "%02x" $(((i + 1) / 256)))
  hex2_num=$(printf "%02x" $(((i + 1) % 256)))
  dec_num=$(printf "%04g" $i)
  checksum=$(od -An -N 256 -t u1 -v "temp-data/uart_data$dec_num" | awk '{for (i=1; i<=16; i++) sum = (sum + $i) % 256} END {print sum}')
  hex_checksum=$(printf "%02x" $checksum)

  printf "\x01\x$hex1_num\x$hex2_num" > uart_data.bin
  cat temp-data/uart_data$dec_num >> uart_data.bin
  printf "\x$hex_checksum" >> uart_data.bin
  cat uart_data.bin > /dev/ttyACM0
  sleep 0.1
  echo $dec_num
  # echo $checksum
  # echo $hex_checksum
  #sleep 0.1

  #for j in $(seq 1 256); do
    # sum=$(od -An -N 256 -t u1 -v "temp-data/uart_data$dec_num" | awk '{for (i=1; i<=16; i++) sum = (sum + $i) % 256} END {print sum}')
    # printf "%d\n" "$sum"
    # echo $byte
    # echo
  #done
  # checksum=$(od -An -N 256 -t u1 "temp-data/uart_data$dec_num" | awk '{s += $1} END {print s % 256}')
  # hex_checksum=$(printf "%02x" $checksum)
  # printf "\x$checksum" > uart_data.bin
  # echo $checksum
  # hex_checksum=$(printf "%02x" $checksum)

  # printf "\x01\x$hex1_num\x$hex2_num" > uart_data.bin
  # cat temp-data/uart_data$dec_num >> uart_data.bin
  # printf "\x$hex_checksum" >> uart_data.bin
  # cat uart_data.bin > /dev/ttyACM0
  # echo $dec_num
  # echo $checksum
  # echo $hex_checksum
done

printf "\x01" > uart_data.bin
for i in $(seq 1 259); do
  printf "\x00" >> uart_data.bin
done
cat uart_data.bin > /dev/ttyACM0
