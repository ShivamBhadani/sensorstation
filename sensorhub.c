#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-1"
#define OLED_ADDR  0x3C
#define MPU6050_ADDR 0x68
#define BMP280_ADDR  0x76

int open_i2c_device(uint8_t addr) {
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return -1;
    }
    return fd;
}

// === OLED Basic Display ===
void access_oled() {
    int fd = open_i2c_device(OLED_ADDR);
    if (fd < 0) return;

    uint8_t init_cmds[] = {
        0xAE, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8,
        0xDA, 0x12, 0x81, 0x7F, 0xA4, 0xA6, 0xD5, 0x80,
        0x8D, 0x14, 0xAF
    };
    for (int i = 0; i < sizeof(init_cmds); i++) {
        uint8_t buf[2] = {0x00, init_cmds[i]};
        write(fd, buf, 2);
    }

    // Clear screen
    for (uint8_t page = 0; page < 8; page++) {
        uint8_t cmd[] = {0x00, 0xB0 + page, 0x00, 0x10};
        write(fd, cmd, 4);

        uint8_t data[129] = {0x40};
        memset(data + 1, 0x00, 128);
        write(fd, data, 129);
    }

    printf("OLED accessed and cleared.\n");
    close(fd);
}

// === MPU6050 Read WHO_AM_I ===
void access_mpu6050() {
    int fd = open_i2c_device(MPU6050_ADDR);
    if (fd < 0) return;

    uint8_t reg = 0x75;  // WHO_AM_I register
    write(fd, &reg, 1);
    uint8_t whoami;
    if (read(fd, &whoami, 1) == 1) {
        printf("MPU6050 WHO_AM_I: 0x%X\n", whoami); // Should be 0x68
    } else {
        printf("Failed to read from MPU6050\n");
    }

    close(fd);
}

// === BMP280 Read Chip ID ===
void access_bmp280() {
    int fd = open_i2c_device(BMP280_ADDR);
    if (fd < 0) return;

    uint8_t reg = 0xD0;  // Chip ID register
    write(fd, &reg, 1);
    uint8_t chip_id;
    if (read(fd, &chip_id, 1) == 1) {
        printf("BMP280 Chip ID: 0x%X\n", chip_id); // Should be 0x58 or 0x60
    } else {
        printf("Failed to read from BMP280\n");
    }

    close(fd);
}

int main() {
    int choice;
    printf("Select sensor to access:\n");
    printf("1. OLED 128x64 (0x3C)\n");
    printf("2. MPU6050 (0x68)\n");
    printf("3. BMP280 (0x76)\n");
    printf("Enter choice (1-3): ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            access_oled();
            break;
        case 2:
            access_mpu6050();
            break;
        case 3:
            access_bmp280();
            break;
        default:
            printf("Invalid choice.\n");
    }

    return 0;
}

