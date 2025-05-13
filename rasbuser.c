#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_DEVICE "/dev/i2c-1"
#define OLED_ADDR  0x3C

#define WIDTH 128
#define HEIGHT 64

int i2c_fd;

// Write command to SSD1306
void ssd1306_command(uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = 0x00; // Control byte for command
    buffer[1] = cmd;
    write(i2c_fd, buffer, 2);
}

// Write data to SSD1306
void ssd1306_data(uint8_t *data, size_t size) {
    uint8_t *buffer = malloc(size + 1);
    buffer[0] = 0x40; // Control byte for data
    memcpy(buffer + 1, data, size);
    write(i2c_fd, buffer, size + 1);
    free(buffer);
}

// Initialize SSD1306
void ssd1306_init() {
    ssd1306_command(0xAE); // Display off
    ssd1306_command(0xA8); // Set MUX ratio
    ssd1306_command(0x3F); // 1/64 duty
    ssd1306_command(0xD3); ssd1306_command(0x00); // Display offset
    ssd1306_command(0x40); // Display start line
    ssd1306_command(0xA1); // Segment remap
    ssd1306_command(0xC8); // COM output scan direction
    ssd1306_command(0xDA); ssd1306_command(0x12); // COM pins hardware config
    ssd1306_command(0x81); ssd1306_command(0x7F); // Contrast
    ssd1306_command(0xA4); // Entire display ON from RAM
    ssd1306_command(0xA6); // Normal display
    ssd1306_command(0xD5); ssd1306_command(0x80); // Oscillator frequency
    ssd1306_command(0x8D); ssd1306_command(0x14); // Enable charge pump
    ssd1306_command(0xAF); // Display ON
}

// Clear display
void ssd1306_clear() {
    uint8_t buffer[WIDTH];
    memset(buffer, 0, WIDTH);

    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_command(0xB0 + page); // Page address
        ssd1306_command(0x00);        // Lower column
        ssd1306_command(0x10);        // Higher column
        ssd1306_data(buffer, WIDTH);
    }
}

int main() {
    if ((i2c_fd = open(I2C_DEVICE, O_RDWR)) < 0) {
        perror("Failed to open I2C device");
        exit(1);
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Failed to connect to OLED");
        close(i2c_fd);
        exit(1);
    }

    ssd1306_init();
    ssd1306_clear();

    // Draw simple pattern (horizontal line on page 3)
    uint8_t line[WIDTH];
    memset(line, 0xFF, WIDTH); // full line

    ssd1306_command(0xB3); // Page 3
    ssd1306_command(0x00); // Column low
    ssd1306_command(0x10); // Column high
    ssd1306_data(line, WIDTH);

    close(i2c_fd);
    return 0;
}

