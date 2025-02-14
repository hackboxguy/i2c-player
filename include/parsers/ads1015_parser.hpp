#pragma once

#include "i2c_device_parser.hpp"
#include <cstdint>
#include <vector>

class ADS1015Parser : public I2CDeviceParser {
public:
    // Parse raw ADC data from the device
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    // Constants for voltage conversion
    static constexpr float VOLTAGE_RANGE = 4.096f;  // Full scale range in volts
    static constexpr int TOTAL_STEPS = 2047;        // 11-bit ADC (2^11 - 1)
    
    // Gain settings (for reference)
    enum class Gain {
        GAIN_6_144V = 0,  // +/-6.144V
        GAIN_4_096V = 1,  // +/-4.096V (default)
        GAIN_2_048V = 2,  // +/-2.048V
        GAIN_1_024V = 3,  // +/-1.024V
        GAIN_0_512V = 4,  // +/-0.512V
        GAIN_0_256V = 5   // +/-0.256V
    };

    // Helper methods
    float convertToVoltage(int16_t raw_value) const;
    void printVoltage(float voltage) const;
    void printDiagnostics(int16_t raw_value) const;
};
