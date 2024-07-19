#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

#define BUFFER_SIZE 32768

// -------------------- UART Config ----------------------
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// -------------------- I2C Config -----------------------
#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_ADDR 0x08  // I2C address of the receiver (Arduino)

// -------------------- SPI Config -----------------------
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19

/*
    Data sending function via UART interface, configure UART TX and RX pins
    and transfter the data in two-bytes format
    Parameter:
        data    -   the buffer address
    Return:
        NULL
*/
void send_data_uart(volatile uint16_t* data) {
    uart_init(UART_ID, BAUD_RATE); // initialize UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // set GPIO0 as UART TX
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // set GPIO1 as UART RX

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        uart_putc_raw(UART_ID, data[i] & 0xFF);        // send lower byte
        uart_putc_raw(UART_ID, (data[i] >> 8) & 0xFF); // send upper byte
    }
}

// similar function like the UART example but use i2c interface instead
void send_data_via_i2c(volatile uint16_t* data) {
    i2c_init(I2C_PORT, 100 * 1000);  // initialize I2C at 100kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        uint8_t lower_byte = data[i] & 0xFF;
        uint8_t upper_byte = (data[i] >> 8) & 0xFF;

        i2c_write_blocking(I2C_PORT, I2C_ADDR, &lower_byte, 1, true);
        i2c_write_blocking(I2C_PORT, I2C_ADDR, &upper_byte, 1, false);
    }
}

// the spi interface example
void send_data_via_spi(volatile uint16_t* data) {
    spi_init(SPI_PORT, 500 * 1000);  // initialize SPI at 500kHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);  // deassert CS

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        uint8_t lower_byte = data[i] & 0xFF;
        uint8_t upper_byte = (data[i] >> 8) & 0xFF;

        gpio_put(PIN_CS, 0);  // assert CS
        spi_write_blocking(SPI_PORT, &lower_byte, 1);  
        spi_write_blocking(SPI_PORT, &upper_byte, 1);  
        gpio_put(PIN_CS, 1);  // deassert CS
    }
}
