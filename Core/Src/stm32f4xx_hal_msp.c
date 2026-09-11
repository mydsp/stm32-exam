#include "stm32f4xx_hal.h"
void HAL_MspInit(void) { __HAL_RCC_SYSCFG_CLK_ENABLE(); __HAL_RCC_PWR_CLK_ENABLE(); }
void HAL_UART_MspInit(UART_HandleTypeDef *huart) { }
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) { }