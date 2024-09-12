### About
#### Multi-controller triggered based ADC
The experimental setup uses multiple pico (in needs), this specific implementation uses two picos. You can add as many picos as you want by modifying the program configs.

### Setup
Example: connect GPIO 8 of pico A to GPIO 9 of pico B, and connect GPIO 9 of pico A to GPIO 8 of pico B. Supply the pulse source to GPIO 2 pins of both pico A and B. You can supply different source of ADC signals to two picos' ADC input pin. Make sure that both picos share the common ground. You need to configure the data transfer wiring based on your setup, e.g., pico A to SPI0, pico B to SPI1 on the same pi 5.

### Usage
Edit the `machine_state` and the `lock` status corresponds to your physical setup. Make pico A to be machine state 0 and let pico B to be machine state 1, thus two versions of `adc_multi.uf2` should be according to the physical setup