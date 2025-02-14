#include "i2c_player.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <iomanip>
#include <iostream>

I2CPlayer::I2CPlayer(const std::string& device, bool verbose_mode,
                     int wait_ms, ErrorAction action, int retries)
    : device_path(device), verbose(verbose_mode),
      i2c_wait_ms(wait_ms), error_action(action),
      retry_count(retries), recording(false) {

    i2c_fd = open(device_path.c_str(), O_RDWR);
    if (i2c_fd < 0) {
        throw std::runtime_error("Failed to open I2C device: " + device_path);
    }
}

I2CPlayer::~I2CPlayer() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
    }
}

void I2CPlayer::registerParser(const std::string& device_name, 
                             std::unique_ptr<I2CDeviceParser> parser) {
    parsers[device_name] = std::move(parser);
}

bool I2CPlayer::checkNAK(const char* operation) {
    if (errno == ENXIO || errno == EIO) {
        std::cerr << "NAK detected during " << operation << "\n";

        switch(error_action) {
            case ErrorAction::STOP:
                throw std::runtime_error("I2C NAK - device not responding");
            case ErrorAction::RETRY:
                return true;
            case ErrorAction::CONTINUE:
                std::cerr << "Continuing after NAK...\n";
                return false;
        }
    }
    return false;
}

uint8_t I2CPlayer::readByte(uint8_t addr, uint8_t reg) {
    for(int attempt = 0; attempt <= retry_count; attempt++) {
        try {
            if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
                throw std::runtime_error("Failed to set I2C slave address for reading");
            }

            if (write(i2c_fd, &reg, 1) != 1) {
                if (checkNAK("register write")) {
                    if (attempt < retry_count) continue;
                    throw std::runtime_error("Failed to write register address after retries");
                }
            }

            uint8_t data;
            if (read(i2c_fd, &data, 1) != 1) {
                if (checkNAK("data read")) {
                    if (attempt < retry_count) continue;
                    throw std::runtime_error("Failed to read I2C data after retries");
                }
            }

            if (verbose) {
                std::cout << "Read: 0x" << std::hex << (int)addr
                         << " reg:0x" << (int)reg
                         << " data:0x" << (int)data << std::dec;
                if (attempt > 0) {
                    std::cout << " (retry " << attempt << ")";
                }
                std::cout << "\n";
            }

            return data;

        } catch (const std::exception& e) {
            if (attempt == retry_count) throw;
            if (verbose) {
                std::cout << "Retry " << (attempt + 1) << "/" << retry_count
                         << ": " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms * 2));
        }
    }
    throw std::runtime_error("Read failed after all retries");
}

void I2CPlayer::writeByte(uint8_t addr, uint8_t reg, uint8_t data) {
    for(int attempt = 0; attempt <= retry_count; attempt++) {
        try {
            if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
                throw std::runtime_error("Failed to set I2C slave address");
            }

            uint8_t buf[2] = {reg, data};
            if (write(i2c_fd, buf, 2) != 2) {
                if (checkNAK("write")) {
                    if (attempt < retry_count) continue;
                    throw std::runtime_error("Failed to write I2C data after retries");
                }
            }

            if (verbose) {
                std::cout << "Write: 0x" << std::hex << (int)addr
                         << " reg:0x" << (int)reg
                         << " data:0x" << (int)data << std::dec;
                if (attempt > 0) {
                    std::cout << " (retry " << attempt << ")";
                }
                std::cout << "\n";
            }
            return;

        } catch (const std::exception& e) {
            if (attempt == retry_count) throw;
            if (verbose) {
                std::cout << "Retry " << (attempt + 1) << "/" << retry_count
                         << ": " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms * 2));
        }
    }
    throw std::runtime_error("Write failed after all retries");
}

void I2CPlayer::writeSingleByte(uint8_t addr, uint8_t data) {
    for(int attempt = 0; attempt <= retry_count; attempt++) {
        try {
            if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
                throw std::runtime_error("Failed to set I2C slave address");
            }

            if (write(i2c_fd, &data, 1) != 1) {
                if (checkNAK("write")) {
                    if (attempt < retry_count) continue;
                    throw std::runtime_error("Failed to write I2C data after retries");
                }
            }
            fsync(i2c_fd);

            if (verbose) {
                std::cout << "Single Write: 0x" << std::hex << (int)addr
                         << " data:0x" << (int)data << std::dec;
                if (attempt > 0) {
                    std::cout << " (retry " << attempt << ")";
                }
                std::cout << "\n";
            }
            return;

        } catch (const std::exception& e) {
            if (attempt == retry_count) throw;
            if (verbose) {
                std::cout << "Retry " << (attempt + 1) << "/" << retry_count
                         << ": " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms * 2));
        }
    }
    throw std::runtime_error("Write failed after all retries");
}

