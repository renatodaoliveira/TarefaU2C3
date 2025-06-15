#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h" // Para i2c_inst_t

// Definições de dimensões e endereço I2C do display
#define SSD1306_HEIGHT 64
#define SSD1306_WIDTH 128
#define SSD1306_I2C_ADDR _u(0x3C)

// Comandos do controlador SSD1306
#define SSD1306_SET_CONTRAST          _u(0x81)
#define SSD1306_DISPLAY_ALL_ON_RESUME _u(0xA4)
#define SSD1306_DISPLAY_ALL_ON        _u(0xA5)
#define SSD1306_NORMAL_DISPLAY        _u(0xA6)
#define SSD1306_INVERT_DISPLAY        _u(0xA7)
#define SSD1306_DISPLAY_OFF           _u(0xAE)
#define SSD1306_DISPLAY_ON            _u(0xAF)
#define SSD1306_SET_DISPLAY_OFFSET    _u(0xD3)
#define SSD1306_SET_COM_PINS          _u(0xDA)
#define SSD1306_SET_VCOM_DETECT       _u(0xDB)
#define SSD1306_SET_DISPLAY_CLOCK_DIV _u(0xD5)
#define SSD1306_SET_PRECHARGE         _u(0xD9)
#define SSD1306_SET_MULTIPLEX         _u(0xA8)
#define SSD1306_SET_LOW_COLUMN        _u(0x00)
#define SSD1306_SET_HIGH_COLUMN       _u(0x10)
#define SSD1306_SET_START_LINE        _u(0x40)
#define SSD1306_MEMORY_MODE           _u(0x20)
#define SSD1306_COLUMN_ADDR           _u(0x21)
#define SSD1306_PAGE_ADDR             _u(0x22)
#define SSD1306_COM_SCAN_INC          _u(0xC0)
#define SSD1306_COM_SCAN_DEC          _u(0xC8)
#define SSD1306_SEG_REMAP             _u(0xA0)
#define SSD1306_CHARGE_PUMP           _u(0x8D)
#define SSD1306_EXTERNAL_VCC          0x1
#define SSD1306_SWITCH_CAP_VCC        0x2

// Comandos de scroll
#define SSD1306_ACTIVATE_SCROLL               _u(0x2F)
#define SSD1306_DEACTIVATE_SCROLL             _u(0x2E)
#define SSD1306_SET_VERTICAL_SCROLL_AREA    _u(0xA3)
#define SSD1306_RIGHT_HORIZONTAL_SCROLL       _u(0x26)
#define SSD1306_LEFT_HORIZONTAL_SCROLL        _u(0x27)
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL _u(0x29)
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  _u(0x2A)

// Constantes relacionadas ao buffer e páginas
#define SSD1306_PAGE_HEIGHT         _u(8)
#define ssd1306_n_pages             (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define ssd1306_buffer_length       (ssd1306_n_pages * SSD1306_WIDTH)

// Estrutura para definir uma área de renderização
struct render_area {
    uint8_t start_column;
    uint8_t end_column;
    uint8_t start_page;
    uint8_t end_page;
    int buffer_length; // Comprimento do buffer para esta área
};

// Funções de baixo nível para o driver SSD1306
void ssd1306_init();
void ssd1306_send_cmd(uint8_t cmd);
void ssd1306_send_cmd_list(const uint8_t *cmd_list, int size);
void ssd1306_send_buffer(const uint8_t *buf, int buflen);
void ssd1306_render(const uint8_t *buf, struct render_area *area);
void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on);
void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1, bool on);
void ssd1306_draw_char(uint8_t *buf, int16_t x, int16_t y, uint8_t character);
void ssd1306_draw_string(uint8_t *buf, int16_t x, int16_t y, const char *str);
void ssd1306_draw_utf8_string(uint8_t *buf, int16_t x, int16_t y, const char *utf8_str);
void ssd1306_draw_utf8_multiline(uint8_t *buf, int16_t x, int16_t y, const char *utf8_str);
void calculate_render_area_buffer_length(struct render_area *area);

#endif