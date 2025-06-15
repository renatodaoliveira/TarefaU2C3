/**
 * @file main_core1.c
 * @brief Lógica do Núcleo 1: Gerenciamento da conexão Wi-Fi.
 *
 * Este núcleo é responsável por:
 * - Inicializar o chip CYW43 (para Wi-Fi).
 * - Conectar-se à rede Wi-Fi especificada.
 * - Monitorar o status da conexão e tentar reconectar em caso de falha.
 * - Enviar o status da conexão e o endereço IP obtido para o Núcleo 0 via FIFO.
 */

#include "core1/main_core1.h"
#include "config/config_geral.h"
#include "core0/main_core0_utils.h" // Para FIFO_TIPO_IP_ADDRESS
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include <stdio.h>  // Para printf no Core 1 (debug)
#include <string.h> // Para memset

// Protótipos de funções locais
static bool verificar_conexao_wifi();
static void enviar_status_wifi_para_core0(uint16_t status_wifi, uint16_t tentativa);
static void enviar_ip_para_core0(const uint8_t *ip);
static void tentar_conectar_wifi();
static void monitorar_e_reconectar_wifi();


/**
 * @brief Ponto de entrada para o código do Núcleo 1.
 */
void main_core1_entry(void) {
    printf("[CORE1] Núcleo 1 iniciado.\n");

    if (cyw43_arch_init()) {
        printf("[CORE1] Falha ao inicializar CYW43.\n");
        enviar_status_wifi_para_core0(2, 0); // 2 = Falha
        return; // Não há muito o que fazer se o chip Wi-Fi falhar ao iniciar
    }
    printf("[CORE1] CYW43 inicializado.\n");

    tentar_conectar_wifi();
    monitorar_e_reconectar_wifi(); // Loop infinito de monitoramento
}

/**
 * @brief Verifica o status atual da conexão Wi-Fi.
 * @return true se conectado, false caso contrário.
 */
static bool verificar_conexao_wifi(void) {
    // CYW43_LINK_UP (3) indica conexão bem-sucedida
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

/**
 * @brief Envia o status da conexão Wi-Fi para o Núcleo 0 via FIFO.
 * @param status_wifi Status da conexão (0=DOWN, 1=UP, 2=FAIL, 3=CONNECTING).
 * @param tentativa Número da tentativa de conexão (0 se for um evento geral).
 */
static void enviar_status_wifi_para_core0(uint16_t status_wifi, uint16_t tentativa) {
    // Empacota tentativa e status em um único uint32_t
    // Tentativa nos 16 bits mais significativos, status nos 16 bits menos significativos
    uint32_t pacote_fifo = ((tentativa & 0xFFFF) << 16) | (status_wifi & 0xFFFF);
    multicore_fifo_push_blocking(pacote_fifo);
    printf("[CORE1] FIFO -> CORE0: Status WiFi=%u, Tentativa=%u\n", status_wifi, tentativa);
}

/**
 * @brief Envia o endereço IP obtido para o Núcleo 0 via FIFO.
 * @param ip Ponteiro para o array de 4 bytes do endereço IP.
 */
static void enviar_ip_para_core0(const uint8_t *ip) {
    // Primeiro, envia um pacote especial para indicar que o próximo dado é um IP
    uint32_t pacote_tipo_ip = (FIFO_TIPO_IP_ADDRESS << 16) | 0; // 0xFFFE nos bits superiores
    multicore_fifo_push_blocking(pacote_tipo_ip);

    // Em seguida, envia o IP como um uint32_t
    uint32_t ip_binario = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
    multicore_fifo_push_blocking(ip_binario);
    printf("[CORE1] FIFO -> CORE0: IP Enviado %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
}

/**
 * @brief Tenta conectar-se à rede Wi-Fi.
 */
static void tentar_conectar_wifi() {
    cyw43_arch_enable_sta_mode(); // Habilita modo Station (cliente)
    printf("[CORE1] Tentando conectar a: %s\n", WIFI_SSID);
    enviar_status_wifi_para_core0(3, 0); // 3 = Conectando

    for (uint16_t tentativa = 1; tentativa <= 5; tentativa++) { // Tenta 5 vezes
        printf("[CORE1] Tentativa de conexão Wi-Fi #%u...\n", tentativa);
        // Tenta conectar com timeout
        int resultado_conexao = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, TEMPO_CONEXAO * 5 // Timeout maior para primeira conexão
        );

        if (resultado_conexao == 0 && verificar_conexao_wifi()) {
            printf("[CORE1] Wi-Fi conectado com sucesso!\n");
            enviar_status_wifi_para_core0(1, tentativa); // 1 = Conectado (UP)
            
            // Envia o endereço IP para o Núcleo 0
            // O endereço IP está em cyw43_state.netif[0].ip_addr.addr (para IPv4)
            const uint8_t *ip_addr = (const uint8_t *)&(cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr);
            enviar_ip_para_core0(ip_addr);
            return; // Sai da função se conectado
        } else {
            printf("[CORE1] Falha na tentativa de conexão Wi-Fi #%u (Resultado: %d, Link Status: %d)\n",
                   tentativa, resultado_conexao, cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA));
            enviar_status_wifi_para_core0(2, tentativa); // 2 = Falha
            sleep_ms(TEMPO_CONEXAO); // Aguarda antes de tentar novamente
        }
    }
    printf("[CORE1] Falha ao conectar ao Wi-Fi após múltiplas tentativas.\n");
    enviar_status_wifi_para_core0(2, 0); // Falha final, sem tentativa específica
}

