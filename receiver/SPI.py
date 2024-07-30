import spidev
import threading

BUFFER_SIZE = 32768
SPI0_DEV = "/dev/spidev0.0"
SPI1_DEV = "/dev/spidev1.0"
FILE0 = "data_spi0.bin"
FILE1 = "data_spi1.bin"

def save_data_to_file(filename, data):
    with open(filename, "wb") as f:
        f.write(data)

def receive_data_via_spi(spi_dev, filename):
    spi = spidev.SpiDev()
    spi.open(0, spi_dev)
    spi.max_speed_hz = 500000

    while True:
        data = spi.readbytes(BUFFER_SIZE * 2)
        save_data_to_file(filename, data)

def main():
    t0 = threading.Thread(target=receive_data_via_spi, args=(0, FILE0))
    t1 = threading.Thread(target=receive_data_via_spi, args=(1, FILE1))

    t0.start()
    t1.start()

    t0.join()
    t1.join()

if __name__ == "__main__":
    main()
