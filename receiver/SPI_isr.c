/*
    About:
        This is a receiver program on Rasperry Pi 5. This program uses GPIO's as interrupt service
    and receives data as the master, and then essentially store the data received to bin file

    Compilation: 
        gcc -o SPI_isr SPI_isr.c -l wiringPi
*/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>

// Define the SPI devices
#define SPI0 "/dev/spidev0.0"
#define SPI1 "/dev/spidev1.0"

// Define the buffer size
#define BUFF_LEN 20005

// Define the clock frequency
#define CLOCK_FREQ 25000000

// Define the data folder
#define DATA_FOLDER "data"

// Define the GPIO pins for interrupt
#define GPIO_PIN0 22
#define GPIO_PIN1 27

// Function to create the data folder if it doesn't exist
void create_data_folder() {
    struct stat sb;
    if (stat(DATA_FOLDER, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        if (mkdir(DATA_FOLDER, 0777) == -1) {
            perror("Error creating data folder");
        }
    }
}

// Interrupt callback function for SPI0
void gpio_callback0(void) {

    printf("Interrupt Raised from pico A (GPIO %d)\n", GPIO_PIN0);
    delayMicroseconds(500); // this is really important, DON'T delete

    // Read data from SPI device 0
    int spi_fd = open(SPI0, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device 0");
        return;
    }

    // Set the SPI mode and clock speed
    uint8_t mode = 0;
    uint32_t speed = CLOCK_FREQ;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        close(spi_fd);
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI clock speed");
        close(spi_fd);
        return;
    }

    // Create a filename based on the current time
    char filename[50];
    sprintf(filename, "%s/data0%d.bin", DATA_FOLDER, millis());

    // Open the binary file
    FILE *bin_file = fopen(filename, "wb");
    if (bin_file == NULL) {
        perror("Error opening binary file");
        close(spi_fd);
        return;
    }

    // Receive data from the SPI device
    uint16_t rx_data[BUFF_LEN];
    for (int i = 0; i < BUFF_LEN; i++) {
        struct spi_ioc_transfer transfer = {
            .tx_buf = (unsigned long)NULL,
            .rx_buf = (unsigned long)&rx_data[i],
            .len = 2,
            .speed_hz = speed,
            .bits_per_word = 16,
        };
        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
            perror("Error receiving SPI data");
            fclose(bin_file);
            close(spi_fd);
            return;
        }
    }

    printf("Pico A Transfer Finished\n");

    fwrite(rx_data, sizeof(uint16_t), BUFF_LEN, bin_file);
    fflush(bin_file);
    fclose(bin_file);

    printf("Pico A Data Written\n");

    close(spi_fd);

    printf("Pico A SPI closed\n");
}

// Interrupt callback function for SPI1
void gpio_callback1(void) {

    printf("Interrupt Raised from pico B (GPIO %d)\n", GPIO_PIN1);
    delayMicroseconds(500);

    // Read data from SPI device 1
    int spi_fd = open(SPI1, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device 1");
        return;
    }

    // Set the SPI mode and clock speed
    uint8_t mode = 0;
    uint32_t speed = CLOCK_FREQ;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        close(spi_fd);
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI clock speed");
        close(spi_fd);
        return;
    }

    // Create a filename based on the current time
    char filename[50];
    sprintf(filename, "%s/data1%d.bin", DATA_FOLDER, millis());

    // Open the binary file
    FILE *bin_file = fopen(filename, "wb");
    if (bin_file == NULL) {
        perror("Error opening binary file");
        close(spi_fd);
        return;
    }

    // Receive data from the SPI device
    uint16_t rx_data[BUFF_LEN];
    for (int i = 0; i < BUFF_LEN; i++) {
        struct spi_ioc_transfer transfer = {
            .tx_buf = (unsigned long)NULL,
            .rx_buf = (unsigned long)&rx_data[i],
            .len = 2,
            .speed_hz = speed,
            .bits_per_word = 16,
        };
        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
            perror("Error receiving SPI data");
            fclose(bin_file);
            close(spi_fd);
            return;
        }
    }

    printf("Pico B Transfer Finished\n");

    fwrite(rx_data, sizeof(uint16_t), BUFF_LEN, bin_file);
    fflush(bin_file);
    fclose(bin_file);

    printf("Pico B Data Written\n");

    close(spi_fd);

    printf("Pico B SPI closed\n");
}

int main() {
    // Create the data folder if it doesn't exist
    create_data_folder();

    // Initialize WiringPi
    wiringPiSetupGpio();

    // Set up the GPIO pins for interrupt
    pinMode(GPIO_PIN0, INPUT);
    pinMode(GPIO_PIN1, INPUT);

    printf("Waiting for interrupt...\n");

    // Set up the interrupt callback functions
    wiringPiISR(GPIO_PIN0, INT_EDGE_RISING, gpio_callback0);
    wiringPiISR(GPIO_PIN1, INT_EDGE_RISING, gpio_callback1);

    // Main loop
    while (1) {}

    return 0;
}