#pragma once

#include "i2c_device_parser.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <array>   // Added this header for std::array

class EEPROMParser : public I2CDeviceParser {
public:
    // Constructor to set display preferences
    EEPROMParser(bool show_ascii = true, uint8_t bytes_per_line = 16);

    // Parse raw EEPROM data
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    // Display format settings
    bool show_ascii;            // Show ASCII representation
    uint8_t bytes_per_line;     // Number of bytes to show per line

    // Helper methods
    void printHexDump(const std::vector<uint8_t>& buffer) const;
    void printAsciiLine(const std::vector<uint8_t>& buffer, 
                       size_t start, size_t count) const;
    bool isPrintable(uint8_t byte) const;
    
    // Address formatting
    std::string formatAddress(size_t address) const;
    
    // Data validation
    bool isCommonEEPROMSize(size_t size) const;
    void printSizeInfo(size_t size) const;
    
    // Constants
    static constexpr uint8_t DEFAULT_BYTES_PER_LINE = 16;
    static constexpr char ASCII_PLACEHOLDER = '.';
    static constexpr std::array<size_t, 8> COMMON_SIZES = {
        128,    // 24C01
        256,    // 24C02
        512,    // 24C04
        1024,   // 24C08
        2048,   // 24C16
        4096,   // 24C32
        8192,   // 24C64
        16384   // 24C128
    };
};
