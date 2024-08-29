import spidev
import time

# Initialize SPI
spi = spidev.SpiDev()
spi.open(0, 1)  # (bus, device) -> (SPI bus, chip select)
spi.max_speed_hz = 992063  # set the SPI clock speed
spi.mode = 0b00  # SPI mode (0, 1, 2, or 3)

# Function to receive data
def receive_spi_data(num_bytes):
    # Send dummy bytes (0x00) to receive data
    received_data = spi.xfer2([0x42] * num_bytes, 1000 * 1000, 5000)
    return received_data

# Example usage
try:
    while True:
        data = receive_spi_data(256)  
        print(f"Received data: {data}")
        time.sleep(5)
except KeyboardInterrupt:
    spi.close()
