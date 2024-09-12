/*
    *** SPI receiver in master mode (include writing logic) ***
    
    About: 
        This program uses the Linux spidev infrastructure that directly maps
           to kernel space to make SPI transfers.
    Usage: 
        After properly configuring the SPI wiring,
        this program constantly makes single SPI transfers for BUFF_LEN
        times for one transfer cycle, and then writes the received data
        to a separate binary file.
           
    Compilation: 
        gcc -o SPI_wr SPI_wr.c
        ./SPI_wr

*/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

// define the SPI device
// #define SPI_DEVICE      "/dev/spidev0.0"
#define SPI_DEVICE      "/dev/spidev1.0"
#define BUFF_LEN        0x3000   // buffer size
#define CLOCK_FREQ      992063  // clock frequency, match this to the slave device baudrate

int main() {
    // open the SPI device
    int spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0){
        perror("Error opening SPI device");
        return 1;
    }

    // open the binary file for writing
    FILE *bin_file = fopen("data1.bin", "wb");
    if (bin_file == NULL) {
        perror("Error opening binary file");
        close(spi_fd);
        return 1;
    }

    // set the SPI mode and clock speed
    uint8_t mode = 0;
    uint32_t speed = CLOCK_FREQ;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        close(spi_fd);
        fclose(bin_file);
        return 1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI clock speed");
        close(spi_fd);
        fclose(bin_file);
        return 1;
    }

    uint16_t rx_data[BUFF_LEN];

    sleep(1); // sleep for some time

    // receive data from SPI for BUFF_LEN times
    for (int i = 0; i < BUFF_LEN; i++) {
        struct spi_ioc_transfer transfer = {
            .tx_buf = (unsigned long)NULL,
            .rx_buf = (unsigned long)&rx_data[i],
            .len = 2,
            .speed_hz = speed,
            .bits_per_word = 16,
        };
        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0){
            perror("Error receiving SPI data");
            close(spi_fd);
            fclose(bin_file);
            return 1;
        }
    }

    // write the received data to the binary file
    fwrite(rx_data, sizeof(uint16_t), BUFF_LEN, bin_file);

    // flush the file buffer to ensure data is written
    fflush(bin_file);

    printf("Data written to data.bin\n");

    sleep(1); // sleep for some time

    // close the SPI device and the binary file
    close(spi_fd);
    fclose(bin_file);

    return 0;
}
