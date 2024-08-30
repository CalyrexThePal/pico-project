#include "hardware/spi.h"

#define SPI_PORT            spi0
#define BUFFER_SIZE         12000
#define CLOCK_FREQUENCY     1000000

/*
    Function: 
        send_data_via_spi: 
        - to send adc data to the master control via spi architecture
        - machine is set to slave mode
        - use all default spi pin, be careful with the wiring

    Parameter(s):
        data - incoming data buffer that's to be transffered

    Return: 
        none
*/
void send_data_via_spi(volatile uint16_t* buff) {
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

    printf("The baudrate is at: %u\n Hz", spi_get_baudrate(SPI_PORT));

    printf("SPI master says: The following buffer \
            will be written to MOSI endlessly:\n");

    // indefinite loop
    for (size_t i = 0; ; ++i) {
        // write the output buffer to MOSI, and at the same time read from MISO.
        spi_write16_blocking(SPI_PORT, (const uint16_t*)buff, BUFFER_SIZE);
        
        // sleep for a little bit of time.
        sleep_ms(1000);
    }
#endif
}
