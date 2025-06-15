/**
 * @file oled_driver.c
 * @brief Implementação de baixo nível para o controlador de display OLED SSD1306 via I2C.
 * Baseado no código original de ssd1306_i2c.c.
 */

#include "drivers/oled_ssd1306/oled_driver.h"
#include "drivers/oled_ssd1306/ssd1306_font.h" // Para a fonte de caracteres
#include "config/config_geral.h"         // Para SDA_PIN, SCL_PIN e I2C_PORT (i2c1)
#include "hardware/i2c.h"
#include <string.h> // Para memset, memcpy
#include <stdlib.h> // Para malloc, free
#include <ctype.h>  // Para toupper (se usado)
#include "pico/stdlib.h" // Para assert

// O i2c_inst_t usado é i2c1, conforme setup_oled.c original
#define I2C_PORT i2c1

void calculate_render_area_buffer_length(struct render_area *area) {
    area->buffer_length = (area->end_column - area->start_column + 1) * (area->end_page - area->start_page + 1);
}

void ssd1306_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd}; // 0x80 para Co=0, D/C#=0 (comando)
    i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, buf, 2, false);
}

void ssd1306_send_cmd_list(const uint8_t *cmd_list, int size) {
    for (int i = 0; i < size; i++) {
        ssd1306_send_cmd(cmd_list[i]);
    }
}

void ssd1306_send_buffer(const uint8_t *buf, int buflen) {
    // Para enviar dados, o primeiro byte é 0x40 (Co=0, D/C#=1)
    uint8_t *temp_buf = malloc(buflen + 1);
    if (!temp_buf) return;

    temp_buf[0] = 0x40; // Byte de controle para dados
    memcpy(temp_buf + 1, buf, buflen);

    i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);
    free(temp_buf);
}

void ssd1306_init() {
    // Sequência de inicialização para display 128x64
    const uint8_t cmds[] = {
        SSD1306_DISPLAY_OFF,
        SSD1306_SET_DISPLAY_CLOCK_DIV,
        0x80, // Ratio
        SSD1306_SET_MULTIPLEX,
        SSD1306_HEIGHT - 1,
        SSD1306_SET_DISPLAY_OFFSET,
        0x00, // Sem offset
        SSD1306_SET_START_LINE | 0x00,
        SSD1306_CHARGE_PUMP,
        0x14, // Habilitar charge pump
        SSD1306_MEMORY_MODE,
        0x00, // Modo de endereçamento horizontal
        SSD1306_SEG_REMAP | 0x01, // Mapear coluna 127 para SEG0
        SSD1306_COM_SCAN_DEC,     // Escanear de COM[N-1] para COM0
        SSD1306_SET_COM_PINS,
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
    0x02,
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
    0x12,
#else // Padrão para outros, ou se não especificado corretamente
    0x12, // Assumindo 64
#endif
        SSD1306_SET_CONTRAST,
        0xFF, // Contraste máximo
        SSD1306_SET_PRECHARGE,
        0xF1, // Fase 1: 1 DCLK, Fase 2: 15 DCLKs
        SSD1306_SET_VCOM_DETECT,
        0x30, // 0.83 * Vcc
        SSD1306_DISPLAY_ALL_ON_RESUME, // Sair do modo "display all on"
        SSD1306_NORMAL_DISPLAY,
        SSD1306_DEACTIVATE_SCROLL,
        SSD1306_DISPLAY_ON
    };
    ssd1306_send_cmd_list(cmds, sizeof(cmds));
}

void ssd1306_render(const uint8_t *buf, struct render_area *area) {
    uint8_t cmds[] = {
        SSD1306_COLUMN_ADDR,
        area->start_column,
        area->end_column,
        SSD1306_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    ssd1306_send_cmd_list(cmds, sizeof(cmds));
    ssd1306_send_buffer(buf, area->buffer_length);
}

void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
        return;
    }
    // O buffer é organizado por páginas, cada página tem 8 linhas de pixels.
    // Cada byte no buffer representa uma coluna de 8 pixels.
    int byte_idx = (y / 8) * SSD1306_WIDTH + x;
    uint8_t bit_idx = y % 8;

    if (on) {
        buf[byte_idx] |= (1 << bit_idx);
    } else {
        buf[byte_idx] &= ~(1 << bit_idx);
    }
}

