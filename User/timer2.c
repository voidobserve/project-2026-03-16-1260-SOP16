#include "timer2.h"
#include "power_on.h"
#define TIMER2_PEROID_VAL (SYSCLK / 128 / 10000 - 1) // 周期值=系统时钟/分频/频率 - 1

static volatile u16 pwm_duty_add_cnt; // 用于控制pwm增加的时间计数
static volatile u16 pwm_duty_sub_cnt; // 用于控制pwm递减的时间计数

volatile bit flag_is_pwm_add_time_comes = 0; // 标志位，pwm占空比递增时间到来
volatile bit flag_is_pwm_sub_time_comes = 0; // 标志位，pwm占空比递减时间到来

static volatile u16 pwm_duty_change_cnt = 0; // 用于控制pwm变化的时间计数（用在旋钮调节的PWM占空比中）

void timer2_config(void)
{
    __EnableIRQ(TMR2_IRQn); // 使能timer2中断
    IE_EA = 1;              // 使能总中断

    // 设置timer2的计数功能，配置一个频率为1kHz的中断
    TMR_ALLCON = TMR2_CNT_CLR(0x1);                               // 清除计数值
    TMR2_PRH = TMR_PERIOD_VAL_H((TIMER2_PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR2_PRL = TMR_PERIOD_VAL_L((TIMER2_PEROID_VAL >> 0) & 0xFF);
    TMR2_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                          // 计数等于周期时允许发生中断
    TMR2_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x7) | TMR_MODE_SEL(0x1); // 选择系统时钟，128分频，计数模式
}

// 定时器 中断服务函数
void TIMR2_IRQHandler(void) interrupt TMR2_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR2_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR2_CONH & TMR_PRD_PND(0x1)) // 约100us触发一次中断
    {
        TMR2_CONH |= TMR_PRD_PND(0x1); // 清除pending

        pwm_duty_add_cnt++;
        pwm_duty_sub_cnt++;
        pwm_duty_change_cnt++;

        if (pwm_duty_sub_cnt >= 13) // 1300us，1.3ms
        // if (pwm_duty_sub_cnt >= 50)
        {
            pwm_duty_sub_cnt = 0;
            flag_is_pwm_sub_time_comes = 1;
        }

        // if (pwm_duty_add_cnt >= 133) // 13300us, 13.3ms
        if (pwm_duty_add_cnt >= 13) // 1300us，1.3ms
        {
            pwm_duty_add_cnt = 0;
            flag_is_pwm_add_time_comes = 1;
        }

#if 1                                  // 调节PWM占空比
                                       // if (pwm_duty_change_cnt >= 10) // 1000us,1ms
                                       // if (pwm_duty_change_cnt >= 1) // 100us（用遥控器调节，在50%以上调节pwm占空比的时候，灯光会有抖动）
                                       // if (pwm_duty_change_cnt >= 5) // 500us
                                       // if (pwm_duty_change_cnt >= 10) // x * 100us （用遥控器调节到50%以下pwm占空比的时候，灯光会有抖动）
        if (pwm_duty_change_cnt >= 20) // x * 100us （ 用遥控器调节时，灯光不会有抖动，样机最高功率为870W--加上风扇）
        // if (pwm_duty_change_cnt >= 30) // x * 100us （用遥控器调节时，灯光不会有抖动，但是调节时间过长，感觉不跟手）
        {

            pwm_duty_change_cnt = 0;

            if (0 == flag_is_in_power_on) // 不处于开机缓启动，才使能PWM占空比调节
            {
                // =================================================================
                // pwm_channel_0                                               //
                // =================================================================
                if (adjust_pwm_channel_0_duty > cur_pwm_channel_0_duty)
                {
                    cur_pwm_channel_0_duty++;
                }
                else if (adjust_pwm_channel_0_duty < cur_pwm_channel_0_duty)
                {
                    cur_pwm_channel_0_duty--;
                }

                set_pwm_channel_0_duty(cur_pwm_channel_0_duty);

                if (cur_pwm_channel_0_duty <= 0)
                {
                    // 小于某个值，直接输出0%占空比，关闭PWM输出，引脚配置为输出模式
                    pwm_channel_0_disable();
                }
                else // 如果大于0
                {
                    pwm_channel_0_enable();
                }

            } // if (0 == flag_is_in_power_on) // 不处于开机缓启动，才使能PWM占空比调节
        }
#endif // 调节PWM占空比
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR2_IRQn);
}