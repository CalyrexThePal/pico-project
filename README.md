## About
This are some c codes I've been working with on raspberry pi pico, mainly on it's adc unit, and I will try more stuff in the future

## Usage
To use it, you need to configure your raspberry pi pico c/c++ sdk, following the guides on: [link](https://github.com/raspberrypi/pico-sdk), 
as well as their excellent official pdf: [link](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

## Build
To build the repo:
```bash
mkdir build
cd build
export PICO_SDK_PATH=../../pico-sdk
make -j4
```
Inside the build folder, there should be a src folder created that contains all the binary executable, just load the executable to raspberry pi pico 
and you're good to go!
