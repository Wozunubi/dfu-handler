#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/drivers/uart.h>

#define XMODEM_SOH  0x01
#define XMODEM_EOT  0x04
#define XMODEM_ACK  0x06
#define XMODEM_NAK  0x15
#define XMODEM_CAN  0x18

#define PACKET_LEN  256

static int receive_packet(const struct device *uart_dev, uint16_t *packet_num, uint8_t *data) {
  uint8_t packet[PACKET_LEN+4];
  uint8_t checksum = 0;
  uint32_t start_time;
  int ret;

  for (int i = 0; i < sizeof(packet); i++) {
    start_time = k_uptime_seconds();

    while (uart_poll_in(uart_dev, &packet[i]) < 0) {
      k_yield();

      if (k_uptime_seconds() - start_time > 20) {
        return -ETIMEDOUT;
      }
    }
  }

  if (packet[0] != XMODEM_SOH) {
    return -EINVAL;
  }

  *packet_num = ((uint16_t)packet[1] << 8) | packet[2];

  for (int i = 0; i < PACKET_LEN; i++) {
    data[i] = packet[3 + i];
    checksum += data[i];
  }

  if (packet[3 + PACKET_LEN] != checksum) {
    return -EIO;
  }

  return 0;
}

void receive_file(const struct device *uart_dev, const struct flash_area *fa) {
  uint16_t packet_num, packet_counter = 0;
  uint8_t data[PACKET_LEN];
  int ret;

  while (true) {
    ret = receive_packet(uart_dev, &packet_num, data);
    packet_counter++;

    if (ret < 0) {
      printk("Error receiving frame: %d\n", ret);
      return;
    }

    if (packet_num == 0) { // End of transmission
      uart_poll_out(uart_dev, XMODEM_ACK);
      break;
    }

    if (packet_num == packet_counter) {
      ret = flash_area_write(fa, (packet_num - 1) * PACKET_LEN, data, PACKET_LEN);
      if (ret != 0) {
        printk("Flash write error: %d\n", ret);
        return;
      }

      uart_poll_out(uart_dev, XMODEM_ACK); // Send acknowledgment
    } else if (packet_num == packet_counter-1) {
      uart_poll_out(uart_dev, XMODEM_ACK);
    } else {
      uart_poll_out(uart_dev, XMODEM_CAN);
      return;
    }
  }

  printk("\nFile transfer complete\n\n");
}

int main(void) {
  const int VERSION = 0/*, PACKET_LEN = 128*/, NUM_PACKETS = 1024, TIMEOUT = 1; // in ms
  int ret, byte = 0, packet = 0;
  unsigned char recv_char, data[PACKET_LEN];

  const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
  const struct flash_area *fa;

  printk("version %d\n", VERSION);
  
  if (!device_is_ready(uart_dev)) {
    printk("uart device not ready\n");
    return -ENODEV;
  }

  ret = flash_area_open(FIXED_PARTITION_ID(slot1_partition), &fa);
  if (ret != 0) {
    printk("flash open error, %d\n", ret);
    return ret;
  }

  ret = flash_area_erase(fa, 0, NUM_PACKETS*PACKET_LEN);
  if (ret != 0) {
    printk("flash erase error, %d\n", ret);
    flash_area_close(fa);
    return ret;
  }

  printk("receiving data\n");

  receive_file(uart_dev, fa);

  /*while (packet < NUM_PACKETS) {
    while (uart_poll_in(uart_dev, &recv_char) < 0) {
      k_yield();
    }

    data[byte] = recv_char;
    byte++;
    
    if (byte >= PACKET_LEN) {
      unsigned char test = packet+48;
      uart_poll_out(uart_dev, test);

      ret = flash_area_write(fa, packet*PACKET_LEN, data, PACKET_LEN);
      if (ret != 0) {
        printk("flash write error, %d\n", ret);
        flash_area_close(fa);
        return ret;
      }

      byte = 0;
      packet++;
    }
  }*/

  flash_area_close(fa);

  //printk("flash complete\n");

  sys_reboot(SYS_REBOOT_COLD);

  return 0;
}
