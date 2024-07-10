#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

#define ADC_PIN 26 // ADC0 aka GPIO26
#define TRIGGER_PIN 2 // GPIO2
#define SAMPLE_BUFFER_SIZE 32768 // choose your buffer size (logrithmic of 2), 2^(x=15)
#define ADC_CHANNEL 0 // ADC channels, pick from 0-3 (4 is reserved for temp. sensor)

#define Fs 50000.0 // Sample rate (Hz) (must not goes higer than 75 kSPS)
#define ADCCLK 48000000.0 // ADC clock rate (unmutable!)

// ---------------- preprocessor variable ----------------
// #define DEEBUGG
#define PRINT_BUFFER

volatile uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];
volatile uint32_t timestamp[SAMPLE_BUFFER_SIZE];
volatile uint16_t sample_index = 0;
volatile bool sampling_done = false;

// digital-to-voltage conversion
const float conversion_factor = 3.3f / (1 << 12); 

/*
    Callback function for the interrupt that enables the ADC sampling
    Parameter:
        uint gpio       - the operating pin
        uint32_t events - the event to trigger the callback fuction 
                          from the given pin, examples can be found on
                          pico c-sdk section 4.1.9.3

    Return:
        NULL
*/
void gpio_callback(uint gpio, uint32_t events) {
    if (sample_index < SAMPLE_BUFFER_SIZE) {
        // single ADC sample acquire
        sample_buffer[sample_index] = adc_read();
        // get timestamp in microsecond from timer API
        timestamp[sample_index] = time_us_32();

        sample_index++;

        // check if the sample index is out of the BUFFER bound
        if (sample_index >= SAMPLE_BUFFER_SIZE) {
            sampling_done = true;
        }
    }
}

// simple helper print function to print the BUFFER
void print_buffer(){
    // output the buffer over serial
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        uint16_t result = sample_buffer[i];
        uint32_t time = timestamp[i];
        printf("Raw value: %d, voltage: %f, at time: %d\n", result, result * conversion_factor, time);
    }
}

int main() {
    stdio_init_all(); // initialize stdio lib
    printf("Program started\n");

    // ----------------------------------------
    // ------------ Initialize ADC ------------
    // ----------------------------------------
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);
    adc_set_clkdiv(ADCCLK/Fs); // adjust the sampling rate
    printf("ADC initialized\n");

    // ----------------------------------------
    // ----- Triggering PIN for Reading -------
    // ----------------------------------------
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_IN);
    gpio_pull_up(TRIGGER_PIN);
    gpio_set_irq_enabled_with_callback(TRIGGER_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    printf("Trigger pin initialized\n");



    while (!sampling_done) {
        tight_loop_contents();
    }

#ifdef PRINT_BUFFER
    print_buffer();
#endif

    return 0;
}
