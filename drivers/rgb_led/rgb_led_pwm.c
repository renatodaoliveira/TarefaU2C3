#include "drivers/rgb_led/rgb_led_pwm.h"
#include "hardware/pwm.h" // Para funções pwm_*
#include "pico/stdlib.h"  // Para gpio_set_function

// Variáveis estáticas para armazenar os números dos slices de PWM
static uint slice_r, slice_g, slice_b;

/**
 * @brief Inicializa os pinos GPIO conectados ao LED RGB para operarem com PWM.
 */
void init_rgb_pwm() {
    // Configura os pinos GPIO para a função PWM
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    // Obtém o número do slice de PWM para cada pino
    slice_r = pwm_gpio_to_slice_num(LED_R);
    slice_g = pwm_gpio_to_slice_num(LED_G);
    slice_b = pwm_gpio_to_slice_num(LED_B);

    // Obtém a configuração padrão do PWM
    pwm_config config = pwm_get_default_config();
    // Define um divisor de clock para o PWM (ajuste conforme necessário para a frequência desejada)
    // Um divisor de 4.f com clock de sistema de 125MHz resulta em ~31.25MHz para o contador PWM.
    // Com um TOP de 0xFFFF (65535), a frequência do PWM será ~476Hz.
    pwm_config_set_clkdiv(&config, 4.f);
    // Para PWM_STEP (0xFFFF), o wrap é automático.

    // Inicializa cada slice de PWM com a configuração e habilita-o
    pwm_init(slice_r, &config, true);
    pwm_init(slice_g, &config, true);
    pwm_init(slice_b, &config, true);
}

/**
 * @brief Define os níveis de duty cycle para cada cor do LED RGB.
 */
void set_rgb_pwm(uint16_t r_val, uint16_t g_val, uint16_t b_val) {
    // Define o nível (duty cycle) para cada pino/cor
    // O segundo argumento é o valor do contador onde o PWM deve ir para nível baixo.
    // Como estamos usando 0xFFFF como PWM_STEP (wrap implícito), estes valores
    // representam diretamente o duty cycle.
    pwm_set_gpio_level(LED_R, r_val);
    pwm_set_gpio_level(LED_G, g_val);
    pwm_set_gpio_level(LED_B, b_val);
}