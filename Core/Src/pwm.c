#include "main.h"
#include "stm32f4xx_hal.h"
#include "pwm.h"

static TIM_HandleTypeDef htim3;
static float s_pan_deg = 90.0f, s_tilt_deg = 90.0f;

static float pulse_to_duty_pct(uint16_t pulse_us)
{
  return (float)pulse_us * 100.0f / 20000.0f;
}

void PWM_Init(void)
{
  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = SERVO_PAN_PIN | SERVO_TILT_PIN;
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(SERVO_PORT, &g);

  /* 84MHz / 84 = 1MHz 计数，数到 20000 即 50Hz；CCR 直接以微秒为单位 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84 - 1;
  htim3.Init.Period = 20000 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&htim3);

  TIM_OC_InitTypeDef oc = {0};
  oc.OCMode = TIM_OCMODE_PWM1;
  oc.Pulse = 1500;                     /* 上电先到 90 度中位 */
  oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  oc.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &oc, SERVO_PAN_CH);
  HAL_TIM_PWM_ConfigChannel(&htim3, &oc, SERVO_TILT_CH);

  HAL_TIM_PWM_Start(&htim3, SERVO_PAN_CH);
  HAL_TIM_PWM_Start(&htim3, SERVO_TILT_CH);
}

static uint16_t angle_to_pulse(uint8_t deg)
{
  if (deg > 180) deg = 180;
  return 500 + (uint32_t)deg * 2000 / 180;   /* 500us=0度, 2500us=180度 */
}

void Servo_SetAngle(uint8_t pan, uint8_t tilt)
{
  s_pan_deg  = pan;
  s_tilt_deg = tilt;
  __HAL_TIM_SET_COMPARE(&htim3, SERVO_PAN_CH,  angle_to_pulse(pan));
  __HAL_TIM_SET_COMPARE(&htim3, SERVO_TILT_CH, angle_to_pulse(tilt));
}

void Servo_GetState(float *pan_deg, float *pan_duty_pct,
                    float *tilt_deg, float *tilt_duty_pct)
{
  *pan_deg  = s_pan_deg;
  *tilt_deg = s_tilt_deg;
  *pan_duty_pct = pulse_to_duty_pct(angle_to_pulse((uint8_t)s_pan_deg));
  *tilt_duty_pct = pulse_to_duty_pct(angle_to_pulse((uint8_t)s_tilt_deg));
}
