#include "pico/stdlib.h"
#include "hardware/spi.h"

// spi pins
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19

#define SIZE 5000

uint16_t buffer[SIZE];

// the spi interface
void send_data_via_spi(volatile uint16_t *buffer){
    
    // ------------- BASIC SPI CONFIG -------------
    spi_init(SPI_PORT, 500 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1); // deassert CS

    uint8_t spi_buffer[SIZE * 2];
    for (size_t i = 0; i < SIZE; i++){
        spi_buffer[2 * i] = buffer[i] & 0xFF;
        spi_buffer[2 * i + 1] = (buffer[i] >> 8) & 0xFF;
    }

    gpio_put(PIN_CS, 0); // assert CS
    spi_write_blocking(SPI_PORT, spi_buffer, SIZE * 2);
    gpio_put(PIN_CS, 1); // deassert CS
}

int main(){
    stdio_init_all(); // initialize stdio lib
    sleep_ms(8000);   // wait for USB initialization

    for (int i = 0; i < SIZE; i++){
        buffer[i] = i;
    }
    send_data_via_spi(buffer);
}