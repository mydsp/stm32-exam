#include "main.h"
#include "pwm.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;

static void UART1_Init(void);
void SystemClock_Config(void);   /* 原型已在 main.h，保持一致 */

/* JustFloat 接收：12 字节滑动窗口。
 * 帧内后 4 字节等于帧尾 00 00 80 7F 即整帧有效。
 * 数据是 0~180 的小数值 float，不会拼出 00 00 80 7F 这个序列，
 * 所以滑窗对齐在本协议下不会误触发。 */
static int RX_Pump(uint8_t b, float *pan, float *tilt)
{
  static uint8_t w[12];
  static uint8_t filled = 0;
  if (filled < 12) {
    w[filled++] = b;
  } else {
    memmove(w, w + 1, 11);
    w[11] = b;
  }
  if (filled == 12 && w[8] == 0x00 && w[9] == 0x00 && w[10] == 0x80 && w[11] == 0x7F) {
    memcpy(pan,  w,     4);
    memcpy(tilt, w + 4, 4);
    filled = 0;
    return 1;
  }
  return 0;
}

static void Send_JustFloat(const float *ch, uint32_t n)
{
  uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F};
  HAL_UART_Transmit(&huart1, (uint8_t *)ch, n * 4, 20);
  HAL_UART_Transmit(&huart1, tail, 4, 20);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  UART1_Init();
  PWM_Init();

  uint8_t banner[] = "\r\n=== Soft2: VOFA+ servo control ===\r\n";
  HAL_UART_Transmit(&huart1, banner, sizeof(banner) - 1, 100);

  uint32_t last_fb = HAL_GetTick();
  float pan_cmd = 90.0f, tilt_cmd = 90.0f;

  while (1) {
    uint8_t b;
    if (HAL_UART_Receive(&huart1, &b, 1, 2) == HAL_OK) {
      float p, t;
      if (RX_Pump(b, &p, &t)) {
        if (p < 0.0f)   p = 0.0f;
        if (p > 180.0f) p = 180.0f;
        if (t < 0.0f)   t = 0.0f;
        if (t > 180.0f) t = 180.0f;
        pan_cmd  = p;
        tilt_cmd = t;
        Servo_SetAngle((uint8_t)pan_cmd, (uint8_t)tilt_cmd);
      }
    }

    /* 20Hz 回传指令角 + PWM 占空比百分数，四通道 JustFloat */
    if (HAL_GetTick() - last_fb >= 50) {
      last_fb = HAL_GetTick();
      float fb[4];
      Servo_GetState(&fb[0], &fb[1], &fb[2], &fb[3]);
      Send_JustFloat(fb, 4);
    }
  }
}

static void UART1_Init(void)
{
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &g);

  huart1.Instance = USART1;
  huart1.Init.BaudRate = UART_BAUD;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  osc.PLL.PLLM = 16;
  osc.PLL.PLLN = 336;
  osc.PLL.PLLP = RCC_PLLP_DIV4;
  osc.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&osc);

  clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);   /* 84MHz 需 2 等待周期 */
}

void Error_Handler(void) { while (1) {} }
