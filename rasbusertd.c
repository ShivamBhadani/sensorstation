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

// 5x8 font for 'T' and 'D'
uint8_t font_T[5] = {
    0x00, // row 0
    0x7F, // row 1
    0x01, // row 2
    0x01, // row 3
    0x01  // row 4;
};
uint8_t font_D[5] = {
    0x00,
    0xFE, // #######
    0x82, // #     #
    0x82, // #     #
    0x7C  //  #####
};

// Write command to SSD1306
void ssd1306_command(uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = 0x00;
    buffer[1] = cmd;
    write(i2c_fd, buffer, 2);
}

// Write data to SSD1306
void ssd1306_data(uint8_t *data, size_t size) {
    uint8_t *buffer = malloc(size + 1);
    buffer[0] = 0x40;
    memcpy(buffer + 1, data, size);
    write(i2c_fd, buffer, size + 1);
    free(buffer);
}

// Initialize display
void ssd1306_init() {
    ssd1306_command(0xAE);
    ssd1306_command(0xA8); ssd1306_command(0x3F);
    ssd1306_command(0xD3); ssd1306_command(0x00);
    ssd1306_command(0x40);
    ssd1306_command(0xA1);
    ssd1306_command(0xC8);
    ssd1306_command(0xDA); ssd1306_command(0x12);
    ssd1306_command(0x81); ssd1306_command(0x7F);
    ssd1306_command(0xA4);
    ssd1306_command(0xA6);
    ssd1306_command(0xD5); ssd1306_command(0x80);
    ssd1306_command(0x8D); ssd1306_command(0x14);
    ssd1306_command(0xAF);
}

// Clear screen
void ssd1306_clear() {
    uint8_t buffer[WIDTH];
    memset(buffer, 0, WIDTH);
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_command(0xB0 + page);
        ssd1306_command(0x00);
        ssd1306_command(0x10);
        ssd1306_data(buffer, WIDTH);
    }
}

// Set position (page and column)
void ssd1306_set_cursor(uint8_t page, uint8_t col) {
    ssd1306_command(0xB0 + page);             // Page address
    ssd1306_command(0x00 + (col & 0x0F));     // Lower column
    ssd1306_command(0x10 + ((col >> 4) & 0x0F)); // Higher column
}

int main() {
    if ((i2c_fd = open(I2C_DEVICE, O_RDWR)) < 0) {
        perror("Failed to open I2C device");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Failed to connect to OLED");
        close(i2c_fd);
        return 1;
    }

    ssd1306_init();
    ssd1306_clear();

    // Write 'T'
    ssd1306_set_cursor(2, 10);
    ssd1306_data(font_T, 5);

    // Space between letters
    uint8_t space = 0x00;
    ssd1306_data(&space, 1);

    // Write 'D'
    ssd1306_data(font_D, 5);

    close(i2c_fd);
    return 0;
}

