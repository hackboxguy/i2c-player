#pragma once

#include <vector>
#include <cstdint>

class I2CDeviceParser {
public:
    virtual void parse(const std::vector<uint8_t>& buffer) = 0;
    virtual ~I2CDeviceParser() = default;
};
