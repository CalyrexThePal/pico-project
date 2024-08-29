#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

// define the SPI device
#define SPI_DEVICE      "/dev/spidev0.0"
#define BUFF_LEN        0x100 
#define CLOCK_FREQ      992063

int main() {
    // open the SPI device
    int spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return 1;
    }

    // set the SPI mode and clock speed
    uint8_t mode = 0;
    uint32_t speed = CLOCK_FREQ; // 20 MHz clock speed
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        return 1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI clock speed");
        return 1;
    }

    while(1){
        for(int i = 0; i < BUFF_LEN; i++) {
            // receive a byte of data
            uint16_t rx_data;
            struct spi_ioc_transfer transfer = {
                .tx_buf = (unsigned long)NULL,
                .rx_buf = (unsigned long)&rx_data,
                .len = 2,
                .speed_hz = speed,
                .bits_per_word = 16,
            };
            if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0){
                perror("Error receiving SPI data");
                return 1;
            }

            // print the received data
            printf("Received: 0x%04x\n", rx_data);

            // wait for a short period of time
            usleep(5);
        }

        printf("Simulate data writing...\n");
        sleep(5);
    }

    // close the SPI device
    close(spi_fd);

    return 0;
}