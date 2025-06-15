/**
 * @file main_core0.c
 * @brief Núcleo 0 - Controle principal do sistema: OLED, RGB, FIFO e MQTT.
 *
 * Responsabilidades:
 * - Inicializar hardware (OLED, RGB, USB Serial).
 * - Lançar o Núcleo 1 para gerenciamento da conexão Wi-Fi.
 * - Receber mensagens do Núcleo 1 via FIFO (status Wi-Fi, IP, ACK MQTT).
 * - Processar e exibir essas mensagens no OLED e controlar LED RGB.
 * - Iniciar o cliente MQTT após receber um IP válido.
 * - Enviar periodicamente mensagens "PING" via MQTT.
 */

#include "config/config_geral.h"
#include "shared/estado_compartilhado.h"
#include "core0/fila_circular.h"
#include "core0/main_core0_utils.h" // Para util_tratar_mensagem_intercore, etc.
#include "drivers/rgb_led/rgb_led_pwm.h"
#include "drivers/oled_ssd1306/oled_interface.h" // Para oled_setup_interface, etc.
#include "drivers/oled_ssd1306/oled_driver.h" // para ssd1306_draw_utf8_string especificamente
#include "core1/main_core1.h"           // Para declaração de main_core1_entry
#include "core1/mqtt_client_core1.h"    // Para iniciar_cliente_mqtt, publicar_mensagem_mqtt
#include "pico/multicore.h"
#include "lwip/ip_addr.h" // Para ip4_addr_t (usado em tratar_ip_recebido)

// --- NOVAS INCLUSÕES ---
#include <stdlib.h>      // Para rand() e srand()
#include "pico/rand.h"   // Para get_rand_32(), uma boa semente no Pico
// --- FIM DAS NOVAS INCLUSÕES ---


// Fila para mensagens recebidas do núcleo 1
static FilaCircularInterCore fila_mensagens_core1;
static absolute_time_t proximo_envio_ping;

// Protótipos de funções locais
static void inicializar_perifericos_core0();
static void iniciar_nucleo1();
static void verificar_fifo_do_core1();
static void processar_fila_mensagens();
static void tentar_inicializar_mqtt();
static void enviar_ping_mqtt_periodicamente();

int main() {
    inicializar_perifericos_core0();
    iniciar_nucleo1();

    oled_exibir_mensagem_temporaria("Sistema Ativado!\nAguardando WiFi...", 0);

    while (true) {
        verificar_fifo_do_core1();
        processar_fila_mensagens();
        tentar_inicializar_mqtt();
        enviar_ping_mqtt_periodicamente();
        sleep_ms(50); // Pequena pausa para não sobrecarregar o loop
    }
    return 0; // Nunca alcançado
}

/**
 * @brief Inicializa os periféricos controlados pelo Núcleo 0.
 */
static void inicializar_perifericos_core0() {
    stdio_init_all(); // Inicializa stdio (USB e/ou UART)
    util_espera_usb_serial(); // Aguarda conexão serial USB

    oled_setup_interface(); // Configura I2C e OLED
    init_rgb_pwm();         // Configura PWM para o LED RGB
    set_rgb_pwm(PWM_STEP, 0, PWM_STEP); // LED Roxo indicando inicialização

    fila_intercore_inicializar(&fila_mensagens_core1);

    // --- SEMEAR O GERADOR DE NÚMEROS ALEATÓRIOS ---
    srand(get_rand_32()); // Usa o gerador de hardware do RP2040 como semente
    // --- FIM DA SEMEADURA ---

    printf("Núcleo 0: Periféricos inicializados.\n");
    oled_clear_global_buffer();
    ssd1306_draw_utf8_string(buffer_oled, 0, 0, "Core0: OK");
    oled_render_global_buffer();
    sleep_ms(1000);
}

/**
 * @brief Lança a execução do código do Núcleo 1.
 */
static void iniciar_nucleo1() {
    printf("Núcleo 0: Lançando Núcleo 1...\n");
    multicore_launch_core1(main_core1_entry); // main_core1_entry é a função principal do core1
    oled_clear_global_buffer();
    ssd1306_draw_utf8_string(buffer_oled, 0, 0, "Core0: OK");
    ssd1306_draw_utf8_string(buffer_oled, 0, 16, "Core1: Lancado");
    oled_render_global_buffer();
    sleep_ms(1000);
}

