#ifndef MQTT_PLATFORM_H
#define MQTT_PLATFORM_H

// Este arquivo é específico para a biblioteca MQTT Paho ou similar,
// se estiver usando a API MQTT nativa do lwIP, ele pode não ser estritamente necessário
// ou pode precisar de adaptações dependendo da versão/implementação do MQTT.
// Para lwIP MQTT (lwip/apps/mqtt.h), geralmente não é necessário um MQTTPlatform.h complexo.

// No contexto do Pico SDK e lwIP, as inclusões de sockets e tipos são gerenciadas pelo lwIP.
// Se você estivesse portando uma biblioteca MQTT que espera um "platform header",
// você definiria aqui os tipos e funções de rede esperados.

// Para a API MQTT do lwIP (lwip/apps/mqtt.h), esta definição é importante se a biblioteca MQTT
// que você está usando (se for uma de terceiros e não a nativa do lwIP) procura por ela.
// Se estiver usando apenas lwip/apps/mqtt.h, este arquivo é mais um placeholder ou para
// configurações muito específicas da plataforma que a lib MQTT possa requerer.
// A linha abaixo é comum em exemplos que usam a lib Paho MQTT client para lwIP.
// Para o cliente MQTT integrado do lwIP, esta linha pode ser desnecessária ou precisar de ajuste.
// O projeto original usava `lwip/apps/mqtt.h`, então esta é provavelmente a intenção.

#include "lwip/sockets.h"  // Garante que as definições de socket do lwIP estão disponíveis

// Esta macro é usada por algumas bibliotecas MQTT para incluir este arquivo de plataforma.
// Verifique a documentação da biblioteca MQTT específica que você está usando.
// Para o cliente MQTT nativo do lwIP (lwip/apps/mqtt.h), isso é menos relevante.
#define MQTTCLIENT_PLATFORM_HEADER "config/MQTTPlatform.h" // Atualizado o caminho

#endif