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

// Basic 5x8 font for letters needed (ASCII lowercase + space)
const uint8_t font5x8[][5] = {
    // 'a' to 'z'
    {0x7C,0x12,0x11,0x12,0x7C}, // a
    {0x7F,0x49,0x49,0x49,0x36}, // b
    {0x3E,0x41,0x41,0x41,0x22}, // c
    {0x7F,0x41,0x41,0x22,0x1C}, // d
    {0x7F,0x49,0x49,0x49,0x41}, // e
    {0x7F,0x09,0x09,0x09,0x01}, // f
    {0x3E,0x41,0x49,0x49,0x7A}, // g
    {0x7F,0x08,0x08,0x08,0x7F}, // h
    {0x00,0x41,0x7F,0x41,0x00}, // i
    {0x20,0x40,0x41,0x3F,0x01}, // j
    {0x7F,0x08,0x14,0x22,0x41}, // k
    {0x7F,0x40,0x40,0x40,0x40}, // l
    {0x7F,0x02,0x0C,0x02,0x7F}, // m
    {0x7F,0x04,0x08,0x10,0x7F}, // n
    {0x3E,0x41,0x41,0x41,0x3E}, // o
    {0x7F,0x09,0x09,0x09,0x06}, // p
    {0x3E,0x41,0x51,0x21,0x5E}, // q
    {0x7F,0x09,0x19,0x29,0x46}, // r
    {0x46,0x49,0x49,0x49,0x31}, // s
    {0x01,0x01,0x7F,0x01,0x01}, // t
    {0x3F,0x40,0x40,0x40,0x3F}, // u
    {0x1F,0x20,0x40,0x20,0x1F}, // v
    {0x7F,0x20,0x18,0x20,0x7F}, // w
    {0x63,0x14,0x08,0x14,0x63}, // x
    {0x07,0x08,0x70,0x08,0x07}, // y
    {0x61,0x51,0x49,0x45,0x43}, // z
    // space
    {0x00,0x00,0x00,0x00,0x00}  // space
};

// Map characters to font index
int get_font_index(char c) {
    if (c >= 'a' && c <= 'z')
        return c - 'a';
    else if (c == ' ')
        return 26;
    else
        return 26; // fallback to space
}

// I2C Command
void ssd1306_command(uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd};
    write(i2c_fd, buffer, 2);
}

// I2C Data
void ssd1306_data(uint8_t *data, size_t size) {
    uint8_t *buffer = malloc(size + 1);
    buffer[0] = 0x40;
    memcpy(buffer + 1, data, size);
    write(i2c_fd, buffer, size + 1);
    free(buffer);
}

// Init Display
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

// Clear Screen
void ssd1306_clear() {
    uint8_t buffer[WIDTH] = {0};
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_command(0xB0 + page);
        ssd1306_command(0x00);
        ssd1306_command(0x10);
        ssd1306_data(buffer, WIDTH);
    }
}

// Set cursor
void ssd1306_set_cursor(uint8_t page, uint8_t col) {
    ssd1306_command(0xB0 + page);
    ssd1306_command(0x00 + (col & 0x0F));
    ssd1306_command(0x10 + ((col >> 4) & 0x0F));
}

// Draw character
void ssd1306_draw_char(char c) {
    int idx = get_font_index(c);
    ssd1306_data((uint8_t *)font5x8[idx], 5);

    uint8_t space = 0x00;
    ssd1306_data(&space, 1); // 1px spacing
}

// Draw string
void ssd1306_draw_string(uint8_t page, uint8_t col, const char *str) {
    ssd1306_set_cursor(page, col);
    while (*str) {
        ssd1306_draw_char(*str++);
    }
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

    // Draw name on page 0
    ssd1306_draw_string(0, 0, "tejsaw bhasker");

    close(i2c_fd);
    return 0;
}

