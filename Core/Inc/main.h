#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

/* USART1: PA9(TX)/PA10(RX) */
#define UART_BAUD            115200

/* CAN1: PB8(RX)/PB9(TX) - AF9（裸寄存器操作，不依赖 HAL CAN） */
#define CAN_RX_PIN           GPIO_PIN_8
#define CAN_TX_PIN           GPIO_PIN_9
#define CAN_PORT             GPIOB

/* TIM3: PA6(CH1)/PA7(CH2) - 50Hz PWM 舵机 */
#define SERVO_PAN_PIN        GPIO_PIN_6
#define SERVO_TILT_PIN       GPIO_PIN_7
#define SERVO_PORT           GPIOA
#define SERVO_TIM            TIM3
#define SERVO_PAN_CH         TIM_CHANNEL_1
#define SERVO_TILT_CH        TIM_CHANNEL_2

/* ADC1: PA0(IN0)/PA1(IN1) 电位器 */
#define POT_PAN_PIN          GPIO_PIN_0
#define POT_TILT_PIN         GPIO_PIN_1
#define POT_PORT             GPIOA

void Error_Handler(void);
void SystemClock_Config(void);

#endif