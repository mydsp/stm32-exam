#include "can.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;
static void Print(const char *s) { HAL_UART_Transmit(&huart1, (uint8_t *)s, strlen(s), 100); }

/* CAN 寄存器位定义（F401 CMSIS 缺，从 F405 抄） */
#define CAN_BIT_INRQ    (1UL << 0)
#define CAN_BIT_INAK    (1UL << 0)
#define CAN_BIT_LBKM    (1UL << 30)
#define CAN_BIT_RQCP0   (1UL << 0)
#define CAN_BIT_RFOM0   (1UL << 5)
#define CAN_BIT_TXRQ    (1UL << 0)

void CAN_Init(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER &= ~(0xF << 16);
  GPIOB->MODER |= (0xA << 16);
  GPIOB->AFR[1] &= ~(0xFF);
  GPIOB->AFR[1] |= (0x99);

  CAN1->MCR |= CAN_BIT_INRQ;
  while (!(CAN1->MSR & CAN_BIT_INAK));

  CAN1->BTR = (7 << 0) | (6 << 16) | (1 << 20);
  CAN1->MCR = (1UL << 6);  /* ABOM=1, 其他=0 */

  CAN1->MCR &= ~CAN_BIT_INRQ;
  while (CAN1->MSR & CAN_BIT_INAK);

  Print("CAN1 loopback ready (1Mbps)\r\n");
}

void CAN_LoopbackTest(void)
{
  CAN1->MCR |= CAN_BIT_INRQ;
  CAN1->BTR |= CAN_BIT_LBKM;
  CAN1->MCR &= ~CAN_BIT_INRQ;
  while (CAN1->MSR & CAN_BIT_INAK);

  CAN1->sTxMailBox[0].TIR = (0x11 << 21);
  CAN1->sTxMailBox[0].TDTR = 1;
  CAN1->sTxMailBox[0].TDLR = 0xAA;
  CAN1->sTxMailBox[0].TIR |= CAN_BIT_TXRQ;

  while (!(CAN1->TSR & CAN_BIT_RQCP0));
  Print("TX: ID=0x11 data=0xAA\r\n");

  uint32_t id = (CAN1->sFIFOMailBox[0].RIR >> 21) & 0x7FF;
  uint8_t data = CAN1->sFIFOMailBox[0].RDLR & 0xFF;
  CAN1->RF0R |= CAN_BIT_RFOM0;

  char buf[64];
  snprintf(buf, sizeof(buf), "RX: ID=0x%02X data=0x%02X\r\n", (unsigned int)id, data);
  Print(buf);

  Print((id == 0x11 && data == 0xAA) ? "CAN LOOPBACK PASS!\r\n" : "CAN LOOPBACK FAIL\r\n");
}