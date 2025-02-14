#pragma once

#include "i2c_device_parser.hpp"
#include <cstdint>
#include <vector>
#include <string>

class DS3231Parser : public I2CDeviceParser {
public:
    // Parse raw RTC data
    void parse(const std::vector<uint8_t>& buffer) override;

private:
    // Register bit masks
    static constexpr uint8_t HOUR_12_24_MASK = 0x40;  // 12/24 hour mode flag
    static constexpr uint8_t HOUR_AM_PM_MASK = 0x20;  // AM/PM flag
    static constexpr uint8_t HOUR_12_MASK = 0x1F;     // 12-hour format mask
    static constexpr uint8_t HOUR_24_MASK = 0x3F;     // 24-hour format mask
    static constexpr uint8_t SEC_MIN_MASK = 0x7F;     // Seconds/Minutes mask
    
    // Helper methods for time conversion
    uint8_t bcdToDecimal(uint8_t bcd) const;
    std::string getDayOfWeek(uint8_t day) const;
    
    // Time component extraction
    uint8_t extractSeconds(uint8_t raw) const;
    uint8_t extractMinutes(uint8_t raw) const;
    struct HourFormat {
        uint8_t hour;
        bool is_12_hour;
        bool is_pm;
    };
    HourFormat extractHours(uint8_t raw) const;
    
    // Date component extraction
    uint8_t extractDay(uint8_t raw) const;
    uint8_t extractMonth(uint8_t raw) const;
    uint8_t extractYear(uint8_t raw) const;

    // Validation methods
    bool isValidDayOfWeek(uint8_t day) const;
    bool isValidDate(uint8_t day, uint8_t month, uint8_t year) const;
    
    // Output formatting
    void printTime(uint8_t hours, uint8_t minutes, uint8_t seconds, bool is_12_hour, bool is_pm) const;
    void printDate(uint8_t day, uint8_t month, uint8_t year) const;
    void printDiagnostics(const std::vector<uint8_t>& buffer) const;
};
