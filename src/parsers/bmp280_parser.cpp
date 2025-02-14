#include "parsers/bmp280_parser.hpp"
#include <iostream>
#include <iomanip>

void BMP280Parser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 6) {
        std::cerr << "Insufficient data for BMP280 parsing\n";
        return;
    }

    // Temperature calculation (based on BMP280 datasheet)
    int32_t adc_T = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    float temperature = calculateTemperature(adc_T);

    // Pressure calculation (based on BMP280 datasheet)
    int32_t adc_P = (buffer[3] << 12) | (buffer[4] << 4) | (buffer[5] >> 4);
    float pressure = calculatePressure(adc_P);

    std::cout << "BMP280 Sensor Data:\n";
    std::cout << "Temperature: "
              << std::fixed << std::setprecision(2)
              << temperature << " °C\n";

    std::cout << "Pressure: "
              << std::fixed << std::setprecision(2)
              << pressure / 100.0f << " hPa\n";  // Convert Pa to hPa
}

float BMP280Parser::calculateTemperature(int32_t adc_T) const {
    int32_t var1 = ((((adc_T >> 3) - (dig_T1 << 1))) * dig_T2) >> 11;
    int32_t var2 = (((((adc_T >> 4) - dig_T1) * ((adc_T >> 4) - dig_T1)) >> 12) * dig_T3) >> 14;
    t_fine = var1 + var2;
    float temperature = ((t_fine * 5 + 128) >> 8) / 100.0f;
    return temperature;
}

float BMP280Parser::calculatePressure(int32_t adc_P) const {
    int64_t var1, var2, p;

    var1 = static_cast<int64_t>(t_fine) - 128000;
    var2 = var1 * var1 * static_cast<int64_t>(dig_P6);
    var2 = var2 + ((var1 * static_cast<int64_t>(dig_P5)) << 17);
    var2 = var2 + (static_cast<int64_t>(dig_P4) << 35);
    var1 = ((var1 * var1 * static_cast<int64_t>(dig_P3)) >> 8) + 
           ((var1 * static_cast<int64_t>(dig_P2)) << 12);
    var1 = (((static_cast<int64_t>(1) << 47) + var1)) * static_cast<int64_t>(dig_P1) >> 33;

    if (var1 == 0) {
        return 0.0f; // avoid division by zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (static_cast<int64_t>(dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(dig_P7) << 4);

    return static_cast<float>(p) / 256.0f;
}
