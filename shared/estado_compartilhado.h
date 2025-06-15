#ifndef ESTADO_COMPARTILHADO_H
#define ESTADO_COMPARTILHADO_H

#include <stdint.h>
#include <stdbool.h>
#include "drivers/oled_ssd1306/oled_driver.h" // Para struct render_area e ssd1306_buffer_length

// Variáveis compartilhadas entre arquivos e núcleos
extern uint32_t ultimo_ip_bin; // Último IP válido recebido pelo núcleo 1
extern bool mqtt_iniciado;     // Flag para indicar se o cliente MQTT foi iniciado

// Buffer OLED e área de renderização globais
extern uint8_t buffer_oled[]; // Buffer de vídeo para o display OLED
extern struct render_area area; // Área da tela a ser desenhada

#endif