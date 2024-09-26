#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

#define SPI_PORT                spi0
#define TRANSFER_PIN            4
#define BUF_LEN                 20000  // buffer size: 12288
#define CLOCK_FREQUENCY         25000000 

// helper function to print buffer
void printbuf(uint16_t buf[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

// Low-level sleep function in microseconds
void sleep_us_low_level(uint64_t us) {
    absolute_time_t end_time = make_timeout_time_us(us);
    busy_wait_until(end_time);
}

// Low-level sleep function in milliseconds
void sleep_ms_low_level(uint64_t ms) {
    sleep_us_low_level(ms * 1000);
}

int main() {
    // enable usb so we can print
    stdio_init_all();
    sleep_ms(3000);

#if !defined(spi_default) || \
    !defined(PICO_DEFAULT_SPI_SCK_PIN) || \
    !defined(PICO_DEFAULT_SPI_TX_PIN) || \
    !defined(PICO_DEFAULT_SPI_RX_PIN) || \
    !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_master example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI master example\n");

    // enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(SPI_PORT, CLOCK_FREQUENCY);
    spi_set_slave(SPI_PORT, true); // Set SPI0 to slave mode
    spi_set_format(SPI_PORT, 16, 0, 0, 0);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    uint16_t out_buf[BUF_LEN], in_buf[BUF_LEN];

    printf("The baudrate is at: %u Hz\n", spi_get_baudrate(SPI_PORT));

    // uint16_t b16_num = 0;
    // initialize output buffer
    for (uint16_t i = 0; i < BUF_LEN; i++) {
        out_buf[i] = 641;
    }

    gpio_init(TRANSFER_PIN);
    gpio_set_dir(TRANSFER_PIN, GPIO_OUT); // impedence low

    // indefinite loop
    for (size_t i = 0; ; ++i) {
        // generate signal sending to masterboard
        gpio_put(TRANSFER_PIN, 1);  // set GPIO pin HIGH
        sleep_ms_low_level(100);
        gpio_put(TRANSFER_PIN, 0);  // set GPIO pin LOW

        printf("Starts stransferrring\n");

        // write the output buffer to MOSI, and at the same time read from MISO.
        int bytes_written = spi_write16_blocking(SPI_PORT, out_buf, BUF_LEN);

        printf("Bytes written: %d\n", bytes_written);
        
        // sleep for a little bit of time.
        sleep_ms_low_level(5000);
    }

    printf("Buffer finished transferring\n");
    
#endif

    return 0;
}