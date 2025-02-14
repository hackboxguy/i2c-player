#include "parsers/veml7700_parser.hpp"
#include <iostream>
#include <iomanip>

void VEML7700Parser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 2) {
        std::cerr << "Insufficient data for VEML7700 parsing\n";
        return;
    }

    // VEML7700 uses little-endian format
    uint16_t raw_value = buffer[0] | (buffer[1] << 8);

    std::cout << "VEML7700 Light Sensor Data:\n";
    
    // Print raw data information
    std::cout << "Raw Bytes: 0x"
              << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[0])
              << " 0x" << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[1])
              << std::dec << "\n";
    
    std::cout << "Raw Value: 0x" << std::hex << raw_value << std::dec << "\n";

    // If this is a configuration read
    if (raw_value & (ALS_GAIN_MASK | ALS_IT_MASK)) {
        printConfiguration(raw_value);
        return;
    }

    // Calculate and display light intensity
    float light_intensity = calculateLux(raw_value);
    std::cout << "Light Intensity: "
              << std::fixed << std::setprecision(2)
              << light_intensity << " lux\n";

    // Print diagnostics for unusual readings
    printDiagnostics(raw_value);
}

float VEML7700Parser::calculateLux(uint16_t raw_value) const {
    // Basic conversion using default resolution
    float lux = raw_value * BASE_RESOLUTION;
    return lux;
}

void VEML7700Parser::printConfiguration(uint16_t config_value) const {
    std::cout << "\nConfiguration Register Analysis:\n";
    
    // Analyze gain setting
    analyzeGainSetting(config_value);
    
    // Analyze integration time
    analyzeIntegrationTime(config_value);
    
    // Check sensor status
    checkSensorStatus(config_value);
    
    // Print persistence protect number
    uint8_t persist = (config_value & ALS_PERS_MASK) >> 4;
    std::cout << "Persistence Protection: " << static_cast<int>(persist) << " samples\n";
    
    // Check interrupt enable
    bool int_enabled = config_value & ALS_INT_EN_MASK;
    std::cout << "Interrupt: " << (int_enabled ? "Enabled" : "Disabled") << "\n";
}

void VEML7700Parser::analyzeGainSetting(uint16_t config_value) const {
    uint8_t gain = (config_value & ALS_GAIN_MASK) >> 11;
    std::cout << "Gain Setting: ";
    switch (static_cast<Gain>(gain)) {
        case Gain::X1:
            std::cout << "x1 (High Dynamic Range)\n";
            break;
        case Gain::X2:
            std::cout << "x2 (High Sensitivity)\n";
            break;
        case Gain::X1_8:
            std::cout << "x1/8 (Extended Range)\n";
            break;
        case Gain::X1_4:
            std::cout << "x1/4 (Standard)\n";
            break;
        default:
            std::cout << "Unknown\n";
    }
}

void VEML7700Parser::analyzeIntegrationTime(uint16_t config_value) const {
    uint8_t it = (config_value & ALS_IT_MASK) >> 6;
    std::cout << "Integration Time: ";
    switch (static_cast<IntegrationTime>(it)) {
        case IntegrationTime::MS25:
            std::cout << "25ms (Fastest)\n";
            break;
        case IntegrationTime::MS50:
            std::cout << "50ms\n";
            break;
        case IntegrationTime::MS100:
            std::cout << "100ms (Default)\n";
            break;
        case IntegrationTime::MS200:
            std::cout << "200ms\n";
            break;
        case IntegrationTime::MS400:
            std::cout << "400ms\n";
            break;
        case IntegrationTime::MS800:
            std::cout << "800ms (Highest Sensitivity)\n";
            break;
        default:
            std::cout << "Unknown\n";
    }
}

void VEML7700Parser::checkSensorStatus(uint16_t config_value) const {
    bool shutdown = config_value & ALS_SD_MASK;
    std::cout << "Sensor Status: " << (shutdown ? "Power Down" : "Active") << "\n";
}

float VEML7700Parser::getGainFactor(uint16_t config_value) const {
    uint8_t gain = (config_value & ALS_GAIN_MASK) >> 11;
    switch (static_cast<Gain>(gain)) {
        case Gain::X2:   return 2.0f;
        case Gain::X1_8: return 0.125f;
        case Gain::X1_4: return 0.25f;
        case Gain::X1:
        default:         return 1.0f;
    }
}

float VEML7700Parser::getIntegrationFactor(uint16_t config_value) const {
    uint8_t it = (config_value & ALS_IT_MASK) >> 6;
    switch (static_cast<IntegrationTime>(it)) {
        case IntegrationTime::MS25:  return 0.25f;
        case IntegrationTime::MS50:  return 0.5f;
        case IntegrationTime::MS200: return 2.0f;
        case IntegrationTime::MS400: return 4.0f;
        case IntegrationTime::MS800: return 8.0f;
        case IntegrationTime::MS100:
        default:                     return 1.0f;
    }
}

void VEML7700Parser::printDiagnostics(uint16_t raw_value) const {
    if (raw_value == 0) {
        std::cout << "\nDiagnostic Information:\n"
                  << "Zero reading detected. Possible causes:\n"
                  << "  - Sensor in dark environment\n"
                  << "  - Sensor powered down\n"
                  << "  - Incorrect configuration\n"
                  << "  - Communication error\n"
                  << "\nRecommended configuration options:\n"
                  << "  0x1800: ALS Enable, Highest Sensitivity\n"
                  << "  0x1000: ALS Enable, Medium Sensitivity\n"
                  << "  0x0000: Shutdown Mode\n";
    } else if (raw_value == MAX_VALUE) {
        std::cout << "\nDiagnostic Information:\n"
                  << "Saturation detected. Consider:\n"
                  << "  - Reducing gain\n"
                  << "  - Reducing integration time\n"
                  << "  - Using extended dynamic range mode\n";
    }
}
