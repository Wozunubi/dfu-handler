#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/drivers/uart.h>

int main(void) {
  const int VERSION = 2, PACKET_LEN = 256, NUM_PACKETS = 1024, TIMEOUT = 1; // in ms
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

  while (packet < NUM_PACKETS) {
    //sys_clock_announce(0);

    while (uart_poll_in(uart_dev, &recv_char) < 0) {
      k_yield();

      /*if (sys_clock_elapsed() > 5) {
        printk("timed out\n");
        flash_area_close(fa);
        return -ETIMEDOUT;
      }*/
    }    

    data[byte] = recv_char;
    byte++;
    
    if (byte >= PACKET_LEN) {
      ret = flash_area_write(fa, packet*PACKET_LEN, data, PACKET_LEN);
      if (ret != 0) {
        printk("flash write error, %d\n", ret);
        flash_area_close(fa);
        return ret;
      }

      byte = 0;
      packet++;
    }
  }

  /*for (int i = 0; i < 5; i++) {
    ret = flash_area_read(fa, i*len, read_data, len);

    if (ret != 0) {
      printk("there, %d\n", ret);
      flash_area_close(fa);
      return 0;
    }

    printk("chunk %d\n", i);

    for (int j = 0; j < len; j++) {
      printk("%d: 0x", j);
      if (read_data[j] < 0x10) printk("0");
      printk("%X\n", read_data[j]);
    }
  }*/

  flash_area_close(fa);

  printk("flash complete\n");

  sys_reboot(SYS_REBOOT_COLD);

  return 0;
}
