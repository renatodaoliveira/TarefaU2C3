/**
 * @file main_core0_utils.c
 * @brief Funções utilitárias para o Núcleo 0.
 * Inclui tratamento de mensagens da FIFO, exibição no OLED e controle do LED RGB.
 */

#include "core0/main_core0_utils.h"
#include "config/config_geral.h"
#include "shared/estado_compartilhado.h"
#include "drivers/rgb_led/rgb_led_pwm.h"
#include "drivers/oled_ssd1306/oled_interface.h" 
#include "drivers/oled_ssd1306/oled_driver.h"    
#include <stdio.h> 
#include <stdlib.h> // Para rand() 
#include "lwip/ip_addr.h" // Para ip4addr_ntoa_r

/**
 * @brief Aguarda até que a conexão USB (console serial) esteja pronta.
 */
void util_espera_usb_serial() {
    while (!stdio_usb_connected()) {
        sleep_ms(200); // Aguarda um pouco para não sobrecarregar
    }
    printf("Conexão USB (Serial) estabelecida!\n");
}

/**
 * @brief Trata uma mensagem recebida do núcleo 1.
 */
void util_tratar_mensagem_intercore(MensagemInterCore msg) {
    const char *descricao_status_wifi = "";
    char linha_oled[40]; 

    // Verifica se é uma confirmação de publicação MQTT (PING ACK)
    if (msg.tentativa_ou_tipo == FIFO_TIPO_MQTT_PUB_ACK) {
        oled_clear_global_buffer(); 
        // Re-exibe IP se já tivermos, para não ser apagado pela msg de ACK PING
        if (ultimo_ip_bin != 0) {
            char ip_str_temp[20];
            char linha_oled_txt[20];
            char linha_oled_ip[30];
            ip4addr_ntoa_r((const ip4_addr_t*)&ultimo_ip_bin, ip_str_temp, sizeof(ip_str_temp));
            snprintf(linha_oled_ip, sizeof(linha_oled_txt), "IP Broker:", ip_str_temp);
            snprintf(linha_oled_ip, sizeof(linha_oled_ip), "%s", ip_str_temp);
            ssd1306_draw_utf8_string(buffer_oled, 0, 16, linha_oled_ip); // Exibe na segunda linha
        }
        
        if (msg.status_ou_dado == 0) { // 0 = Sucesso
            snprintf(linha_oled, sizeof(linha_oled), "ACK do PING: OK");
            ssd1306_draw_utf8_string(buffer_oled, 0, 32, linha_oled); // Posição para status do PING
            
            // --- GERAR E APLICAR COR ALEATÓRIA ---
            uint16_t r_aleatorio = rand() % (PWM_STEP + 1); // Gera valor entre 0 e PWM_STEP
            uint16_t g_aleatorio = rand() % (PWM_STEP + 1);
            uint16_t b_aleatorio = rand() % (PWM_STEP + 1);

            // Opcional: garantir que a cor não seja muito escura/apagada
            if (r_aleatorio < (PWM_STEP / 4) && g_aleatorio < (PWM_STEP / 4) && b_aleatorio < (PWM_STEP / 4)) {
                int canal_brilhante = rand() % 3;
                if (canal_brilhante == 0) r_aleatorio = (PWM_STEP / 2) + (rand() % (PWM_STEP / 2));
                else if (canal_brilhante == 1) g_aleatorio = (PWM_STEP / 2) + (rand() % (PWM_STEP / 2));
                else b_aleatorio = (PWM_STEP / 2) + (rand() % (PWM_STEP / 2));

                if (r_aleatorio > PWM_STEP) r_aleatorio = PWM_STEP;
                if (g_aleatorio > PWM_STEP) g_aleatorio = PWM_STEP;
                if (b_aleatorio > PWM_STEP) b_aleatorio = PWM_STEP;
            }
            
            printf("[CORE0] ACK PING OK. Nova cor RGB: R=%u, G=%u, B=%u\n", r_aleatorio, g_aleatorio, b_aleatorio);
            set_rgb_pwm(r_aleatorio, g_aleatorio, b_aleatorio); 
            // --- FIM DA LÓGICA DE COR ALEATÓRIA ---

        } else { // Outro valor = Falha
            snprintf(linha_oled, sizeof(linha_oled), "ACK do PING: FALHOU");
            ssd1306_draw_utf8_string(buffer_oled, 0, 32, linha_oled);
            set_rgb_pwm(PWM_STEP, 0, 0); // LED Vermelho para ACK Falha
        }
        oled_render_global_buffer();
        return; // Importante para não sobrescrever a cor aleatória com a cor do status do WiFi
    }

    // Se não for um ACK de PING, é um status da conexão Wi-Fi
    switch (msg.status_ou_dado) {
        case 0: // CYW43_LINK_DOWN ou inicializando
            descricao_status_wifi = "WiFi: Tentando...";
            set_rgb_pwm(PWM_STEP, PWM_STEP, 0); // LED Amarelo (Vermelho + Verde)
            break;
        case 1: // CYW43_LINK_UP (Conectado)
            descricao_status_wifi = "WiFi: Conectado";
            set_rgb_pwm(0, PWM_STEP, 0);      // LED Verde
            break;
        case 2: // CYW43_LINK_FAIL, CYW43_LINK_NONET, CYW43_LINK_BADAUTH
            descricao_status_wifi = "WiFi: Falha";
            set_rgb_pwm(PWM_STEP, 0, 0);      // LED Vermelho
            break;
        case 3: // CYW43_LINK_CONNECTING (Status intermediário do cyw43)
            descricao_status_wifi = "WiFi: Conectando";
            set_rgb_pwm(0, 0, PWM_STEP);      // LED Azul
            break;
        default:
            descricao_status_wifi = "WiFi: Desconhecido";
            set_rgb_pwm(PWM_STEP, PWM_STEP, PWM_STEP); // LED Branco
            break;
    }

    oled_clear_global_buffer();
    // Re-exibe IP se já tivermos, para não ser apagado pela msg de status Wi-Fi
    if (ultimo_ip_bin != 0) {
        char ip_str_temp[20];
        char linha_oled_txt[20];
        char linha_oled_ip[30];
        ip4addr_ntoa_r((const ip4_addr_t*)&ultimo_ip_bin, ip_str_temp, sizeof(ip_str_temp));
        snprintf(linha_oled_ip, sizeof(linha_oled_txt), "IP Broker:", ip_str_temp);
        ssd1306_draw_utf8_string(buffer_oled, 0, 16, linha_oled_ip); // Exibe na segunda linha
    }

    if (msg.tentativa_ou_tipo > 0 && msg.status_ou_dado != 1 /* não conectado */) {
         snprintf(linha_oled, sizeof(linha_oled), "%s (T%d)", descricao_status_wifi, msg.tentativa_ou_tipo);
    } else {
         snprintf(linha_oled, sizeof(linha_oled), "%s", descricao_status_wifi);
    }
    ssd1306_draw_utf8_string(buffer_oled, 0, 0, linha_oled); // Status Wi-Fi na primeira linha
    oled_render_global_buffer();
    
    printf("[CORE0] Status Wi-Fi: %s (Tentativa: %u)\n", descricao_status_wifi, msg.tentativa_ou_tipo);
}

