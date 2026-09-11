#ifndef __PWM_H
#define __PWM_H
#include "main.h"

void PWM_Init(void);
void Servo_SetAngle(uint8_t pan, uint8_t tilt);
/* 回传用：把最近一次指令角与对应 PWM 占空比（百分数）读出来 */
void Servo_GetState(float *pan_deg, float *pan_duty_pct,
                    float *tilt_deg, float *tilt_duty_pct);

#endif
