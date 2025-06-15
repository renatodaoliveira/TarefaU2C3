#ifndef CONFIG_GERAL_H
#define CONFIG_GERAL_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "pico/mutex.h"

// --- ATENÇÃO: PINOS ATUALIZADOS CONFORME O DOCUMENTO "Organização dos Pinos da BitDogLab" ---
//LED RGB
#define LED_R 13 // Vermelho: GPIO13
#define LED_G 11 // Verde: GPIO11
#define LED_B 12 // Azul: GPIO12
#define PWM_STEP 0xFFFF // (1 << 16) - 1, para ciclo de trabalho máximo em PWM de 16 bits

//Pinos I2C para OLED
#define SDA_PIN 14 // SDA: GPIO14
#define SCL_PIN 15 // SCL: GPIO15

// Tempos e tamanhos
#define TEMPO_CONEXAO 2000      // ms para timeouts de conexão Wi-Fi
#define TEMPO_MENSAGEM 2000     // ms para exibição de mensagens temporárias no OLED
#define TAM_FILA 16             // Tamanho da fila circular para mensagens do Wi-Fi
#define INTERVALO_PING_MS 5000  // Intervalo entre envios de "PING" MQTT

// Configurações de Rede
#define WIFI_SSID "@"                           // SSID da sua Rede Wi-Fi
#define WIFI_PASS "internet"                    // Senha da sua Rede Wi-Fi
#define MQTT_BROKER_IP "192.168.246.110"        // Endereço IP do seu broker Mosquitto
#define MQTT_BROKER_PORT 1883                   // Porta padrão do MQTT
#define TOPICO "pico/PING"                      // Tópico MQTT para publicar o PING

// Para evitar redefinição de oled_utils.h em outros lugares
// Se oled_interface.h for incluído, estas funções estarão disponíveis.
// Caso contrário, declarações podem ser necessárias em outros módulos se não incluírem oled_interface.h
// No entanto, é melhor que os módulos que usam OLED incluam oled_interface.h

// Buffers globais e área de renderização para OLED (definidos em estado_compartilhado.c)
// Eles são declarados em estado_compartilhado.h
// extern uint8_t buffer_oled[];
// extern struct render_area area;

#endif