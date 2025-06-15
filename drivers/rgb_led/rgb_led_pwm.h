#ifndef RGB_LED_PWM_H
#define RGB_LED_PWM_H

#include "config/config_geral.h" // Para definições de pinos LED_R, LED_G, LED_B

/**
 * @brief Inicializa os pinos GPIO conectados ao LED RGB para operarem com PWM.
 * Configura os slices de PWM e a divisão de clock.
 */
void init_rgb_pwm();

/**
 * @brief Define os níveis de duty cycle para cada cor do LED RGB.
 *
 * @param r_val Valor do duty cycle para o LED vermelho (0 a 0xFFFF).
 * @param g_val Valor do duty cycle para o LED verde (0 a 0xFFFF).
 * @param b_val Valor do duty cycle para o LED azul (0 a 0xFFFF).
 */
void set_rgb_pwm(uint16_t r_val, uint16_t g_val, uint16_t b_val);

#endif