void I2CPlayer::write16Bit(uint8_t addr, uint8_t reg, uint16_t data) {
    for(int attempt = 0; attempt <= retry_count; attempt++) {
        try {
            if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
                throw std::runtime_error("Failed to set I2C slave address");
            }

            uint8_t buf[3] = {
                reg,
                static_cast<uint8_t>(data & 0xFF),
                static_cast<uint8_t>((data >> 8) & 0xFF)
            };

            if (write(i2c_fd, buf, 3) != 3) {
                if (checkNAK("16-bit write")) {
                    if (attempt < retry_count) continue;
                    throw std::runtime_error("Failed to write 16-bit I2C data after retries");
                }
            }

            if (verbose) {
                std::cout << "Write16: 0x" << std::hex << (int)addr
                         << " reg:0x" << (int)reg
                         << " data:0x" << data << std::dec;
                if (attempt > 0) {
                    std::cout << " (retry " << attempt << ")";
                }
                std::cout << "\n";
            }
            return;

        } catch (const std::exception& e) {
            if (attempt == retry_count) throw;
            if (verbose) {
                std::cout << "Retry " << (attempt + 1) << "/" << retry_count
                         << ": " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms * 2));
        }
    }
    throw std::runtime_error("16-bit Write failed after all retries");
}