/**
 * @brief Trata um endereço IP binário recebido.
 */
void util_tratar_ip_recebido(uint32_t ip_bin) {
    char ip_str[20];
    // Converte o IP binário para string (formato X.X.X.X)
    ip4addr_ntoa_r((const ip4_addr_t*)&ip_bin, ip_str, sizeof(ip_str));

    oled_clear_global_buffer();
    char linha_oled[30];
    snprintf(linha_oled, sizeof(linha_oled), "IP Broker: \n%s", ip_str);
    ssd1306_draw_utf8_string(buffer_oled, 0, 16, linha_oled); // Exibe na segunda linha
    oled_render_global_buffer();

    printf("[CORE0] Endereço IP recebido: %s\n", ip_str);
    ultimo_ip_bin = ip_bin; // Armazena o IP no estado compartilhado
}

/**
 * @brief Exibe uma mensagem de status relacionada ao MQTT no display OLED.
 */
void util_exibir_status_mqtt_oled(const char *texto) {
    // Não limpa o buffer aqui para poder adicionar a outras informações (ex: IP)
    char linha_oled[40];

    // Re-exibe IP se já tivermos, caso esta função seja chamada e apague
    if (ultimo_ip_bin != 0) {
        char ip_str_temp[20];
        char linha_oled_txt[20];
        char linha_oled_ip[30];
        ip4addr_ntoa_r((const ip4_addr_t*)&ultimo_ip_bin, ip_str_temp, sizeof(ip_str_temp));
        snprintf(linha_oled_ip, sizeof(linha_oled_txt), "IP Broker:", ip_str_temp);
        snprintf(linha_oled_ip, sizeof(linha_oled_ip), "%s", ip_str_temp);
        ssd1306_draw_utf8_string(buffer_oled, 0, 16, linha_oled_ip);
    }

    snprintf(linha_oled, sizeof(linha_oled), "MQTT: %s", texto);
    // Exibe na terceira linha do OLED (0, 16, 32...)
    ssd1306_draw_utf8_string(buffer_oled, 0, 32, linha_oled);
    oled_render_global_buffer();

    printf("[CORE0] Status MQTT: %s\n", texto);
}