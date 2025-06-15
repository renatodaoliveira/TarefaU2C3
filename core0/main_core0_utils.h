#ifndef MAIN_CORE0_UTILS_H
#define MAIN_CORE0_UTILS_H

#include <stdint.h>
#include "core0/fila_circular.h" // Para MensagemInterCore

// Constantes para identificar tipos de mensagem na FIFO
#define FIFO_TIPO_IP_ADDRESS 0xFFFE // Indica que o payload é um endereço IP
#define FIFO_TIPO_MQTT_PUB_ACK 0x9999 // Indica que é um ACK de publicação MQTT

/**
 * @brief Aguarda até que a conexão USB (console serial) esteja pronta.
 */
void util_espera_usb_serial();

/**
 * @brief Trata uma mensagem recebida do núcleo 1 (via fila).
 * Interpreta o status do Wi-Fi ou o ACK de uma publicação MQTT.
 * Atualiza o LED RGB e o display OLED.
 * @param msg A mensagem recebida.
 */
void util_tratar_mensagem_intercore(MensagemInterCore msg);

/**
 * @brief Trata um endereço IP binário recebido do núcleo 1.
 * Converte para string, exibe no OLED e armazena no estado compartilhado.
 * @param ip_bin Endereço IP em formato binário (uint32_t).
 */
void util_tratar_ip_recebido(uint32_t ip_bin);

/**
 * @brief Exibe uma mensagem de status relacionada ao MQTT no display OLED.
 * Usado para feedback visual de operações MQTT (conexão, publicação).
 * @param texto Mensagem de status a ser exibida.
 */
void util_exibir_status_mqtt_oled(const char *texto);

#endif