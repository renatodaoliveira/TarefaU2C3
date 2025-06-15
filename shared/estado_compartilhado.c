/**
 * @file estado_compartilhado.c
 * @brief Definição de variáveis globais compartilhadas e estruturas do OLED.
 *
 * Este arquivo define variáveis globais utilizadas em múltiplos arquivos do sistema,
 * facilitando o compartilhamento de estado entre os núcleos e módulos.
 *
 * Ele define:
 * - O último endereço IP recebido (`ultimo_ip_bin`), utilizado para iniciar o cliente MQTT;
 * - Um flag (`mqtt_iniciado`) que garante que o cliente MQTT só será iniciado uma vez;
 * - Um buffer de vídeo (`buffer_oled`) para escrita no display OLED;
 * - A estrutura `area`, que define a região da tela sendo desenhada.
 */

#include "shared/estado_compartilhado.h"
#include "drivers/oled_ssd1306/oled_driver.h" // Para ssd1306_buffer_length

// ================================
// DEFINIÇÕES GLOBAIS ÚNICAS
// ================================

/**
 * @brief Endereço IP mais recente recebido via núcleo 1.
 * Usado pelo núcleo 0 para verificar se a rede já obteve um IP válido
 * e então iniciar o cliente MQTT.
 */
uint32_t ultimo_ip_bin = 0;

/**
 * @brief Flag de controle que indica se o cliente MQTT já foi iniciado.
 * Evita múltiplas tentativas de iniciar o cliente MQTT.
 */
bool mqtt_iniciado = false;

/**
 * @brief Buffer de vídeo para o display OLED.
 * Este buffer contém os dados de pixels que serão renderizados na tela.
 * Seu tamanho é definido por `ssd1306_buffer_length` do driver OLED.
 */
uint8_t buffer_oled[ssd1306_buffer_length]; // Usa a constante do driver

/**
 * @brief Estrutura que define a área da tela a ser desenhada.
 * Usada pelas funções de renderização para aplicar atualizações no OLED.
 */
struct render_area area;