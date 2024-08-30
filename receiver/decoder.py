import struct
import sys

def read_binary_file(filename, fmt):
    with open(filename, "rb") as file:
        data = file.read()

    if fmt == 'uint16':
        return struct.unpack(f'{len(data)//2}H', data)  # Unsigned 16-bit
    elif fmt == 'int16':
        return struct.unpack(f'{len(data)//2}h', data)  # Signed 16-bit
    elif fmt == 'hex':
        return [f'{val:04x}' for val in struct.unpack(f'{len(data)//2}H', data)]  # Hexadecimal
    else:
        raise ValueError("Unsupported format")

def print_data(data, fmt):
    if fmt in ['uint16', 'int16']:
        for i, value in enumerate(data):
            print(f"{i}: {value}")
    elif fmt == 'hex':
        for i, value in enumerate(data):
            print(f"{i}: 0x{value}")

def main():
    if len(sys.argv) < 3:
        print("Usage: python decoder.py <filename> <format>")
        print("Formats: uint16, int16, hex")
        sys.exit(1)

    filename = sys.argv[1]
    fmt = sys.argv[2]

    try:
        data = read_binary_file(filename, fmt)
        print_data(data, fmt)
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
