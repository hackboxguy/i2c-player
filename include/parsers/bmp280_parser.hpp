#pragma once

#include "i2c_device_parser.hpp"
#include <iostream>
#include <iomanip>

class BMP280Parser : public I2CDeviceParser {
public:
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    float calculateTemperature(int32_t adc_T) const;
    float calculatePressure(int32_t adc_P) const;

    // Temperature calibration constants
    static constexpr int32_t dig_T1 = 27504;
    static constexpr int32_t dig_T2 = 26435;
    static constexpr int32_t dig_T3 = -1000;

    // Pressure calibration constants
    static constexpr int32_t dig_P1 = 36477;
    static constexpr int32_t dig_P2 = -10685;
    static constexpr int32_t dig_P3 = 3024;
    static constexpr int32_t dig_P4 = 2855;
    static constexpr int32_t dig_P5 = 140;
    static constexpr int32_t dig_P6 = -7;
    static constexpr int32_t dig_P7 = 15500;
    static constexpr int32_t dig_P8 = -14600;
    static constexpr int32_t dig_P9 = 6000;

    // Temperature storage for pressure calculation
    mutable int32_t t_fine;  // Add this for pressure calculation dependency
};
