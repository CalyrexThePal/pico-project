'''
    About
    Serial-Peripheral-Interface (SPI) based receiver program to run in your Master host, in this case, 
    the raspberry pi 5. Make sure the Master host device has 'RPi.GPIO' and 'spidev' python packages 
    installed in your environment

    Description:
    Spawn dual threads each correspond to one SPI channel, typical implementation would be:
        Pico A -----> SP0
        Pico B -----> SP1

'''
import spidev
import RPi.GPIO as GPIO
import time
import threading

BUFFER_SIZE = 32768  # same as the buffer size on the Pico

# init SPI for both buses
spi0 = spidev.SpiDev()
spi1 = spidev.SpiDev()

# define Ready/Busy signal pins
PIN_READY_0 = 17  # GPIO 17 for SPI0
PIN_READY_1 = 27  # GPIO 27 for SPI1

def setup_spi(spi, bus, device):
    spi.open(bus, device)
    spi.max_speed_hz = 500000  # 500 kHz
    spi.mode = 0b00  # SPI mode 0

def read_spi_buffer(spi, buffer_name, ready_pin):
    GPIO.setup(ready_pin, GPIO.OUT)

    while True:
        buffer = []
        for _ in range(BUFFER_SIZE):
            # signal ready
            GPIO.output(ready_pin, GPIO.HIGH)

            # read two bytes from the SPI bus
            lower_byte = spi.readbytes(1)[0]
            upper_byte = spi.readbytes(1)[0]

            # signal busy
            GPIO.output(ready_pin, GPIO.LOW)

            # combine the bytes to form the original 16-bit value
            value = (upper_byte << 8) | lower_byte
            buffer.append(value)

        print(f"Buffer from {buffer_name}:", buffer[:20])  # print first 20 values for verification
        # process the buffer data here if needed
        time.sleep(1)  # short delay to simulate processing time

def spi_thread(spi, bus, device, buffer_name, ready_pin):
    setup_spi(spi, bus, device)
    read_spi_buffer(spi, buffer_name, ready_pin)

def main():
    GPIO.setmode(GPIO.BCM)

    # create threads for SPI0 (Pico A) and SPI1 (Pico B)
    t0 = threading.Thread(target=spi_thread, args=(spi0, 0, 0, "Pico A (SPI0)", PIN_READY_0))
    t1 = threading.Thread(target=spi_thread, args=(spi1, 1, 0, "Pico B (SPI1)", PIN_READY_1))

    # start the threads
    t0.start()
    t1.start()

    # join the threads to keep the main program running indefinitely
    t0.join()
    t1.join()

if __name__ == "__main__":
    main()
