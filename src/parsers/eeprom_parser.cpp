#include "parsers/eeprom_parser.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

EEPROMParser::EEPROMParser(bool show_ascii_opt, uint8_t bytes_per_line_opt)
    : show_ascii(show_ascii_opt)
    , bytes_per_line(bytes_per_line_opt ? bytes_per_line_opt : DEFAULT_BYTES_PER_LINE) {
}

void EEPROMParser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.empty()) {
        std::cerr << "Empty EEPROM data buffer\n";
        return;
    }

    std::cout << "EEPROM Data Dump:\n";
    printSizeInfo(buffer.size());
    std::cout << std::string(50, '-') << "\n";
    printHexDump(buffer);
}

void EEPROMParser::printHexDump(const std::vector<uint8_t>& buffer) const {
    for (size_t i = 0; i < buffer.size(); i += bytes_per_line) {
        // Print address
        std::cout << formatAddress(i) << "  ";

        // Print hex values
        size_t line_bytes = std::min(static_cast<size_t>(bytes_per_line), 
                                   buffer.size() - i);
        
        for (size_t j = 0; j < bytes_per_line; ++j) {
            if (j < line_bytes) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                         << static_cast<int>(buffer[i + j]) << " ";
            } else {
                std::cout << "   "; // Padding for incomplete lines
            }
            
            // Optional middle space after 8 bytes
            if (j == 7) {
                std::cout << " ";
            }
        }

        // Print ASCII representation if enabled
        if (show_ascii) {
            std::cout << " |";
            printAsciiLine(buffer, i, line_bytes);
            std::cout << "|";
        }
        
        std::cout << "\n";
    }
    std::cout << std::dec; // Reset to decimal mode
}

void EEPROMParser::printAsciiLine(const std::vector<uint8_t>& buffer,
                                 size_t start, size_t count) const {
    for (size_t i = 0; i < bytes_per_line; ++i) {
        if (i < count) {
            uint8_t byte = buffer[start + i];
            std::cout << (isPrintable(byte) ? static_cast<char>(byte) : ASCII_PLACEHOLDER);
        } else {
            std::cout << " "; // Padding for incomplete lines
        }
    }
}

bool EEPROMParser::isPrintable(uint8_t byte) const {
    return (byte >= 32 && byte <= 126) || byte >= 160;
}

std::string EEPROMParser::formatAddress(size_t address) const {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << address;
    return ss.str();
}

bool EEPROMParser::isCommonEEPROMSize(size_t size) const {
    for (auto common_size : COMMON_SIZES) {
        if (common_size == size) {
            return true;
        }
    }
    return false;
}

void EEPROMParser::printSizeInfo(size_t size) const {
    std::cout << "Total Size: " << size << " bytes";
    
    if (isCommonEEPROMSize(size)) {
        // Find the matching EEPROM model
        for (size_t i = 0; i < COMMON_SIZES.size(); ++i) {
            if (COMMON_SIZES[i] == size) {
                std::cout << " (Compatible with 24C" 
                         << std::setw(2) << std::setfill('0') 
                         << (1 << i) << ")";
                break;
            }
        }
    } else {
        std::cout << " (Non-standard size)";
    }
    std::cout << "\n\n";

    // Print address range
    std::cout << "Address Range: 0x0000 to 0x"
              << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
              << (size - 1) << std::dec << "\n";

    if (size % bytes_per_line != 0) {
        std::cout << "Note: Last line will be partial ("
                  << (size % bytes_per_line) << " bytes)\n";
    }
}
