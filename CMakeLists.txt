# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Habilita a exportação de comandos de compilação (útil para linters e IDEs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define a placa alvo
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK do Raspberry Pi Pico
# Certifique-se de que PICO_SDK_PATH está definido no seu ambiente
# ou que o SDK está em um local padrão.
include(pico_sdk_import.cmake)

# Define o nome do projeto e as linguagens
project(MQTTPicoRF C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Adiciona o executável principal do projeto
add_executable(MQTTPicoRF
    # Fontes do Núcleo 0
    core0/main_core0.c
    core0/main_core0_utils.c
    core0/fila_circular.c

    # Fontes do Núcleo 1
    core1/main_core1.c
    core1/mqtt_client_core1.c

    # Drivers
    drivers/rgb_led/rgb_led_pwm.c
    drivers/oled_ssd1306/oled_driver.c
    drivers/oled_ssd1306/oled_interface.c

    # Código Compartilhado
    shared/estado_compartilhado.c
)

# Habilita saída serial via USB (1) e/ou UART (0)
pico_enable_stdio_usb(MQTTPicoRF 1)  # USB CDC habilitado
pico_enable_stdio_uart(MQTTPicoRF 0) # UART desabilitado (GP0/GP1)

# Vincula as bibliotecas necessárias do SDK do Pico
target_link_libraries(MQTTPicoRF PRIVATE
    pico_stdlib   
)

# Adiciona os diretórios de include ao projeto
target_include_directories(MQTTPicoRF PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/core0
    ${CMAKE_CURRENT_LIST_DIR}/core1
    ${CMAKE_CURRENT_LIST_DIR}/drivers/rgb_led
    ${CMAKE_CURRENT_LIST_DIR}/drivers/oled_ssd1306
    ${CMAKE_CURRENT_LIST_DIR}/shared
)

# Vincula as bibliotecas necessárias do SDK do Pico
target_link_libraries(MQTTPicoRF PRIVATE
    pico_multicore                  # Suporte a multicore
    pico_sync                       # Primitivas de sincronização (mutex)
    hardware_pwm                    # Controle de PWM
    hardware_i2c                    # Comunicação I2C
    pico_cyw43_arch_lwip_threadsafe_background # Arquitetura Wi-Fi com lwIP thread-safe
    pico_lwip_mqtt                  # Cliente MQTT para lwIP
)

# Gera arquivos adicionais de saída (UF2, ELF, etc.)
pico_add_extra_outputs(MQTTPicoRF)