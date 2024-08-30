#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT                spi0
#define BUF_LEN                 0x3000  // buffer size: 12288
#define CLOCK_FREQUENCY         1000000 

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

    printf("The baudrate is at: %u\n Hz", spi_get_baudrate(SPI_PORT));

    uint16_t b16_num = 0x1000;
    // initialize output buffer
    for (size_t i = 0; i < BUF_LEN; ++i) {
        out_buf[i] = b16_num + i;
    }

    printf("SPI master says: The following buffer \
            will be written to MOSI endlessly:\n");
            
    // printbuf(out_buf, BUF_LEN);

    // indefinite loop
    for (size_t i = 0; ; ++i) {
        // write the output buffer to MOSI, and at the same time read from MISO.
        spi_write16_read16_blocking(SPI_PORT, out_buf, in_buf, BUF_LEN);
        
        // sleep for a little bit of time.
        sleep_ms(1000);
    }
#endif
}