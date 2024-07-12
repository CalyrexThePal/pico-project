#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "adc_multi.h"

#define SENDER_PIN 8 // GPIO-8 - pin used to send flag signal for unstalling main
#define RECEIVER_PIN 9 // GPIO-9 - pin used to receive flag signal for unstalling main

/*
    ADC configs:
        - set Analog channel
        - set sampling rate
*/
#define ADC_PULSE_PIN 2 // GPIO-2 - pin pulled to receive pulse signal for ADC read
#define ADC_PIN 26 // ADC0 aka GPIO-26, pin corresponding to adc unit
#define ADC_CHANNEL 0 // ADC channels, pick from 0-3 (4 is reserved for temp. sensor)

// choose your buffer size (power of 2), 2^(x=15) to avoid page fault
#define SAMPLE_BUFFER_SIZE 32768 
#define BUFFER_THRESHOLD 22222

#define Fs 50000.0 // Sample rate (Hz) (must not goes higer than 75 kSPS)
#define ADCCLK 48000000.0 // ADC clock rate (unmutable!)

#define MACHINES_EMPLOYED 3 // how many pico we are using

// ---------------- Preprocessor variable ----------------
// #define PRINT_BUFFER_TEST
// #define RECORD_TIME

// choose the transfer interface you're using
#define USB_TR
#define UART_TR
// #define I2C_TR
// #define SPI_TR

// ------------------- Buffer Config ---------------------
volatile uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];
volatile uint32_t timestamp[SAMPLE_BUFFER_SIZE];
volatile uint16_t sample_index = 0;
volatile bool sampling_done = false;

// ------------ ADJUST the following variable ------------
// indicate the current machine, useful for debugging. CHANGE ACCORDINGLY
unsigned short machine_state = 1; 
// machine 1 starts off unlocked, the rest starts off locked. CHANGE ACCORDINGLY
volatile bool lock = false; 

// digital-to-voltage conversion
const float conversion_factor = 3.3f / (1 << 12); 

// *************************** Program Starts ***************************

/*
    Callback function to unlock the stalling flag before reading
*/
void unlock_trigger_callback(uint gpio, uint32_t events) {
    lock = false;
}

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
void ADC_trigger_callback(uint gpio, uint32_t events) {
    if (sample_index < SAMPLE_BUFFER_SIZE) {
        // single ADC sample acquire
        sample_buffer[sample_index] = adc_read();

#ifdef RECORD_TIME
        timestamp[sample_index] = time_us_32(); // get timestamp in microsecond
#endif
        sample_index++;

        // check if the index exceeds certain threshold
        if (sample_index == BUFFER_THRESHOLD){
            // set out-mode for sender pin, and pull up for irq sending
            gpio_set_dir(SENDER_PIN, GPIO_OUT);

            // generate signal sending to machine 2
            gpio_put(SENDER_PIN, 1);  // set GPIO pin HIGH
            sleep_us(100);  // wait for pulse duration
            gpio_put(SENDER_PIN, 0);  // set GPIO pin LOW
        }

        // check if the sample index is out of the BUFFER bound
        if (sample_index >= SAMPLE_BUFFER_SIZE) {
            sampling_done = true;
        }
    }
}


int clear_buffer(){
    // TODO: implement buffer clearing
    return 0;
}

// simple helper print function to print the BUFFER
void print_buffer(){
    // output the buffer over UART serial
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        uint16_t result = sample_buffer[i];
#ifdef RECORD_TIME
        uint32_t time = timestamp[i];
        printf("Raw value: %d, voltage: %f, at time: %d\n", result, result * conversion_factor, time);
#else
        printf("Raw value: %d, voltage: %f", result, result * conversion_factor);
#endif
    }
}

int main() {
    stdio_init_all(); // initialize stdio lib
    printf("Program started\n");

    // initialize all the operating pin
    gpio_init(ADC_PULSE_PIN);
    gpio_init(RECEIVER_PIN);
    gpio_init(SENDER_PIN);
    adc_init();
    printf("Operating PIN's initialized\n");

    // initialize ADC configurations
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);
    adc_set_clkdiv(ADCCLK/Fs); // adjust the sampling rate
    printf("ADC initialized\n");
    
    while(true){
        // *************************************************
        // ------------- Stalling Stage Starts -------------
        // -------------------------------------------------
        // let the first main skips the stalling stage
        if (machine_state <= 1) {
            // set in-mode for receiver pin and pull it up for irq pending
            gpio_set_dir(RECEIVER_PIN, GPIO_IN);
            gpio_pull_up(RECEIVER_PIN);
    
            // initialize callback function for stalling stage, unlock once interrupted 
            gpio_set_irq_enabled_with_callback(RECEIVER_PIN, GPIO_IRQ_EDGE_RISE, true, &unlock_trigger_callback);
    
            while(lock){
                tight_loop_contents();
            }
        }
        // -------------------------------------------------
        // ------------- Stalling Stage Exited -------------
        // *************************************************
    
    
        // *************************************************
        // ---------------- ADC Read Starts ----------------
        // -------------------------------------------------
        gpio_set_dir(ADC_PULSE_PIN, GPIO_IN);
        gpio_pull_up(ADC_PULSE_PIN);
        gpio_set_irq_enabled_with_callback(ADC_PULSE_PIN, GPIO_IRQ_EDGE_RISE, true, &ADC_trigger_callback);
        printf("ADC pulse pin initialized\n");
    
        while (!sampling_done) {
            tight_loop_contents();
        }
        // -------------------------------------------------
        // --------------- ADC Read Complete ---------------
        // *************************************************
    
    
        // *************************************************
        // ----------- Buffer Transferring Starts ----------
        // -------------------------------------------------
        // buffer transfer using UART interface
#if defined(UART_TR)
        send_data_uart(sample_buffer);
#elif defined(I2C_TR)
        send_data_i2c(sample_buffer);
#elif defined(SPI_TR)
        send_data_spi(sample_buffer);
#else
        #error "Please define a transfer Interface!"
#endif
        // -------------------------------------------------
        // ----------- Buffer Transferring Ends ------------
        // *************************************************
    
    
        // TODO: clear the BUFFER and reinitialize the counter
        if(clear_buffer()){
            printf("Error: Cannot be clear");
            return 1;
        }
        
        machine_state += MACHINES_EMPLOYED; // increment the machine states for debug
        lock = true;    // flag the lock status
        sample_index = 0; // reset the buffer counter
    
    }
    
#ifdef PRINT_BUFFER_TEST
    print_buffer();
#endif

    return 0;
}
