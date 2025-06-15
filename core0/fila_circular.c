/**
 * @file fila_circular.c
 * @brief Implementação da fila circular com proteção por mutex para mensagens inter-core.
 */

#include "core0/fila_circular.h"

void fila_intercore_inicializar(FilaCircularInterCore *f) {
    f->frente = 0;
    f->tras = -1;
    f->tamanho = 0;
    mutex_init(&f->mutex_fila);
}

bool fila_intercore_inserir(FilaCircularInterCore *f, MensagemInterCore m) {
    bool sucesso = false;
    mutex_enter_blocking(&f->mutex_fila);

    if (f->tamanho < TAM_FILA) {
        f->tras = (f->tras + 1) % TAM_FILA;
        f->fila[f->tras] = m;
        f->tamanho++;
        sucesso = true;
    }

    mutex_exit(&f->mutex_fila);
    return sucesso;
}

bool fila_intercore_remover(FilaCircularInterCore *f, MensagemInterCore *saida) {
    bool sucesso = false;
    mutex_enter_blocking(&f->mutex_fila);

    if (f->tamanho > 0) {
        *saida = f->fila[f->frente];
        f->frente = (f->frente + 1) % TAM_FILA;
        f->tamanho--;
        sucesso = true;
    }

    mutex_exit(&f->mutex_fila);
    return sucesso;
}

bool fila_intercore_vazia(FilaCircularInterCore *f) {
    mutex_enter_blocking(&f->mutex_fila); // Proteger leitura do tamanho
    bool vazia = (f->tamanho == 0);
    mutex_exit(&f->mutex_fila);
    return vazia;
}