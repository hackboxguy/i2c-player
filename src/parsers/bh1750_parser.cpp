#include "parsers/bh1750_parser.hpp"
#include <iostream>
#include <iomanip>

void BH1750Parser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 2) {
        std::cerr << "Insufficient data for BH1750 parsing\n";
        return;
    }

    // Combine two bytes to get 16-bit value
    // BH1750 sends data in high byte first format (MSB)
    uint16_t raw_value = (buffer[0] << 8) | buffer[1];

    std::cout << "BH1750 Light Sensor Data:\n";
    
    // Print raw data information
    std::cout << "Raw Bytes: 0x" 
              << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[0])
              << " 0x" << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[1])
              << std::dec << "\n";
    
    std::cout << "Raw Value: 0x" << std::hex << raw_value << std::dec << "\n";

    // Calculate and display light intensity
    float light_intensity = calculateLux(raw_value);
    std::cout << "Light Intensity: "
              << std::fixed << std::setprecision(2)
              << light_intensity << " lux\n";

    // Print diagnostics for unusual readings
    printDiagnostics(light_intensity);
}

float BH1750Parser::calculateLux(uint16_t raw_value) const {
    // Convert raw value to lux using standard conversion factor
    // The formula is: raw_value / 1.2 (typical)
    return static_cast<float>(raw_value) / LUX_CONVERSION_FACTOR;
}

void BH1750Parser::printDiagnostics(float lux) const {
    if (lux < 1.0) {
        std::cout << "\nDiagnostic Information:\n"
                  << "Very low light level detected.\n"
                  << "Possible causes:\n"
                  << "1. Actual low light condition\n"
                  << "2. Sensor communication issue\n"
                  << "3. Incorrect measurement mode\n\n";
        printModeReference();
    } else if (lux > 65535.0f) {
        std::cout << "\nDiagnostic Information:\n"
                  << "Light level exceeds sensor range.\n"
                  << "Possible solutions:\n"
                  << "1. Use a different measurement mode\n"
                  << "2. Add optical filters\n"
                  << "3. Adjust sensor positioning\n\n";
        printModeReference();
    }
}

void BH1750Parser::printModeReference() const {
    std::cout << "Operation Mode Reference:\n"
              << "  0x10: Continuous High Resolution (1 lx resolution)\n"
              << "  0x11: Continuous High Resolution 2 (0.5 lx resolution)\n"
              << "  0x13: Continuous Low Resolution (4 lx resolution)\n"
              << "  0x20: One-Time High Resolution\n"
              << "  0x21: One-Time High Resolution 2\n"
              << "  0x23: One-Time Low Resolution\n\n"
              << "Recommended settings:\n"
              << "- Normal lighting: High Resolution (0x10)\n"
              << "- Low light: High Resolution 2 (0x11)\n"
              << "- Bright light: Low Resolution (0x13)\n"
              << "- Power saving: One-Time modes (0x20/0x21/0x23)\n";
}
