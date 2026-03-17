#include "power_on.h"
#include "include.h"
#include "pwm.h"

volatile bit flag_is_in_power_on = 0; // 是否处于开机缓启动
static volatile u32 power_on_step = 0;
volatile bit flag_time_comes_during_power_on = 0; // 标志位，开机缓启动期间，调节时间到来（由定时器置位）

// 控制开机缓启动
void power_on_handle(void)
{
    cur_pwm_channel_0_duty = 0;
    flag_is_in_power_on = 1; // 表示到了开机缓启动

    while (1)
    {
        update_max_pwm_duty_coefficient();

        if (cur_pwm_channel_0_duty >= limited_max_pwm_duty)
        {
            // 当两路pwm都到对应的占空比值之后，才退出开机缓启动
            break;
        }

        if (flag_time_comes_during_power_on) // 如果调节时间到来
        {
            flag_time_comes_during_power_on = 0;
            power_on_step += POWER_ON_ADJUST_STEP; // 累计步长
            if (power_on_step >= 1000)
            {
                power_on_step -= 1000;
                cur_pwm_channel_0_duty++; 
            }
        } 
        
        set_pwm_channel_0_duty(cur_pwm_channel_0_duty);
    }

    // 缓启动后，立即更新 adjust_pwm_channel_0_duty 的值：（ 要给下面这些变量赋值，上电后会根据这些变量的值来调节 ）
    expect_adjust_pwm_channel_0_duty = cur_pwm_channel_0_duty; 
    adjust_pwm_channel_0_duty = cur_pwm_channel_0_duty;
    flag_is_in_power_on = 0; // 表示退出了开机缓启动
}