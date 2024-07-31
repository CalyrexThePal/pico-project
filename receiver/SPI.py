import spidev
import threading

BUFFER_SIZE = 10000
SPI0_DEV = "/dev/spidev0.0"
SPI1_DEV = "/dev/spidev1.0"
FILE0 = "data_spi0.bin"
FILE1 = "data_spi1.bin"

def save_data(filename, data):
    with open(filename, "wb") as f:
        f.write(data)

def receive_data(spi_dev, filename):
    spi = spidev.SpiDev()
    spi.open(0, spi_dev)
    spi.max_speed_hz = 500000

    while True:
        data = spi.readbytes(BUFFER_SIZE*2)
        save_data(filename, data)

def main():
    t0 = threading.Thread(target=receive_data, args=(0, FILE0))
    t1 = threading.Thread(target=receive_data, args=(1, FILE1))

    t0.start()
    t1.start()

    t0.join()
    t1.join()

if __name__ == "__main__":
    main()
