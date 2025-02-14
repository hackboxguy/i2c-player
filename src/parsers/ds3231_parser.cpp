#include "parsers/ds3231_parser.hpp"
#include <iostream>
#include <iomanip>
#include <array>

void DS3231Parser::parse(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 7) {
        std::cerr << "Insufficient data for DS3231 parsing\n";
        return;
    }

    // Extract time components
    uint8_t seconds = extractSeconds(buffer[0]);
    uint8_t minutes = extractMinutes(buffer[1]);
    auto hour_format = extractHours(buffer[2]);
    
    // Extract date components
    uint8_t day_of_week = buffer[3];
    uint8_t date = extractDay(buffer[4]);
    uint8_t month = extractMonth(buffer[5]);
    uint8_t year = extractYear(buffer[6]);

    std::cout << "DS3231 RTC Data:\n";

    // Print time in appropriate format
    printTime(hour_format.hour, minutes, seconds, 
              hour_format.is_12_hour, hour_format.is_pm);

    // Print date if valid
    if (isValidDate(date, month, year)) {
        printDate(date, month, year);
    } else {
        std::cerr << "Invalid date values detected\n";
    }

    // Print day of week if valid
    if (isValidDayOfWeek(day_of_week)) {
        std::cout << "Day of Week: " << getDayOfWeek(day_of_week) << "\n";
    }

    // Print diagnostics for troubleshooting
    printDiagnostics(buffer);
}

uint8_t DS3231Parser::bcdToDecimal(uint8_t bcd) const {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

std::string DS3231Parser::getDayOfWeek(uint8_t day) const {
    static const std::array<const char*, 8> days = {
        "", "Sunday", "Monday", "Tuesday", "Wednesday", 
        "Thursday", "Friday", "Saturday"
    };
    return (day < days.size()) ? days[day] : "Invalid";
}

uint8_t DS3231Parser::extractSeconds(uint8_t raw) const {
    return bcdToDecimal(raw & SEC_MIN_MASK);
}

uint8_t DS3231Parser::extractMinutes(uint8_t raw) const {
    return bcdToDecimal(raw & SEC_MIN_MASK);
}

DS3231Parser::HourFormat DS3231Parser::extractHours(uint8_t raw) const {
    HourFormat result;
    result.is_12_hour = (raw & HOUR_12_24_MASK);
    
    if (result.is_12_hour) {
        result.is_pm = (raw & HOUR_AM_PM_MASK);
        result.hour = bcdToDecimal(raw & HOUR_12_MASK);
    } else {
        result.is_pm = false;
        result.hour = bcdToDecimal(raw & HOUR_24_MASK);
    }
    
    return result;
}

uint8_t DS3231Parser::extractDay(uint8_t raw) const {
    return bcdToDecimal(raw);
}

uint8_t DS3231Parser::extractMonth(uint8_t raw) const {
    return bcdToDecimal(raw & 0x1F);
}

uint8_t DS3231Parser::extractYear(uint8_t raw) const {
    return bcdToDecimal(raw);
}

bool DS3231Parser::isValidDayOfWeek(uint8_t day) const {
    return day > 0 && day <= 7;
}

bool DS3231Parser::isValidDate(uint8_t day, uint8_t month, uint8_t year) const {
    if (month < 1 || month > 12) return false;
    
    static const std::array<uint8_t, 12> days_in_month = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    // Handle leap year for February
    uint8_t max_days = days_in_month[month - 1];
    if (month == 2) {
        bool is_leap_year = (year % 4 == 0);
        if (is_leap_year) max_days = 29;
    }
    
    return day >= 1 && day <= max_days;
}

void DS3231Parser::printTime(uint8_t hours, uint8_t minutes, uint8_t seconds,
                           bool is_12_hour, bool is_pm) const {
    std::cout << "Time: "
              << std::setw(2) << std::setfill('0') << static_cast<int>(hours) << ":"
              << std::setw(2) << std::setfill('0') << static_cast<int>(minutes) << ":"
              << std::setw(2) << std::setfill('0') << static_cast<int>(seconds);
    
    if (is_12_hour) {
        std::cout << (is_pm ? " PM" : " AM");
    }
    std::cout << "\n";
}

void DS3231Parser::printDate(uint8_t day, uint8_t month, uint8_t year) const {
    std::cout << "Date: "
              << std::setw(2) << std::setfill('0') << static_cast<int>(month) << "/"
              << std::setw(2) << std::setfill('0') << static_cast<int>(day) << "/20"
              << std::setw(2) << std::setfill('0') << static_cast<int>(year) << "\n";
}

void DS3231Parser::printDiagnostics(const std::vector<uint8_t>& buffer) const {
    // Check for common issues
    if (buffer[0] & 0x80) {
        std::cout << "\nDiagnostic Information:\n"
                  << "- Oscillator Stop Flag is set\n"
                  << "  This indicates a power loss or other issue\n"
                  << "  RTC may need to be reinitialized\n";
    }

    // Print raw register values for debugging
    std::cout << "\nRegister Values (hex):\n"
              << "Seconds: 0x" << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(buffer[0]) << "\n"
              << "Minutes: 0x" << std::setw(2) << static_cast<int>(buffer[1]) << "\n"
              << "Hours  : 0x" << std::setw(2) << static_cast<int>(buffer[2]) << "\n"
              << "Day    : 0x" << std::setw(2) << static_cast<int>(buffer[3]) << "\n"
              << "Date   : 0x" << std::setw(2) << static_cast<int>(buffer[4]) << "\n"
              << "Month  : 0x" << std::setw(2) << static_cast<int>(buffer[5]) << "\n"
              << "Year   : 0x" << std::setw(2) << static_cast<int>(buffer[6]) << std::dec << "\n";
}