/**
 * @brief Monitora a conexão Wi-Fi e tenta reconectar se cair.
 */
static void monitorar_e_reconectar_wifi() {
    uint32_t contador_sem_conexao = 0;
    while (true) {
        if (!verificar_conexao_wifi()) {
            contador_sem_conexao++;
            printf("[CORE1] Conexão Wi-Fi perdida ou não estabelecida (Cont: %lu).\n", contador_sem_conexao);
            enviar_status_wifi_para_core0(0, 0); // 0 = Down (Perdida)
            
            // Tenta reconectar
            cyw43_arch_enable_sta_mode(); // Garante que o modo STA está ativo
            printf("[CORE1] Tentando reconectar ao Wi-Fi...\n");
            enviar_status_wifi_para_core0(3,0); // 3 = Conectando (para reconexão)

            for (uint16_t tentativa_reconexao = 1; tentativa_reconexao <= 3; tentativa_reconexao++) {
                 printf("[CORE1] Tentativa de reconexão Wi-Fi #%u...\n", tentativa_reconexao);
                int resultado_reconexao = cyw43_arch_wifi_connect_timeout_ms(
                    WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, TEMPO_CONEXAO
                );

                if (resultado_reconexao == 0 && verificar_conexao_wifi()) {
                    printf("[CORE1] Wi-Fi reconectado com sucesso!\n");
                    enviar_status_wifi_para_core0(1, tentativa_reconexao); // 1 = Conectado
                    const uint8_t *ip_addr = (const uint8_t *)&(cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr);
                    enviar_ip_para_core0(ip_addr);
                    contador_sem_conexao = 0; // Reseta contador
                    break; // Sai do loop de tentativas de reconexão
                } else {
                    printf("[CORE1] Falha na tentativa de reconexão #%u.\n", tentativa_reconexao);
                    enviar_status_wifi_para_core0(2, tentativa_reconexao); // 2 = Falha
                    sleep_ms(TEMPO_CONEXAO);
                }
            }
            if (!verificar_conexao_wifi()) {
                 printf("[CORE1] Falha ao reconectar após tentativas.\n");
                 enviar_status_wifi_para_core0(2, 0); // Falha final de reconexão
            }
        } else {
            contador_sem_conexao = 0; // Reseta se estiver conectado
        }
        sleep_ms(INTERVALO_PING_MS); // Verifica o status da conexão periodicamente (ex: a cada 5s)
    }
}