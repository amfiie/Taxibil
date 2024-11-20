#include "i2c-master.hpp"

#include <fcntl.h>
#include <fmt/core.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c.h>

I2cMaster::I2cMaster(const int gpio_id) {
    device.filename = fmt::format("/dev/i2c-{}", gpio_id);
}

I2cMaster::~I2cMaster() {
    close(device.fd);
}

int I2cMaster::initialise() {
    device.fd = open(device.filename.c_str(), I2C_RDWR);
    
    if (device.fd < 0) {
        return device.fd;
    }

   /*auto status = ioctl(device.fd, I2C_SLAVE, device.addr);

    if (status < 0) {
        fmt::print("ioctl failed :(\n");
        return status;
    }*/
    
    return 0;
}

int I2cMaster::read_slave(uint8_t buffer[], size_t buffer_length, uint8_t addr) {
    i2c_msg msgs[1];
    i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = addr;
    msgs[0].flags = I2C_M_RD;
    msgs[0].len = buffer_length;
    msgs[0].buf = buffer;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;
    int val = ioctl(device.fd, I2C_RDWR, &msgset);

    if (val < 0) {
         perror("ioctl(I2C_RDWR) in i2c_write");
	 return -1;
    }

    return 0;
}

int I2cMaster::write_slave(uint8_t buffer[], size_t buffer_length, uint8_t addr) {
    i2c_msg msgs[1];
    i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = addr;
    msgs[0].flags = 0;
    msgs[0].len = buffer_length;
    msgs[0].buf = buffer;


    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;
 
    if (ioctl(device.fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_write");
	return -1;
    }

    return 0;
}
