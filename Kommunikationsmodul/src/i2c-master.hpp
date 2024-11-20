#include <cstdint>
#include <optional>
#include <string>

#include <linux/i2c-dev.h>

struct I2cDevice {
    std::string filename;
    int fd;
};

class I2cMaster {
public:
    I2cMaster(const int gpio_id);
    ~I2cMaster();
    int initialise();
    int read_slave(uint8_t buffer[], size_t buffer_length, uint8_t addr);
    int write_slave(uint8_t buffer[], size_t buffer_length, uint8_t addr);
private:
    I2cDevice device;
};