bool I2CPlayer::pollRegister(uint8_t addr, uint8_t reg, uint8_t mask, uint8_t expected,
                            int timeout_ms, int interval_ms) {
    auto start = std::chrono::steady_clock::now();

    while (true) {
        try {
            uint8_t value = readByte(addr, reg);
            if ((value & mask) == expected) {
                return true;
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            if (elapsed.count() >= timeout_ms) {
                if (verbose) {
                    std::cout << "Polling timeout on register 0x" << std::hex << (int)reg
                             << ": got 0x" << (int)value << ", expected 0x" << (int)expected
                             << " (mask: 0x" << (int)mask << ")" << std::dec << "\n";
                }
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        } catch (const std::exception& e) {
            if (error_action == ErrorAction::STOP) throw;
            if (verbose) {
                std::cout << "Error during polling: " << e.what() << "\n";
            }
            return false;
        }
    }
}

void I2CPlayer::writeFile(uint8_t addr, uint8_t reg, const std::string& filename) {
    std::filesystem::path file_path(filename);
    if (!file_path.is_absolute()) {
        std::filesystem::path csv_path(csv_directory);
        file_path = csv_path.parent_path() / file_path;
    }

    std::ifstream input_file(file_path, std::ios::binary);
    if (!input_file) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    std::vector<uint8_t> data(std::istreambuf_iterator<char>(input_file), {});

    if (verbose) {
        std::cout << "Writing " << data.size() << " bytes from " << file_path << "\n";
    }

    for (uint8_t byte : data) {
        writeByte(addr, reg, byte);
    }
}

std::string I2CPlayer::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

int I2CPlayer::hexToInt(const std::string& hex) {
    std::string cleaned = hex;
    if (cleaned.substr(0, 2) == "0x") {
        cleaned = cleaned.substr(2);
    }
    return std::stoi(cleaned, nullptr, 16);
}

void I2CPlayer::executeSequence(const std::vector<std::string>& sequence, int iterations) {
    if (verbose) {
        std::cout << "Starting loop sequence for " << iterations << " iterations\n";
    }

    for (int iter = 0; iter < iterations; iter++) {
        if (verbose) {
            std::cout << "Loop iteration " << (iter + 1) << "/" << iterations << "\n";
        }

        for (const auto& line : sequence) {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(ss, token, ',')) {
                tokens.push_back(trim(token));
            }

            if (tokens.empty()) continue;

            std::string cmd = tokens[0];
            
            try {
                if (cmd == "WRITE1") {
                    if (tokens.size() != 3) throw std::runtime_error("Invalid WRITE1 format");
                    uint8_t addr = hexToInt(tokens[1]);
                    uint8_t data = hexToInt(tokens[2]);
                    writeSingleByte(addr, data);
                }
                else if (cmd == "DELAY") {
                    if (tokens.size() != 2) throw std::runtime_error("Invalid DELAY format");
                    int delay = std::stoi(tokens[1]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms));
            } catch (const std::exception& e) {
                if (error_action == ErrorAction::STOP) throw;
                std::cerr << "Error in loop sequence: " << e.what() << "\n";
            }
        }
    }

    if (verbose) {
        std::cout << "Loop sequence completed\n";
    }
}

void I2CPlayer::playFile(const std::string& filename) {
    csv_directory = filename;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open input file: " + filename);
    }

    std::string line;
    int line_number = 0;
    bool header_skipped = false;
    std::vector<std::string> loop_sequence;
    bool in_loop = false;
    int loop_count = 0;

    while (std::getline(file, line)) {
        line_number++;

        if (verbose) {
            std::cout << "DEBUG: Line " << line_number << ": [" << line << "]\n";
        }

        if (line.empty() || trim(line)[0] == '#') {
            if (verbose) {
                std::cout << "DEBUG: Skipping line (empty or comment)\n";
            }
            continue;
        }

        if (!header_skipped) {
            header_skipped = true;
            if (verbose) {
                std::cout << "DEBUG: Skipping header line\n";
            }
            continue;
        }

        try {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(ss, token, ',')) {
                std::string trimmed = trim(token);
                if (verbose) {
                    std::cout << "DEBUG: Parsed token: [" << trimmed << "]\n";
                }
                tokens.push_back(trimmed);
            }

            if (tokens.empty()) {
                if (verbose) {
                    std::cout << "DEBUG: Tokens vector is empty\n";
                }
                continue;
            }

            std::string cmd = tokens[0];
            if (verbose) {
                std::cout << "DEBUG: Command: [" << cmd << "]\n";
            }

            if (cmd == "LOOP") {
                if (tokens.size() != 2) throw std::runtime_error("Invalid LOOP format");
                if (in_loop) throw std::runtime_error("Nested loops not supported");
                in_loop = true;
                loop_count = std::stoi(tokens[1]);
                loop_sequence.clear();
                continue;
            }
            else if (cmd == "ENDLOOP") {
                if (!in_loop) throw std::runtime_error("ENDLOOP without LOOP");
                in_loop = false;
                if (verbose) {
                    std::cout << "Executing loop " << loop_count << " times\n";
                }
                executeSequence(loop_sequence, loop_count);
                continue;
            }

            if (cmd == "START_RECORD") {
                if (tokens.size() != 2) throw std::runtime_error("Invalid START_RECORD format");
                int buffer_size = std::stoi(tokens[1]);
                record_buffer.clear();
                record_buffer.reserve(buffer_size);
                recording = true;
                continue;
            }
            else if (cmd == "STOP_RECORD") {
                recording = false;
                continue;
            }
            else if (cmd == "PRINT_RECORD") {
                if (tokens.size() != 2) throw std::runtime_error("Invalid PRINT_RECORD format");
                auto parser = parsers.find(tokens[1]);
                if (parser != parsers.end()) {
                    parser->second->parse(record_buffer);
                } else {
                    std::cerr << "No parser found for device: " << tokens[1] << "\n";
                }
                continue;
            }

            if (in_loop) {
                loop_sequence.push_back(line);
                continue;
            }

            if (cmd == "WRITE") {
                if (tokens.size() != 4) throw std::runtime_error("Invalid WRITE format");
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t reg = hexToInt(tokens[2]);
                uint8_t data = hexToInt(tokens[3]);
                writeByte(addr, reg, data);
            }
            else if (cmd == "WRITE1") {
                if (tokens.size() != 3) throw std::runtime_error("Invalid WRITE1 format");
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t data = hexToInt(tokens[2]);
                writeSingleByte(addr, data);
            }
            else if (cmd == "WRITE16") {
                if (verbose) {
                    std::cout << "DEBUG: Entering WRITE16 processing\n";
                }
                if (tokens.size() != 4) {
                    if (verbose) {
                        std::cout << "DEBUG: Invalid WRITE16 format\n";
                    }
                    throw std::runtime_error("Invalid WRITE16 format");
                }
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t reg = hexToInt(tokens[2]);
                uint16_t data = hexToInt(tokens[3]);
                if (verbose) {
                    std::cout << "DEBUG: WRITE16 parameters:"
                             << " addr=0x" << std::hex << (int)addr
                             << " reg=0x" << (int)reg
                             << " data=0x" << data << std::dec << "\n";
                }
                write16Bit(addr, reg, data);
            }
            else if (cmd == "READ") {
                if (tokens.size() != 3) throw std::runtime_error("Invalid READ format");
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t reg = hexToInt(tokens[2]);
                uint8_t data = readByte(addr, reg);

                if (recording && record_buffer.size() < record_buffer.capacity()) {
                    record_buffer.push_back(data);
                }
            }
            else if (cmd == "POLL") {
                if (tokens.size() != 7) throw std::runtime_error("Invalid POLL format");
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t reg = hexToInt(tokens[2]);
                uint8_t mask = hexToInt(tokens[3]);
                uint8_t expected = hexToInt(tokens[4]);
                int timeout = std::stoi(tokens[5]);
                int interval = std::stoi(tokens[6]);
                if (!pollRegister(addr, reg, mask, expected, timeout, interval)) {
                    throw std::runtime_error("Polling timeout");
                }
            }
            else if (cmd == "DELAY") {
                if (tokens.size() != 2) throw std::runtime_error("Invalid DELAY format");
                int delay = std::stoi(tokens[1]);
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
            else if (cmd == "FILE") {
                if (tokens.size() != 4) throw std::runtime_error("Invalid FILE format");
                uint8_t addr = hexToInt(tokens[1]);
                uint8_t reg = hexToInt(tokens[2]);
                writeFile(addr, reg, tokens[3]);
            }
            else {
                throw std::runtime_error("Unknown command: " + cmd);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i2c_wait_ms));
        } catch (const std::exception& e) {
            std::cerr << "Error at line " << line_number << ": " << e.what() << "\n";
            if (error_action == ErrorAction::STOP) throw;
        }
    }

    if (in_loop) {
        throw std::runtime_error("Unterminated LOOP in CSV");
    }
}
