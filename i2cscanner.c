#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE "/dev/i2c-1"

int main() {
    int file;
    char *filename = I2C_DEVICE;
    int addr;

    // Open I2C bus
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        return 1;
    }

    printf("Scanning I2C bus on %s...\n", filename);

    for (addr = 0x03; addr <= 0x77; addr++) {
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
            continue;
        }

        // Try to write a single byte (dummy write)
        // to check if device responds
        char buf[1] = {0x00};
        if (write(file, buf, 1) == 1) {
            printf("Found device at 0x%02X\n", addr);
        }
        // Some devices don't accept writes, try read
        else if (read(file, buf, 1) == 1) {
            printf("Found (RO) device at 0x%02X\n", addr);
        }
    }

    close(file);
    return 0;
}

