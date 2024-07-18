/*
    Author: Xuanyi Wu, UNLV, Deparment of Physics, Zhou's Lab

    About:
        This program attempts to utilize multiple micro-controller pico to achieve cycling
    adc reading and buffer transferring. You NEED to adjust the machine state before 
    compilation.
        
    Example:
        Machine state - 0; Lock status: false
        Machine state - 1; Lock status: true
    
    Usage:
        In the README file
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "adc_multi.h"

#define SENDER_PIN 8        // GPIO-8 - pin used to send flag signal for unstalling main
#define RECEIVER_PIN 9      // GPIO-9 - pin used to receive flag signal for unstalling main

/*
    ADC configs:
        - set Analog channel
        - set sampling rate
*/
#define ADC_PULSE_PIN 2     // GPIO-2 - pin pulled to receive pulse signal for ADC read
#define ADC_PIN 26          // ADC0 aka GPIO-26, pin corresponding to adc unit
#define ADC_CHANNEL 0       // ADC channels, pick from 0-3 (4 is reserved for temp. sensor)

// choose your buffer size (power of 2), 2^(x=15) to avoid page fault
#define SAMPLE_BUFFER_SIZE 32768
#define BUFFER_THRESHOLD 30000

// USER EDIT (OPTIONAL)
#define Fs 50000.0          // Sample rate (Hz) (must not goes higer than 75 kSPS)
#define ADCCLK 48000000.0   // ADC clock rate (unmutable!)

#define MACHINES_EMPLOYED 2 // how many pico we are using

// ---------------- Preprocessor variable ----------------
// #define RECORD_TIME

// USER EDIT (OPTIONAL): choose the transfer interface you're using, 
//                       comment out the unused ones
// #define UART_TR
// #define I2C_TR
// #define SPI_TR
#define PRINT_BUFFER_USB

// ------------------- Buffer Config ---------------------
volatile uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];
volatile uint32_t timestamp[SAMPLE_BUFFER_SIZE];
volatile uint16_t sample_index = 0;
volatile bool sampling_done = false;

// **********************************************************************
// ------------ IMPORTANT: ADJUST THE FOLLOWING VARIABLE !!! ------------
// **********************************************************************
unsigned int machine_state = 1; // USER EDIT: indicate the current machine, useful for debugging. 
volatile bool lock = true;     // USER EDIT: machine 1 starts off unlocked, the rest starts off locked. 

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
    Callback function for the interrupt that enables the ADC sampling,
    send pulse signal if buffer threshold is reacheds
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
            sleep_us(5);  // pulse duration
            gpio_put(SENDER_PIN, 0);  // set GPIO pin LOW
        }

        // check if the sample index is out of the BUFFER bound
        if (sample_index >= SAMPLE_BUFFER_SIZE) {
            // printf("Sampling is finished\n");
            sampling_done = true;
        }
    }
}


int clear_buffer(volatile uint16_t* data){
    // clear the sample buffer
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        data[i] = 0;
#ifdef RECORD_TIME
        timestamp[i] = 0; // clear the timestamp buffer if RECORD_TIME is defined
#endif
    }
    
    return 0;
}

// simple helper print function to print the BUFFER
void print_buffer_usb(){
    // output the buffer over UART serial
    for (int i = 0; i < 20; i++) {
        uint16_t result = sample_buffer[i];
#ifdef RECORD_TIME
        uint32_t time = timestamp[i];
        printf("Raw value: %d, voltage: %f, at time: %d\n", result, result * conversion_factor, time);
#else
        printf("Raw value: %d, voltage: %f\n", result, result * conversion_factor);
#endif
    }
}

int main() {
    stdio_init_all(); // initialize stdio lib
    sleep_ms(5000); // wait for USB initialization
    printf("USB initilization completed \n\n");

    // initialize all the operating pin
    gpio_init(ADC_PULSE_PIN);
    gpio_init(RECEIVER_PIN);
    gpio_init(SENDER_PIN);
    adc_init();

    // initialize ADC configurations
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);
    adc_set_clkdiv(ADCCLK/Fs); // adjust the sampling rate
    
    printf("Machine state %d: pins initialized... \n", machine_state);

    while(true){
        // *************************************************
        // ------------- Stalling Stage Starts -------------
        // -------------------------------------------------
        // let the first main skips the stalling stage
        if (machine_state >= 1) {
            printf("Machine state %d: stalling! \n", machine_state);
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
        printf("Machine state %d: starting ADC-capturing! \n", machine_state);
        gpio_set_dir(ADC_PULSE_PIN, GPIO_IN);
        gpio_pull_up(ADC_PULSE_PIN);
        gpio_set_irq_enabled_with_callback(ADC_PULSE_PIN, GPIO_IRQ_EDGE_RISE, true, &ADC_trigger_callback);

        while (!sampling_done) {
            tight_loop_contents();
        }
        // -------------------------------------------------
        // --------------- ADC Read Complete ---------------
        // *************************************************
    
        printf("Machine state %d: finished reading, now starts transferring! \n", machine_state);

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
#elif defined(PRINT_BUFFER_USB)
        // print_buffer_usb();
#else
        #error "Please define a transfer Interface!"
#endif
        // -------------------------------------------------
        // ----------- Buffer Transferring Ends ------------
        // *************************************************
    
        printf("Machine state %d: finished transferring! \n", machine_state);

        // clear the BUFFER and reinitialize the counter
        if(clear_buffer(sample_buffer)){
            printf("Error: Buffer cannot be clear. \n");
            return 1;
        }
        // reset the sampling index and flags
        sample_index = 0;
        sampling_done = false;
    
        
        printf("Machine state %d: state reset completed.. \n\n\n", machine_state);
        machine_state += MACHINES_EMPLOYED; // increment the machine states for debug
        lock = true;    // flag the lock status    
    }

    return 0;
}
