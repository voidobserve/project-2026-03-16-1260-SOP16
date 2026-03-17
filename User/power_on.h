#ifndef __POWER_ON_H__
#define __POWER_ON_H__

#include "typedef.h"
#include "pwm.h"

/**
 * @brief 开机缓启动调节函数
 * @attention 需要先初始化对应的定时器，控制每次调节pwm的时间
 *
 * @return * void
 */
#define DEST_POWER_ON_DUTY_VAL ((u16)PWM_DUTY_100_PERCENT)
// 开机缓启动的时间，单位：ms
#define POWER_ON_TIMES ((u16)8000)
/*
    开机缓启动，每ms调节的pwm占空比值，单位：0.001（注意单位）

    这里是每次调节的步长，尽量让每ms调节的步长小于等于1（ POWER_ON_ADJUST_STEP <= 1000 ）
*/
#define POWER_ON_ADJUST_STEP ((u32)DEST_POWER_ON_DUTY_VAL * 1000 / POWER_ON_TIMES)

extern volatile bit flag_is_in_power_on;             // 是否处于开机缓启动
extern volatile bit flag_time_comes_during_power_on; // 标志位，开机缓启动期间，调节时间到来

void power_on_handle(void);

#endif