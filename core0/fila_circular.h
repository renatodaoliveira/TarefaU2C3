#ifndef FILA_CIRCULAR_H
#define FILA_CIRCULAR_H

#include "config/config_geral.h" // Para TAM_FILA, tipos e mutex
#include <stdbool.h>
#include <stdint.h>
#include "pico/mutex.h"

// Estrutura para mensagens trocadas via FIFO, relacionadas ao status do Wi-Fi/MQTT
typedef struct {
    uint16_t tentativa_ou_tipo; // Pode ser número da tentativa ou um tipo especial de msg
    uint16_t status_ou_dado;    // Status da conexão/operação ou dado adicional
} MensagemInterCore;

// Estrutura da fila circular
typedef struct {
    MensagemInterCore fila[TAM_FILA];
    int frente;
    int tras;
    int tamanho;
    mutex_t mutex_fila; // Renomeado para evitar conflito com outros mutexes
} FilaCircularInterCore;

void fila_intercore_inicializar(FilaCircularInterCore *f);
bool fila_intercore_inserir(FilaCircularInterCore *f, MensagemInterCore m);
bool fila_intercore_remover(FilaCircularInterCore *f, MensagemInterCore *saida);
bool fila_intercore_vazia(FilaCircularInterCore *f);

#endif