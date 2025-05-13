#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_BUS "/dev/i2c-1"
#define OLED_ADDR  0x3C
#define MPU6050_ADDR 0x68
#define BMP280_ADDR  0x76

// Basic 6x8 font for ASCII characters 32–127
const uint8_t font6x8[][6] = {
    // Only including characters used in "Tejsaw Bhasker"
    [32] = {0x00,0x00,0x00,0x00,0x00,0x00}, // Space
    ['A'] = {0x7E,0x11,0x11,0x11,0x7E,0x00},
    ['B'] = {0x7F,0x49,0x49,0x49,0x36,0x00},
    ['E'] = {0x7F,0x49,0x49,0x49,0x41,0x00},
    ['H'] = {0x7F,0x08,0x08,0x08,0x7F,0x00},
    ['J'] = {0x20,0x40,0x41,0x3F,0x01,0x00},
    ['K'] = {0x7F,0x08,0x14,0x22,0x41,0x00},
    ['R'] = {0x7F,0x09,0x19,0x29,0x46,0x00},
    ['S'] = {0x26,0x49,0x49,0x49,0x32,0x00},
    ['T'] = {0x01,0x01,0x7F,0x01,0x01,0x00},
    ['W'] = {0x7F,0x20,0x18,0x20,0x7F,0x00},
    ['a'] = {0x20,0x54,0x54,0x54,0x78,0x00},
    ['e'] = {0x38,0x54,0x54,0x54,0x18,0x00},
    ['h'] = {0x7F,0x08,0x08,0x08,0x70,0x00},
    ['j'] = {0x40,0x80,0x80,0x7A,0x00,0x00},
    ['k'] = {0x7F,0x10,0x28,0x44,0x00,0x00},
    ['r'] = {0x7C,0x08,0x04,0x04,0x08,0x00},
    ['s'] = {0x48,0x54,0x54,0x54,0x20,0x00},
    ['t'] = {0x04,0x3F,0x44,0x40,0x20,0x00},
    ['w'] = {0x7C,0x10,0x08,0x10,0x7C,0x00}
};

// I2C open
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

// OLED: Send command or data
void ssd1306_command(int fd, uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd};
    write(fd, buffer, 2);
}
void ssd1306_data(int fd, uint8_t *data, size_t size) {
    uint8_t *buffer = malloc(size + 1);
    buffer[0] = 0x40;
    memcpy(buffer + 1, data, size);
    write(fd, buffer, size + 1);
    free(buffer);
}

// OLED: Show text
void access_oled() {
    int fd = open_i2c_device(OLED_ADDR);
    if (fd < 0) return;

    // Init OLED
    uint8_t init_cmds[] = {
        0xAE, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8,
        0xDA, 0x12, 0x81, 0x7F, 0xA4, 0xA6, 0xD5, 0x80,
        0x8D, 0x14, 0xAF
    };
    for (int i = 0; i < sizeof(init_cmds); i++)
        ssd1306_command(fd, init_cmds[i]);

    // Clear screen
    uint8_t clear[128] = {0};
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_command(fd, 0xB0 + page);
        ssd1306_command(fd, 0x00);
        ssd1306_command(fd, 0x10);
        ssd1306_data(fd, clear, 128);
    }

    // Print message
    const char *msg = "Tejsaw Bhasker";
    ssd1306_command(fd, 0xB0); // Page 0
    ssd1306_command(fd, 0x00);
    ssd1306_command(fd, 0x10);

    for (int i = 0; msg[i]; i++) {
        char c = msg[i];
        const uint8_t *bitmap = font6x8[(uint8_t)c];
        ssd1306_data(fd, (uint8_t *)bitmap, 6);
    }

    printf("OLED: Displayed 'Tejsaw Bhasker'\n");
    close(fd);
}

// MPU6050: Read raw accel/gyro data
void access_mpu6050() {
    int fd = open_i2c_device(MPU6050_ADDR);
    if (fd < 0) return;

    uint8_t wake[2] = {0x6B, 0x00};
    write(fd, wake, 2);

    uint8_t reg = 0x3B;
    write(fd, &reg, 1);
    uint8_t data[14];
    read(fd, data, 14);

    int16_t ax = (data[0] << 8) | data[1];
    int16_t ay = (data[2] << 8) | data[3];
    int16_t az = (data[4] << 8) | data[5];
    int16_t gx = (data[8] << 8) | data[9];
    int16_t gy = (data[10] << 8) | data[11];
    int16_t gz = (data[12] << 8) | data[13];

    printf("MPU6050:\n  Accel: X=%d Y=%d Z=%d\n  Gyro:  X=%d Y=%d Z=%d\n",
        ax, ay, az, gx, gy, gz);

    close(fd);
}

// BMP280: Raw temp/pressure (simplified)
void access_bmp280() {
    int fd = open_i2c_device(BMP280_ADDR);
    if (fd < 0) return;

    uint8_t ctrl_meas[2] = {0xF4, 0x27};
    write(fd, ctrl_meas, 2);
    usleep(100000);

    uint8_t reg = 0xF7;
    write(fd, &reg, 1);
    uint8_t data[6];
    read(fd, data, 6);

    int32_t press_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);

    printf("BMP280:\n  Raw Temp: %ld\n  Raw Pressure: %ld\n",
        (long)temp_raw, (long)press_raw);

    close(fd);
}

// === Main Menu ===
int main() {
    int choice;
    printf("Select device to access:\n");
    printf("1. OLED 128x64 (0x3C)\n");
    printf("2. MPU6050 (0x68)\n");
    printf("3. BMP280 (0x76)\n");
    printf("Enter choice (1-3): ");
    scanf("%d", &choice);

    switch (choice) {
        case 1: access_oled(); break;
        case 2: access_mpu6050(); break;
        case 3: access_bmp280(); break;
        default: printf("Invalid choice.\n");
    }

    return 0;
}