void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1, bool on) {
    // Algoritmo de Bresenham
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true) {
        ssd1306_set_pixel(buf, x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// CORREÇÃO APLICADA AQUI:
// Esta função agora espera o caractere já decodificado (ASCII ou código Latin-1 da fonte)
static inline int get_font_char_offset(uint8_t character_code) {
    if (character_code >= 'A' && character_code <= 'Z') return character_code - 'A' + 1;
    if (character_code >= '0' && character_code <= '9') return character_code - '0' + 27;
    if (character_code >= 'a' && character_code <= 'z') return character_code - 'a' + 37;
    
    switch (character_code) {
        case '.': return 63;
        case ':': return 64;
        case '#': return 65;
        case '!': return 66;
        case '?': return 67;
        // Valores que correspondem aos códigos da fonte para caracteres acentuados
        // Estes são os valores retornados por utf8_to_font_code
        case 195: return 68; // Ã (Latin-1)
        case 194: return 69; // Â (Latin-1)
        case 193: return 70; // Á (Latin-1)
        case 192: return 71; // À (Latin-1)
        case 201: return 72; // É (Latin-1)
        case 202: return 73; // Ê (Latin-1)
        case 205: return 74; // Í (Latin-1)
        case 211: return 75; // Ó (Latin-1)
        case 212: return 76; // Ô (Latin-1)
        case 213: return 77; // Õ (Latin-1)
        case 218: return 78; // Ú (Latin-1)
        case 199: return 79; // Ç (Latin-1)
        case 231: return 80; // ç (Latin-1)
        case 227: return 81; // ã (Latin-1)
        case 225: return 82; // á (Latin-1)
        case 224: return 83; // à (Latin-1)
        case 226: return 84; // â (Latin-1)
        case 233: return 85; // é (Latin-1)
        case 234: return 86; // ê (Latin-1)
        case 237: return 87; // í (Latin-1)
        case 243: return 88; // ó (Latin-1)
        case 244: return 89; // ô (Latin-1)
        case 250: return 90; // ú (Latin-1)
        case ',': return 91;
        case '-': return 92;
        default: return 0; // Caractere desconhecido ou espaço (índice 0 é vazio)
    }
}


void ssd1306_draw_char(uint8_t *buf, int16_t x, int16_t y, uint8_t character_code) {
    if (x < 0 || x > SSD1306_WIDTH - 8 || y < 0 || y > SSD1306_HEIGHT - 8) {
        return;
    }

    int font_offset = get_font_char_offset(character_code);
    int buffer_page = y / 8; // A coordenada Y para o char é a página
    int buffer_start_index = buffer_page * SSD1306_WIDTH + x;

    for (int i = 0; i < 8; i++) { // Cada caractere tem 8 colunas (8 bytes)
        if (buffer_start_index + i < ssd1306_buffer_length) { // Checagem de limite do buffer
             buf[buffer_start_index + i] = font[font_offset * 8 + i];
        }
    }
}

void ssd1306_draw_string(uint8_t *buf, int16_t x, int16_t y, const char *str) {
    int16_t current_x = x;
    while (*str) {
        if (current_x > SSD1306_WIDTH - 8) break; // Evita escrever fora da tela
        ssd1306_draw_char(buf, current_x, y, (uint8_t)*str);
        current_x += 8; // Avança 8 pixels para o próximo caractere
        str++;
    }
}

// Função para converter um par de bytes UTF-8 em um caractere Latin-1 aproximado (ou código de fonte)
static uint8_t utf8_to_font_code(uint8_t c1, uint8_t c2) {
    // Tabela de conversão simplificada de UTF-8 (2 bytes) para os códigos da fonte.
    // Isso é específico para os caracteres suportados pela fonte.
    // (c1 << 8 | c2) forma um uint16_t para facilitar o switch.
    uint16_t utf8_pair = (c1 << 8) | c2;
    switch (utf8_pair) {
        // MAIÚSCULAS ACENTUADAS
        case 0xC381: return 193; // Á
        case 0xC380: return 192; // À
        case 0xC382: return 194; // Â
        case 0xC383: return 195; // Ã
        case 0xC387: return 199; // Ç
        case 0xC389: return 201; // É
        case 0xC38A: return 202; // Ê
        case 0xC38D: return 205; // Í
        case 0xC393: return 211; // Ó
        case 0xC394: return 212; // Ô
        case 0xC395: return 213; // Õ
        case 0xC39A: return 218; // Ú
        // MINÚSCULAS ACENTUADAS
        case 0xC3A1: return 225; // á
        case 0xC3A0: return 224; // à
        case 0xC3A2: return 226; // â
        case 0xC3A3: return 227; // ã
        case 0xC3A7: return 231; // ç
        case 0xC3A9: return 233; // é
        case 0xC3AA: return 234; // ê
        case 0xC3AD: return 237; // í
        case 0xC3B3: return 243; // ó
        case 0xC3B4: return 244; // ô
        // case 0xC3B5: return 245; // õ (não estava na fonte original, mas use 213 (Õ maiúsculo) se precisar ou mapeie para 'o')
                                  // Se a fonte ssd1306_font.h tiver Õ mas não õ, você pode mapear õ para Õ,
                                  // ou para 'o', ou para um '?' se não houver representação.
                                  // A fonte original mapeia Õ para o índice 77 (valor 213). Não tem õ.
                                  // Vamos mapear õ (0xC3B5) para 'o' (índice 51 da fonte) por enquanto.
        case 0xC3B5: return 'o'; // õ -> o (ou 245, se sua fonte tiver e for o código correto)
        case 0xC3BA: return 250; // ú
        default: return '?';     // Caractere desconhecido
    }
}


void ssd1306_draw_utf8_string(uint8_t *buf, int16_t x, int16_t y, const char *utf8_str) {
    int16_t current_x = x;
    const uint8_t *ptr = (const uint8_t *)utf8_str;

    while (*ptr) {
        if (current_x > SSD1306_WIDTH - 8) break;

        uint8_t c1 = *ptr;
        uint8_t char_to_draw;

        if (c1 < 0x80) { // ASCII (1 byte)
            char_to_draw = c1;
            ptr++;
        } else if ((c1 & 0xE0) == 0xC0) { // UTF-8 de 2 bytes
            if (*(ptr + 1)) {
                uint8_t c2 = *(ptr + 1);
                char_to_draw = utf8_to_font_code(c1, c2); // Converte para código da fonte
                ptr += 2;
            } else { // Sequência incompleta
                char_to_draw = '?';
                ptr++;
            }
        } else { // UTF-8 de 3 ou 4 bytes (não suportado pela fonte simples)
            char_to_draw = '?';
            // Avança para o próximo caractere UTF-8
            if ((c1 & 0xF0) == 0xE0) ptr += 3;       // 3-byte
            else if ((c1 & 0xF8) == 0xF0) ptr += 4;  // 4-byte
            else ptr++; // Malformado, avança 1 byte
        }
        
        ssd1306_draw_char(buf, current_x, y, char_to_draw);
        current_x += 8;
    }
}

void ssd1306_draw_utf8_multiline(uint8_t *buf, int16_t x_start, int16_t y_start, const char *utf8_str) {
    int16_t current_x = x_start;
    int16_t current_y = y_start;
    const uint8_t *ptr = (const uint8_t *)utf8_str;

    const int char_width = 8;
    const int char_height = 8; // Altura de cada linha de texto (em pixels)

    while (*ptr && current_y <= (SSD1306_HEIGHT - char_height)) {
        uint8_t c1 = *ptr;
        uint8_t char_to_draw;

        if (c1 == '\n') { // Quebra de linha explícita
            current_x = x_start;
            current_y += char_height;
            ptr++;
            if (current_y > SSD1306_HEIGHT - char_height) break; // Fora da tela
            continue;
        }

        if (c1 < 0x80) { // ASCII
            char_to_draw = c1;
            ptr++;
        } else if ((c1 & 0xE0) == 0xC0) { // UTF-8 de 2 bytes
             if (*(ptr + 1)) {
                uint8_t c2 = *(ptr + 1);
                char_to_draw = utf8_to_font_code(c1, c2);
                ptr += 2;
            } else {
                char_to_draw = '?';
                ptr++;
            }
        } else { // Não suportado
            char_to_draw = '?';
            if ((c1 & 0xF0) == 0xE0) ptr += 3;
            else if ((c1 & 0xF8) == 0xF0) ptr += 4;
            else ptr++;
        }

        if (current_x > SSD1306_WIDTH - char_width) { // Quebra de linha automática
            current_x = x_start;
            current_y += char_height;
            if (current_y > SSD1306_HEIGHT - char_height) break; // Fora da tela
        }
        
        ssd1306_draw_char(buf, current_x, current_y, char_to_draw);
        current_x += char_width;
    }
}