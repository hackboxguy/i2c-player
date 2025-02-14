# I2C Player

A versatile command-line tool for interacting with I2C devices using CSV-based command sequences. This tool allows you to read from and write to I2C devices, execute command sequences, and parse sensor data with built-in device-specific parsers.

## Features

- CSV-based command sequence execution
- Support for multiple I2C devices and sensors
- Built-in parsers for common I2C devices:
  - BMP280 (Temperature/Pressure sensor)
  - VEML7700 (Light sensor)
  - BH1750 (Light sensor)
  - DS3231 (Real-Time Clock)
  - ADS1015 (ADC)
  - 24Cxx EEPROM series
- Error handling with retry capabilities
- Loop sequence support
- Configurable delays between operations
- Verbose mode for debugging

## Prerequisites

- Linux system with I2C support enabled
- CMake (version 3.10 or higher)
- GCC with C++17 support
- I2C development files (`libi2c-dev` package)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Basic syntax:
```bash
./i2c-player --device=/dev/i2c-X --input=sequence.csv [OPTIONS]
```

Options:
```
--device=/dev/i2c-X    I2C device (e.g., /dev/i2c-1)
--input=<file>         Input CSV file with I2C transactions
--verbose              Enable verbose output
--i2cwaitms=<ms>      Wait time between I2C operations in milliseconds (default: 1)
--onerror=<action>    Action on NAK/error: stop|retry|continue (default: stop)
--retries=<n>         Number of retries on error (default: 3)
```

## CSV Command Format

The CSV file should contain commands in the following format:
```csv
command,addr,reg,data
```

Supported commands:
- `WRITE,addr,reg,data` - Write single byte
- `WRITE1,addr,data` - Write single byte without register
- `WRITE16,addr,reg,data` - Write 16-bit value
- `READ,addr,reg` - Read single byte
- `POLL,addr,reg,mask,exp,t,i` - Poll register with timeout
- `DELAY,milliseconds` - Insert delay
- `LOOP,count` - Start loop block
- `ENDLOOP` - End loop block
- `START_RECORD,size` - Start recording reads
- `STOP_RECORD` - Stop recording reads
- `PRINT_RECORD,device` - Parse and print recorded data

## Example CSV Files

### Reading BMP280 Sensor
```csv
# BMP280 Temperature and Pressure Reading
command,addr,reg,data
START_RECORD,6
READ,0x76,0xFA
READ,0x76,0xFB
READ,0x76,0xFC
READ,0x76,0xF7
READ,0x76,0xF8
READ,0x76,0xF9
STOP_RECORD
PRINT_RECORD,BMP280
```

### Blinking LED with PCF8574
```csv
WRITE1,0x38,0xFF
LOOP,30
WRITE1,0x38,0xFE
DELAY,100
WRITE1,0x38,0xFF
DELAY,100
ENDLOOP
```

## Adding New Device Parsers

1. Create parser header (e.g., `include/parsers/new_device_parser.hpp`):
```cpp
#pragma once
#include "i2c_device_parser.hpp"

class NewDeviceParser : public I2CDeviceParser {
public:
    void parse(const std::vector<uint8_t>& buffer) override;
private:
    // Device-specific methods
};
```

2. Create parser implementation (e.g., `src/parsers/new_device_parser.cpp`).
3. Register parser in `I2CPlayer` constructor.

## Project Structure

```
i2c-player/
├── CMakeLists.txt
├── include/
│   ├── i2c_player.hpp
│   ├── error_action.hpp
│   └── parsers/
│       ├── i2c_device_parser.hpp
│       └── [device]_parser.hpp
├── src/
│   ├── main.cpp
│   ├── i2c_player.cpp
│   └── parsers/
│       └── [device]_parser.cpp
└── examples/
    └── *.csv
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests, bug reports, or feature requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
