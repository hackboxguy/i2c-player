#include "i2c_player.hpp"
#include "error_action.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

// Parser includes
#include "parsers/ds3231_parser.hpp"
#include "parsers/ads1015_parser.hpp"
#include "parsers/bmp280_parser.hpp"
#include "parsers/bh1750_parser.hpp"
#include "parsers/veml7700_parser.hpp"
#include "parsers/eeprom_parser.hpp"

void printUsage(const char* progname) {
    std::cerr << "Usage: " << progname << " --input=<csv_file> --device=<i2c_device> [OPTIONS]\n"
              << "Options:\n"
              << "  --input=<file>       Input CSV file with I2C transactions\n"
              << "  --device=<dev>       I2C device (e.g., /dev/i2c-0)\n"
              << "  --verbose            Enable verbose output\n"
              << "  --i2cwaitms=<ms>     Wait time between I2C operations in milliseconds (default: 1)\n"
              << "  --onerror=<action>   Action on NAK/error: stop|retry|continue (default: stop)\n"
              << "  --retries=<n>        Number of retries on error (default: 3)\n"
              << "\nSupported CSV commands:\n"
              << "  WRITE,addr,reg,data          Write single byte\n"
              << "  WRITE1,addr,data             Write single byte without register\n"
              << "  WRITE16,addr,reg,data        Write 16-bit value\n"
              << "  READ,addr,reg                Read single byte\n"
              << "  POLL,addr,reg,mask,exp,t,i   Poll register with timeout\n"
              << "  DELAY,milliseconds           Insert delay\n"
              << "  FILE,addr,reg,filename       Write file contents\n"
              << "  LOOP,count                   Start loop block\n"
              << "  ENDLOOP                      End loop block\n"
              << "  START_RECORD,size            Start recording reads\n"
              << "  STOP_RECORD                  Stop recording reads\n"
              << "  PRINT_RECORD,device          Parse and print recorded data\n"
              << "\nExample: " << progname << " --input=init-serializer.csv --device=/dev/i2c-0 --onerror=retry\n";
}

// Function to register all available device parsers
void registerParsers(I2CPlayer& player) {
    player.registerParser("DS3231", std::make_unique<DS3231Parser>());
    player.registerParser("ADS1015", std::make_unique<ADS1015Parser>());
    player.registerParser("24C02", std::make_unique<EEPROMParser>());
    player.registerParser("BMP280", std::make_unique<BMP280Parser>());
    player.registerParser("BH1750", std::make_unique<BH1750Parser>());
    player.registerParser("VEML7700", std::make_unique<VEML7700Parser>());
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string i2c_device;
    bool verbose = false;
    int i2c_wait_ms = 1;
    ErrorAction error_action = ErrorAction::STOP;
    int retries = 3;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 8) == "--input=") {
            input_file = arg.substr(8);
        } else if (arg.substr(0, 9) == "--device=") {
            i2c_device = arg.substr(9);
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg.substr(0, 11) == "--i2cwaitms=") {
            i2c_wait_ms = std::stoi(arg.substr(11));
        } else if (arg.substr(0, 10) == "--onerror=") {
            std::string action = arg.substr(10);
            if (action == "stop") error_action = ErrorAction::STOP;
            else if (action == "retry") error_action = ErrorAction::RETRY;
            else if (action == "continue") error_action = ErrorAction::CONTINUE;
            else throw std::runtime_error("Invalid error action: " + action);
        } else if (arg.substr(0, 10) == "--retries=") {
            retries = std::stoi(arg.substr(10));
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate required arguments
    if (input_file.empty() || i2c_device.empty()) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        // Create I2C player instance
        I2CPlayer player(i2c_device, verbose, i2c_wait_ms, error_action, retries);
        
        // Register all available device parsers
        registerParsers(player);

        // Execute the I2C commands from the CSV file
        player.playFile(input_file);
        
        if (verbose) {
            std::cout << "I2C sequence completed successfully\n";
        }
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
