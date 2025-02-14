#pragma once

#include "i2c_device_parser.hpp"
#include <cstdint>
#include <vector>

class BH1750Parser : public I2CDeviceParser {
public:
    // Parse raw light sensor data
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    // Constants for light intensity conversion
    static constexpr float LUX_CONVERSION_FACTOR = 1.2f;  // Standard conversion for BH1750
    
    // Operational modes (for reference)
    enum class Mode {
        CONTINUOUS_HIGH_RES   = 0x10,  // 1 lx resolution
        CONTINUOUS_HIGH_RES2  = 0x11,  // 0.5 lx resolution
        CONTINUOUS_LOW_RES    = 0x13,  // 4 lx resolution
        ONE_TIME_HIGH_RES    = 0x20,   // Single high res measurement
        ONE_TIME_HIGH_RES2   = 0x21,   // Single high res2 measurement
        ONE_TIME_LOW_RES     = 0x23    // Single low res measurement
    };

    // Helper methods
    float calculateLux(uint16_t raw_value) const;
    void printDiagnostics(float lux) const;
    void printModeReference() const;
};
