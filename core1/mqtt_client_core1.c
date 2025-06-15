/**
 * @file mqtt_client_core1.c
 * @brief Implementação do cliente MQTT utilizando a pilha lwIP.
 *
 * Gerencia a conexão com o broker MQTT e a publicação de mensagens.
 * As funções são chamadas pelo Núcleo 0, mas as operações de rede
 * são executadas no contexto da pilha lwIP (geralmente associada ao Núcleo 1
 * quando se usa `pico_cyw43_arch_lwip_threadsafe_background`).
 */

#include "core1/mqtt_client_core1.h"
#include "config/config_geral.h"
#include "core0/main_core0_utils.h" // Para FIFO_TIPO_MQTT_PUB_ACK e util_exibir_status_mqtt_oled
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "pico/multicore.h" // Para multicore_fifo_push_blocking
#include <stdio.h>
#include <string.h>


// Ponteiro para a instância do cliente MQTT
static mqtt_client_t *cliente_mqtt_inst;

// Informações de conexão do cliente MQTT
static struct mqtt_connect_client_info_t cliente_info_mqtt;

// Callbacks MQTT
static void mqtt_callback_conexao(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_callback_publicacao(void *arg, err_t result);
// static void mqtt_callback_dados_entrada(void *arg, const uint8_t *data, uint16_t len, uint8_t flags);
// static void mqtt_callback_inscricao(void *arg, err_t result);


/**
 * @brief Callback invocado após uma tentativa de conexão com o broker MQTT.
 */
static void mqtt_callback_conexao(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    LWIP_UNUSED_ARG(client);
    LWIP_UNUSED_ARG(arg);

    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conexão com broker ACEITA.\n");
        // A exibição no OLED é melhor controlada pelo Core 0.
        // O Core 0 chamará util_exibir_status_mqtt_oled("Conectado") se desejar.
        // Aqui, poderíamos enviar uma mensagem para o Core 0, mas o util_exibir_status_mqtt_oled
        // já é chamado pelo core0 ao iniciar o cliente.
        // Vamos apenas imprimir no console por enquanto.

        // Exemplo: inscrever-se em um tópico após conectar (se necessário)
        // mqtt_set_inpub_callback(client, mqtt_callback_dados_entrada, mqtt_callback_inscricao, arg);
        // mqtt_subscribe(client, "pico/comandos", 1, mqtt_callback_inscricao, arg);

    } else {
        printf("[MQTT] Falha na conexão com broker. Status: %d\n", status);
        // Similarmente, o Core 0 pode exibir "Falha MQTT"
    }
}

/**
 * @brief Callback invocado após uma tentativa de publicação de mensagem.
 */
static void mqtt_callback_publicacao(void *arg, err_t result) {
    LWIP_UNUSED_ARG(arg);
    uint16_t status_pub;

    if (result == ERR_OK) {
        printf("[MQTT] Publicação MQTT bem-sucedida.\n");
        status_pub = 0; // 0 para sucesso
    } else {
        printf("[MQTT] Falha na publicação MQTT. Erro: %d\n", result);
        status_pub = 1; // 1 para falha
    }

    // Envia o status da publicação (ACK do PING) de volta para o Núcleo 0
    // Usando FIFO_TIPO_MQTT_PUB_ACK para identificar esta mensagem
    uint32_t pacote_fifo = (FIFO_TIPO_MQTT_PUB_ACK << 16) | status_pub;
    multicore_fifo_push_blocking(pacote_fifo);
}

/**
 * @brief Inicializa e conecta o cliente MQTT.
 */
void iniciar_cliente_mqtt(void) {
    ip_addr_t ip_broker;

    // Converte o endereço IP do broker de string para o formato lwIP
    if (!ip4addr_aton(MQTT_BROKER_IP, &ip_broker)) {
        printf("[MQTT] Endereço IP do broker inválido: %s\n", MQTT_BROKER_IP);
        // O Core 0 já exibiu "Iniciando...", agora pode exibir falha
        // util_exibir_status_mqtt_oled("IP Broker Inv."); // Chamado pelo Core 0
        return;
    }

    // Cria uma nova instância do cliente MQTT
    cliente_mqtt_inst = mqtt_client_new();
    if (!cliente_mqtt_inst) {
        printf("[MQTT] Erro ao criar cliente MQTT.\n");
        // util_exibir_status_mqtt_oled("Erro Criacao"); // Chamado pelo Core 0
        return;
    }

    // Configura as informações do cliente
    memset(&cliente_info_mqtt, 0, sizeof(cliente_info_mqtt));
    cliente_info_mqtt.client_id = "rp2040_pico_w_client"; // ID do cliente
    // cliente_info_mqtt.client_user = "usuario"; // Se houver autenticação
    // cliente_info_mqtt.client_pass = "senha";   // Se houver autenticação
    // cliente_info_mqtt.keep_alive = 60; // Keep-alive em segundos

    // Tenta conectar ao broker MQTT
    // O callback mqtt_callback_conexao será chamado com o resultado.
    // O último argumento (NULL) é o 'arg' passado para os callbacks.
    err_t err = mqtt_client_connect(
        cliente_mqtt_inst,
        &ip_broker,
        MQTT_BROKER_PORT,
        mqtt_callback_conexao,
        NULL, // arg para callback
        &cliente_info_mqtt
    );

    if (err == ERR_OK) {
        printf("[MQTT] Tentativa de conexão MQTT iniciada...\n");
        // util_exibir_status_mqtt_oled("Conectando..."); // Chamado pelo Core 0
    } else {
        printf("[MQTT] Erro ao iniciar conexão MQTT: %d\n", err);
        // util_exibir_status_mqtt_oled("Erro Conexao"); // Chamado pelo Core 0
    }
}

/**
 * @brief Publica uma mensagem MQTT.
 */
void publicar_mensagem_mqtt(const char *mensagem) {
    if (!cliente_mqtt_inst || !mqtt_client_is_connected(cliente_mqtt_inst)) {
        printf("[MQTT] Não conectado. Não é possível publicar.\n");
        // util_exibir_status_mqtt_oled("Nao Conectado"); // Chamado pelo Core 0
        // Envia um ACK de falha para o Core0 para que ele saiba que o PING não foi
        uint32_t pacote_fifo = (FIFO_TIPO_MQTT_PUB_ACK << 16) | 1; // 1 = falha
        multicore_fifo_push_blocking(pacote_fifo);
        return;
    }

    err_t err = mqtt_publish(
        cliente_mqtt_inst,
        TOPICO,
        mensagem,
        strlen(mensagem),
        0, // QoS 0 (sem garantia de entrega)
        0, // Retain flag 0 (não reter a mensagem no broker)
        mqtt_callback_publicacao,
        NULL // arg para callback
    );

    if (err != ERR_OK) {
        printf("[MQTT] Erro ao tentar publicar mensagem: %d\n", err);
        // util_exibir_status_mqtt_oled("Erro Pub"); // Chamado pelo Core 0
        // O callback de publicação já envia o status para o Core0
    } else {
        printf("[MQTT] Mensagem '%s' enviada para publicação no tópico '%s'.\n", mensagem, TOPICO);
    }
}

/**
 * @brief Loop de manutenção do MQTT (não utilizado ativamente neste exemplo).
 */
void loop_mqtt() {
    // A biblioteca lwIP MQTT geralmente lida com keep-alives internamente
    // se configurado em cliente_info_mqtt.keep_alive.
    // Este loop poderia ser usado para processamento adicional se necessário.
}