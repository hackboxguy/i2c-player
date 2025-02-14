#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "error_action.hpp"
#include "parsers/i2c_device_parser.hpp"

class I2CPlayer {
public:
    I2CPlayer(const std::string& device, bool verbose_mode = false,
              int wait_ms = 1, ErrorAction action = ErrorAction::STOP,
              int retries = 3);
    ~I2CPlayer();

    void playFile(const std::string& filename);
    void registerParser(const std::string& device_name, 
                       std::unique_ptr<I2CDeviceParser> parser);

private:
    // I2C operations
    uint8_t readByte(uint8_t addr, uint8_t reg);
    void writeByte(uint8_t addr, uint8_t reg, uint8_t data);
    void writeSingleByte(uint8_t addr, uint8_t data);
    void write16Bit(uint8_t addr, uint8_t reg, uint16_t data);
    bool pollRegister(uint8_t addr, uint8_t reg, uint8_t mask, uint8_t expected,
                     int timeout_ms, int interval_ms);
    void writeFile(uint8_t addr, uint8_t reg, const std::string& filename);
    void executeSequence(const std::vector<std::string>& sequence, int iterations);

    // Utility functions
    std::string trim(const std::string& str);
    int hexToInt(const std::string& hex);
    bool checkNAK(const char* operation);

    // Member variables
    int i2c_fd;
    std::string device_path;
    bool verbose;
    std::string csv_directory;
    int i2c_wait_ms;
    ErrorAction error_action;
    int retry_count;
    std::vector<uint8_t> record_buffer;
    bool recording;
    std::unordered_map<std::string, std::unique_ptr<I2CDeviceParser>> parsers;
};
