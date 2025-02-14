#include "parsers/ads1015_parser.hpp"
#include <iostream>
#include <iomanip>

void ADS1015Parser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 2) {
        std::cerr << "Insufficient data for ADS1015 parsing\n";
        return;
    }

    // Combine two bytes to get 12-bit ADC reading
    // ADS1015 transmits data in most significant byte first (MSB) format
    int16_t raw_value = (buffer[0] << 4) | (buffer[1] >> 4);

    // If the value is negative (12-bit signed), extend the sign
    if (raw_value & 0x800) {
        raw_value |= 0xF000;
    }

    std::cout << "ADS1015 ADC Data:\n";
    
    // Print raw data information
    std::cout << "Raw Bytes: 0x" 
              << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[0])
              << " 0x" << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[1])
              << std::dec << "\n";
    
    std::cout << "Raw Value: " << raw_value << " (0x" 
              << std::hex << std::setw(3) << std::setfill('0') << (raw_value & 0xFFF) 
              << std::dec << ")\n";

    // Convert and display voltage
    float voltage = convertToVoltage(raw_value);
    printVoltage(voltage);

    // Print additional diagnostics if value seems unusual
    if (raw_value == 0 || raw_value == TOTAL_STEPS || raw_value == -TOTAL_STEPS) {
        printDiagnostics(raw_value);
    }
}

float ADS1015Parser::convertToVoltage(int16_t raw_value) const {
    // Convert raw ADC value to voltage
    // For 12-bit ADC in differential mode, range is -2048 to +2047
    return (static_cast<float>(raw_value) * VOLTAGE_RANGE) / TOTAL_STEPS;
}

void ADS1015Parser::printVoltage(float voltage) const {
    std::cout << "Voltage: " 
              << std::fixed << std::setprecision(3) 
              << voltage << " V\n";

    // Add context for voltage reading
    if (std::abs(voltage) >= VOLTAGE_RANGE) {
        std::cout << "Note: Reading at or beyond full-scale range\n";
    }
}

void ADS1015Parser::printDiagnostics(int16_t raw_value) const {
    std::cout << "\nDiagnostic Information:\n";
    
    if (raw_value == 0) {
        std::cout << "Zero reading detected. Possible causes:\n"
                  << "  - Input shorted to ground\n"
                  << "  - Input within noise floor\n"
                  << "  - ADC configuration issue\n";
    }
    else if (raw_value == TOTAL_STEPS || raw_value == -TOTAL_STEPS) {
        std::cout << "Full-scale reading detected. Possible causes:\n"
                  << "  - Input voltage beyond ADC range\n"
                  << "  - Incorrect gain setting\n"
                  << "  - Open circuit on input\n";
    }

    // Print gain reference
    std::cout << "\nGain Settings Reference:\n"
              << "  GAIN_6_144V (0): ±6.144V range\n"
              << "  GAIN_4_096V (1): ±4.096V range (default)\n"
              << "  GAIN_2_048V (2): ±2.048V range\n"
              << "  GAIN_1_024V (3): ±1.024V range\n"
              << "  GAIN_0_512V (4): ±0.512V range\n"
              << "  GAIN_0_256V (5): ±0.256V range\n";
}
