#ifndef OLED_INTERFACE_H
#define OLED_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h" // Para i2c_inst_t

// Forward declaration para struct render_area se não for incluído de oled_driver.h diretamente
struct render_area; 

/**
 * @brief Inicializa o barramento I2C e o display OLED.
 * Configura os pinos I2C, inicializa o controlador SSD1306,
 * define a área de renderização global e limpa o display.
 * Usa as definições de pinos e porta I2C de config_geral.h e oled_driver.h.
 */
void oled_setup_interface();

/**
 * @brief Limpa o buffer gráfico global do OLED e atualiza o display.
 * O buffer e a área são os globais definidos em estado_compartilhado.
 */
void oled_clear_global_buffer();

/**
 * @brief Exibe uma mensagem no display OLED por um tempo e depois limpa.
 * Usa o buffer e área globais.
 *
 * @param mensagem Texto UTF-8 a ser exibido.
 * @param linha_y Posição vertical (em pixels) para iniciar a mensagem.
 */
void oled_exibir_mensagem_temporaria(const char *mensagem, int linha_y);

/**
 * @brief Renderiza o conteúdo do buffer global na área global do display.
 */
void oled_render_global_buffer();

#endif