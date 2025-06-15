#ifndef MQTT_CLIENT_CORE1_H
#define MQTT_CLIENT_CORE1_H

/**
 * @brief Inicializa e conecta o cliente MQTT ao broker.
 * As configurações do broker (IP, porta) são obtidas de `config_geral.h`.
 * Esta função é chamada pelo Núcleo 0 após a obtenção de um IP válido.
 * As operações de rede MQTT ocorrem no contexto da pilha lwIP (gerenciada pelo Núcleo 1).
 */
void iniciar_cliente_mqtt(void);

/**
 * @brief Publica uma mensagem no tópico MQTT pré-definido.
 * O tópico é definido em `config_geral.h`.
 * Chamada pelo Núcleo 0.
 *
 * @param mensagem A string da mensagem a ser publicada.
 */
void publicar_mensagem_mqtt(const char *mensagem);

/**
 * @brief Loop de manutenção do MQTT (atualmente não implementado).
 * Poderia ser usado para tarefas como manter a conexão viva (keep-alive)
 * se não for gerenciado automaticamente pela biblioteca MQTT.
 */
void loop_mqtt(); // Renomeado para evitar conflito com 'mqtt_loop' de bibliotecas

#endif