/**
 * @brief Verifica se há dados na FIFO enviados pelo Núcleo 1 e os processa.
 */
static void verificar_fifo_do_core1() {
    if (multicore_fifo_rvalid()) { // Há dados para ler?
        uint32_t pacote_fifo = multicore_fifo_pop_blocking();
        
        // O pacote superior (16 bits) indica o tipo ou tentativa
        uint16_t tipo_ou_tentativa = (pacote_fifo >> 16) & 0xFFFF;
        
        if (tipo_ou_tentativa == FIFO_TIPO_IP_ADDRESS) {
            // Se é um IP, o próximo item na FIFO é o próprio IP
            uint32_t ip_bin = multicore_fifo_pop_blocking();
            util_tratar_ip_recebido(ip_bin);
        } else {
            // Caso contrário, é uma mensagem de status (Wi-Fi ou MQTT ACK)
            MensagemInterCore msg;
            msg.tentativa_ou_tipo = tipo_ou_tentativa;
            msg.status_ou_dado = pacote_fifo & 0xFFFF; // Os 16 bits inferiores são o status

            if (!fila_intercore_inserir(&fila_mensagens_core1, msg)) {
                printf("[CORE0] ERRO: Fila de mensagens do Core1 cheia! Mensagem descartada.\n");
                oled_exibir_mensagem_temporaria("Core0: Fila FIFO cheia!", 0);
            }
        }
    }
}

/**
 * @brief Processa mensagens da fila interna que foram recebidas do Núcleo 1.
 */
static void processar_fila_mensagens() {
    MensagemInterCore msg_recebida;
    if (fila_intercore_remover(&fila_mensagens_core1, &msg_recebida)) {
        util_tratar_mensagem_intercore(msg_recebida);
    }
}

/**
 * @brief Tenta inicializar o cliente MQTT se ainda não foi feito e um IP já foi obtido.
 */
static void tentar_inicializar_mqtt() {
    if (!mqtt_iniciado && ultimo_ip_bin != 0) {
        printf("[CORE0] IP recebido. Iniciando cliente MQTT...\n");
        util_exibir_status_mqtt_oled("Iniciando..."); // Mostra no OLED
        iniciar_cliente_mqtt(); // Função do módulo mqtt_client_core1.c
        mqtt_iniciado = true;   // Marca como iniciado (variável de estado_compartilhado.c)
        proximo_envio_ping = make_timeout_time_ms(INTERVALO_PING_MS); // Prepara para o primeiro PING
    }
}

/**
 * @brief Envia uma mensagem "PING" via MQTT em intervalos regulares.
 */
static void enviar_ping_mqtt_periodicamente() {
    if (mqtt_iniciado && absolute_time_diff_us(get_absolute_time(), proximo_envio_ping) <= 0) {
        printf("[CORE0] Enviando PING MQTT...\n");
        
        oled_clear_global_buffer(); // Limpa para nova mensagem
        // Re-exibe IP se já tivermos
        if (ultimo_ip_bin != 0) {
            char ip_str_temp[20];
            char linha_oled_ip[30];
            ip4addr_ntoa_r((const ip4_addr_t*)&ultimo_ip_bin, ip_str_temp, sizeof(ip_str_temp));
            snprintf(linha_oled_ip, sizeof(linha_oled_ip), "%s", ip_str_temp);
            ssd1306_draw_utf8_string(buffer_oled, 0, 16, linha_oled_ip);
        }
        // Exibe status do MQTT
        char linha_oled_mqtt_status[40];
        snprintf(linha_oled_mqtt_status, sizeof(linha_oled_mqtt_status), "\nMQTT: Ping...");
        ssd1306_draw_utf8_string(buffer_oled, 0, 32, linha_oled_mqtt_status);
        oled_render_global_buffer();

        publicar_mensagem_mqtt("PING"); // Função do módulo mqtt_client_core1.c
        
        proximo_envio_ping = make_timeout_time_ms(INTERVALO_PING_MS); // Agenda o próximo PING
    }
}