#ifndef __PWM_H__
#define __PWM_H__

#include "include.h"

// 周期值 == 系统时钟 / 预分频 / pwm频率，：
// 旧版和做过货的版本，这里的值都是6000，之后的修改要尽量保持在6000，否则会对调节有影响
#define STMR_ALL_PERIOD_VAL (SYSCLK / 1 / 8000) //
// #define STMR_ALL_PERIOD_VAL (SYSCLK / 1 / 24000) //
// #define STMR_ALL_PERIOD_VAL (SYSCLK / 1 / 32000) //
#define STMR0_PEROID_VAL (STMR_ALL_PERIOD_VAL)
#define STMR1_PEROID_VAL (STMR_ALL_PERIOD_VAL)

#define MAX_PWM_DUTY (STMR_ALL_PERIOD_VAL + 1) // 100%占空比   (SYSCLK 4800 0000 /  8000  == 6000)

enum
{
    PWM_DUTY_100_PERCENT = MAX_PWM_DUTY,                       // 100%占空比
    PWM_DUTY_80_PERCENT = (u16)((u32)MAX_PWM_DUTY * 80 / 100), // 80%
    PWM_DUTY_75_PERCENT = (u16)((u32)MAX_PWM_DUTY * 75 / 100), // 75%占空比
    PWM_DUTY_50_PERCENT = (u16)((u32)MAX_PWM_DUTY * 50 / 100), // 50%占空比
    PWM_DUTY_35_PERCENT = (u16)((u32)MAX_PWM_DUTY * 35 / 100), // 35%占空比
    PWM_DUTY_30_PERCENT = (u16)((u32)MAX_PWM_DUTY * 30 / 100), // 30%占空比
    PWM_DUTY_25_PERCENT = (u16)((u32)MAX_PWM_DUTY * 25 / 100), // 25%占空比
};

// 定时，限制占空比的时间（用于上电多久之后，限制最大的占空比），单位：ms
#define SCHEDULE_TIME_TO_LIMIT_PWM ((u32)1 * 60 * 60 * 1000)
// #define SCHEDULE_TIME_TO_LIMIT_PWM ((u32)6 * 60 * 1000) // 6min，给客户测试用
// #define SCHEDULE_TIME_TO_LIMIT_PWM ((u32)30 * 1000) // USER_TO_DO 测试时使用
// 时间到来之后，要限制的最大占空比值
#define SCHEDULE_TIME_TO_LIMIT_PWM_VAL ((u16)PWM_DUTY_80_PERCENT)

// 由温度限制的PWM占空比 （对所有PWM通道都生效）
extern volatile u16 limited_pwm_duty_due_to_temp;
// 由于发动机不稳定，而限制的可以调节到的占空比（对所有PWM通道都生效，默认为最大占空比）
extern volatile u16 limited_pwm_duty_due_to_unstable_engine;

extern volatile u16 cur_pwm_channel_0_duty;           // 当前设置的、 pwm_channle_0 的占空比
extern volatile u16 expect_adjust_pwm_channel_0_duty; // 存放期望调节到的 pwm_channle_0 占空比
extern volatile u16 adjust_pwm_channel_0_duty;        // pwm_channle_0 要调整到的占空比

extern volatile bit flag_is_time_to_limit_pwm;

void pwm_init(void);
void set_pwm_channel_0_duty(u16 channel_duty);
extern u8 get_pwm_channel_0_status(void); // 获取第一路PWM的运行状态
extern void pwm_channel_0_enable(void);
extern void pwm_channel_0_disable(void);

u16 get_pwm_channel_x_adjust_duty(u16 pwm_adjust_duty);

#endif