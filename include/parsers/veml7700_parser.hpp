#pragma once

#include "i2c_device_parser.hpp"
#include <cstdint>
#include <vector>

class VEML7700Parser : public I2CDeviceParser {
public:
    // Parse raw light sensor data
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    // Gain settings
    enum class Gain {
        X1 = 0,      // x1 gain
        X2 = 1,      // x2 gain
        X1_8 = 2,    // x1/8 gain
        X1_4 = 3     // x1/4 gain
    };

    // Integration time settings
    enum class IntegrationTime {
        MS25 = 0,    // 25ms
        MS50 = 1,    // 50ms
        MS100 = 2,   // 100ms
        MS200 = 3,   // 200ms
        MS400 = 4,   // 400ms
        MS800 = 5    // 800ms
    };

    // Constants for light intensity calculation
    static constexpr float BASE_RESOLUTION = 0.0036f;  // Base resolution in lux per count
    static constexpr uint16_t MAX_VALUE = 0xFFFF;
    
    // Configuration register bit positions
    static constexpr uint16_t ALS_GAIN_MASK = 0x1800;
    static constexpr uint16_t ALS_IT_MASK = 0x03C0;
    static constexpr uint16_t ALS_PERS_MASK = 0x0030;
    static constexpr uint16_t ALS_INT_EN_MASK = 0x0002;
    static constexpr uint16_t ALS_SD_MASK = 0x0001;

    // Helper methods
    float calculateLux(uint16_t raw_value) const;
    void printConfiguration(uint16_t config_value) const;
    void printDiagnostics(uint16_t raw_value) const;
    
    // Configuration analysis
    void analyzeGainSetting(uint16_t config_value) const;
    void analyzeIntegrationTime(uint16_t config_value) const;
    void checkSensorStatus(uint16_t config_value) const;
    
    // Conversion factors
    float getGainFactor(uint16_t config_value) const;
    float getIntegrationFactor(uint16_t config_value) const;
};
