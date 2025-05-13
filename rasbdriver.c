#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_DEVICE "/dev/i2c-1"
#define I2C_ADDR 0x48  // Replace with your device's address

int main() {
    int file;

    // Open I2C bus
    if ((file = open(I2C_DEVICE, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        exit(1);
    }

    // Specify the address of the I2C Slave to communicate with
    if (ioctl(file, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        close(file);
        exit(1);
    }

    // Write a byte to register 0x01
    unsigned char buffer[2];
    buffer[0] = 0x01;     // Register address
    buffer[1] = 0x10;     // Data to write
    if (write(file, buffer, 2) != 2) {
        perror("Failed to write to the I2C device");
    } else {
        printf("Wrote 0x%02X to register 0x%02X\n", buffer[1], buffer[0]);
    }

    // Read a byte from register 0x00
    buffer[0] = 0x00;  // Register to read
    if (write(file, buffer, 1) != 1) {
        perror("Failed to select register for reading");
    } else {
        if (read(file, buffer, 1) != 1) {
            perror("Failed to read from the I2C device");
        } else {
            printf("Read 0x%02X from register 0x00\n", buffer[0]);
        }
    }

    close(file);
    return 0;
}

