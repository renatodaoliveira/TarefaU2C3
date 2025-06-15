/**
 * @file oled_interface.c
 * @brief Funções de interface de alto nível para o display OLED.
 * Combina funcionalidades de oled_utils.c, display.c, setup_oled.c.
 */

#include "drivers/oled_ssd1306/oled_interface.h"
#include "drivers/oled_ssd1306/oled_driver.h" // Para ssd1306_init, render, etc.
#include "config/config_geral.h"       // Para pinos I2C, tempos
#include "shared/estado_compartilhado.h" // Para buffer_oled, area globais
#include "hardware/i2c.h"
#include "pico/stdlib.h" // Para gpio_set_function, gpio_pull_up
#include <string.h>      // Para memset

// O i2c_inst_t usado é i2c1
#define I2C_PORT i2c1

/**
 * @brief Inicializa o barramento I2C e o display OLED.
 */
void oled_setup_interface() {
    // Inicializa o barramento I2C na porta i2c1 com frequência de 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);

    // Define os pinos SDA e SCL como função I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    // Ativa resistores de pull-up nos pinos I2C
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Inicializa o display OLED com o controlador SSD1306
    ssd1306_init();

    // Define a área de renderização global para cobrir toda a tela
    area.start_column = 0;
    area.end_column = SSD1306_WIDTH - 1;
    area.start_page = 0;
    area.end_page = ssd1306_n_pages - 1;

    // Calcula o tamanho do buffer necessário para essa área
    calculate_render_area_buffer_length(&area); // Passa o ponteiro da 'area' global

    // Limpa completamente o display usando o buffer global
    oled_clear_global_buffer();
}

/**
 * @brief Limpa o buffer gráfico global do OLED e atualiza o display.
 */
void oled_clear_global_buffer() {
    // Preenche o buffer global com zeros (todos os pixels apagados)
    memset(buffer_oled, 0, ssd1306_buffer_length);

    // Envia o buffer global atualizado para o display, usando a 'area' global
    ssd1306_render(buffer_oled, &area);
}

/**
 * @brief Exibe uma mensagem no display OLED por um tempo e depois limpa.
 */
void oled_exibir_mensagem_temporaria(const char *mensagem, int linha_y) {
    oled_clear_global_buffer(); // Limpa usando o buffer global

    // Escreve o texto no buffer global
    ssd1306_draw_utf8_multiline(buffer_oled, 0, linha_y, mensagem);

    // Renderiza o buffer global
    oled_render_global_buffer();

    // Espera TEMPO_MENSAGEM milissegundos
    sleep_ms(TEMPO_MENSAGEM);

    // Limpa novamente o conteúdo
    oled_clear_global_buffer();
    oled_render_global_buffer();
}

/**
 * @brief Renderiza o conteúdo do buffer global na área global do display.
 */
void oled_render_global_buffer() {
    ssd1306_render(buffer_oled, &area);
}