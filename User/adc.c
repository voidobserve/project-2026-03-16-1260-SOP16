#include "adc.h"
#include "my_config.h"
#include "engine.h"

// 存放温度状态的变量
volatile u8 temp_status = TEMP_NORMAL;

// USER_TO_DO 需要改成初始值是正常状态下的值：
volatile u16 adc_val_from_engine = U16_MAX_VAL; // 存放 从发动机一侧 采集到的ad值
volatile u16 adc_val_from_knob = U16_MAX_VAL;   // 存放 从旋钮一侧 采集到的ad值
volatile u16 adc_val_from_temp = U16_MAX_VAL;   // 存放 从热敏电阻一侧 采集到的ad值

volatile u8 cur_adc_status = ADC_STATUS_NONE; // 状态机，表示当前adc的状态

// adc相关的引脚配置
void adc_pin_config(void)
{
    // P30--8脚配置为模拟输入模式
    P3_MD0 |= GPIO_P30_MODE_SEL(0x3);

    // P27--9脚配置为模拟输入模式
    P2_MD1 |= GPIO_P27_MODE_SEL(0x3);

    // P31--7脚配置为模拟输入模式
    P3_PU &= ~(0x01 << 1); // 关闭上拉
    P3_PD &= ~(0x01 << 1); // 关闭下拉
    P3_MD0 |= GPIO_P31_MODE_SEL(0x3);
}

void adc_config(void)
{
    __EnableIRQ(ADC_IRQn);    // 使能ADC中断
    IE_EA = 1;                // 使能总中断
    ADC_CFG1 |= (0x0F << 3) | // ADC时钟分频为16分频，为系统时钟/16
                (0x01 << 0);  // ADC0 通道中断使能
    delay_ms(1);              // 等待adc稳定
}

void adc_channel_sel(const u8 adc_sel_pin)
{
    switch (adc_sel_pin)
    {
    case ADC_SEL_PIN_ENGINE:
        // ADC配置
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x01)); // 关闭外部参考电压，清除选择的参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x5) |                         // 选择内部参考电压 4.2V (用户手册说未校准)
                     ADC_TEN_SEL(0x3);                           /* 关闭测试信号 */
        ADC_ACON0 = ADC_CMP_EN(0x1) |                            // 打开ADC中的CMP使能信号
                    ADC_BIAS_EN(0x1) |                           // 打开ADC偏置电流能使信号
                    ADC_BIAS_SEL(0x1);                           // 偏置电流：1x
        ADC_CHS0 = ADC_ANALOG_CHAN(0x17) |                       // 选则引脚对应的通道（0x17--P27）
                   ADC_EXT_SEL(0x0);                             // 选择外部通道
        break;
        // =================================================================================================
    case ADC_SEL_PIN_KNOB:                 // 检测旋钮调光
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x7)); // 关闭外部参考电压、清除选择的参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x6) |   // 选择内部参考电压VCCA
                     ADC_TEN_SEL(0x3);     // 关闭测试信号
        ADC_ACON0 = ADC_CMP_EN(0x1) |      // 打开ADC中的CMP使能信号
                    ADC_BIAS_EN(0x1) |     // 打开ADC偏置电流能使信号
                    ADC_BIAS_SEL(0x1);     // 偏置电流：1x
        ADC_CHS0 = ADC_ANALOG_CHAN(0x19) | // 选则引脚对应的通道（0x19--P31）
                   ADC_EXT_SEL(0x0);       // 选择外部通道
        break;
        // =================================================================================================
    case ADC_SEL_PIN_TEMP:                                                       // 采集热敏电阻对应的电压的引脚
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0) | ADC_INREF_SEL(0)); // 关闭外部参考电压，清除选择的参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x6) |                                         // 选择内部参考电压VCCA
                     ADC_TEN_SEL(0x3);                                           // 关闭测试信号
        ADC_ACON0 = ADC_CMP_EN(0x1) |                                            // 打开ADC中的CMP使能信号
                    ADC_BIAS_EN(0x1) |                                           // 打开ADC偏置电流能使信号
                    ADC_BIAS_SEL(0x1);                                           // 偏置电流：1x

        ADC_CHS0 = ADC_ANALOG_CHAN(0x18) | // 选则引脚对应的通道（0x18--P30）
                   ADC_EXT_SEL(0x0);       // 选择外部通道
        break;
        // =================================================================================================
    default:
        break;
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0转换
                ADC_EN(0x1);        // 使能A/D转换
}

// 从引脚上采集滤波后的电压值,函数内部会将采集到的ad转换成对应的电压值
u32 get_voltage_from_pin(void)
{
    // 4095（adc转换后，可能出现的最大的值） * 0.0012 == 4.914，约等于5V（VCC）
    return adc_val_from_temp * 12 / 10; // 假设是4095，4095 * 12/10 == 4915mV
}

// 温度检测功能
void temperature_scan(void)
{
    volatile u32 voltage = 0; // 存放adc采集到的电压，单位：mV

    // 如果已经超过75摄氏度且超过5min，不用再检测8脚的电压，等待用户排查原因，再重启（重新上电）
    // if (TEMP_75_5_MIN == temp_status)
    // {
    //     return;
    // }

    // 如果已经超过75摄氏度，不用再检测8脚的电压，等待用户排查原因，再重启（重新上电）
    if (TEMP_75 == temp_status)
    {
        return;
    }

    voltage = get_voltage_from_pin(); // 采集热敏电阻上的电压
    if (voltage >= (u32)U16_MAX_VAL * 12 / 10)
    {
        // 如果还没有采集到数据，adc_val_from_temp == U16_MAX_VAL，直接返回
        // printf("adc tmp not ready\n");
        return;
    }

    // 如果之前的温度为正常，检测是否超过75摄氏度（±5摄氏度）
    if (TEMP_NORMAL == temp_status)
    {
        // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
        static volatile u8 cnt = 0;
        if (voltage < VOLTAGE_TEMP_75) // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
        {
            cnt++;
        }
        else
        {
            cnt = 0;
        }

        if (cnt >= 10)
        {
            cnt = 0;
            // 测试的时候，如果引脚悬空，可能会执行不到这里：
            temp_status = TEMP_75; // 状态标志设置为超过75摄氏度   USER_TO_DO  在测试时会屏蔽掉，需要检查一下
        }
        else
        {
            temp_status = TEMP_NORMAL;
        }

        return; // 函数返回，让调节占空比的函数先进行调节
    }
#if 0
    else if (TEMP_75 == temp_status)
    {
        // 如果之前的温度超过75摄氏度
        static bit tmr1_is_open = 0;

        if (0 == tmr1_is_open)
        {
            tmr1_is_open = 1;
            tmr1_cnt = 0;
            tmr1_enable(); // 打开定时器，开始记录是否大于75摄氏度且超过一定时间
        }

        // 如果超过75摄氏度并且过了5min，再检测温度是否超过75摄氏度
        if (tmr1_cnt >= (u32)TMR1_CNT_5_MINUTES)
        {
            static volatile u8 cnt = 0;

            if (voltage < VOLTAGE_TEMP_75) // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
            {
                cnt++;
            }
            else
            {
                cnt = 0;
            }

            if (cnt >= 10)
            {
                cnt = 0;
                temp_status = TEMP_75_5_MIN; // 状态标志设置为超过75摄氏度且超过5min
                tmr1_disable();              // 关闭定时器
                tmr1_cnt = 0;                // 清空时间计数值
                tmr1_is_open = 0;
            }
            else
            {
                temp_status = TEMP_75;
            }

            return;
        }
    }
#endif
}

// 根据温度（电压值扫描）或9脚的状态来设定占空比
void set_duty(void)
{
    // 如果温度正常，根据9脚的状态来调节PWM占空比
    if (TEMP_NORMAL == temp_status)
    {
        if (flag_is_time_to_check_engine)
        {
            flag_is_time_to_check_engine = 0;
            according_pin9_to_adjust_pwm();
        }
    }
    else if (TEMP_75 == temp_status)
    {
        // limited_pwm_duty_due_to_temp = PWM_DUTY_50_PERCENT; // 将pwm占空比限制到最大占空比的 50%
        limited_pwm_duty_due_to_temp = PWM_DUTY_35_PERCENT; // 将pwm占空比限制到最大占空比的 35%
    }
    // else if (TEMP_75_5_MIN == temp_status)
    // {
    //     limited_pwm_duty_due_to_temp = PWM_DUTY_25_PERCENT; // 将pwm占空比限制到最大占空比的 25%
    // }
}

void ADC_IRQHandler(void) interrupt ADC_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(ADC_IRQn);

    // ---------------- 用户函数处理 -------------------
    if (ADC_STA & ADC_CHAN0_DONE(0x01))
    {
        volatile u16 adc_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 先接收ad值
        ADC_STA |= ADC_CHAN0_DONE(0x01);                              // 清除ADC0转换完成标志位

        if (ADC_STATUS_SEL_ENGINE == cur_adc_status)
        {
            // 更新发动机检测一端的ad值

            static u8 i = 0; // adc采集次数的计数
            static volatile u32 g_tmpbuff = 0;
            static volatile u16 g_adcmax = 0;
            static volatile u16 g_adcmin = 0xFFFF;

            if (i < 20)
            {
                i++;

                if (i >= 2) // 丢弃前两次采样值
                {
                    if (adc_val > g_adcmax)
                        g_adcmax = adc_val; // 最大
                    if (adc_val < g_adcmin)
                        g_adcmin = adc_val; // 最小
                    g_tmpbuff += adc_val;
                }

                if (i < 20)
                    ADC_CFG0 |= 0x01 << 0; // 开启adc0转换
            }

            if (i >= 20)
            {
                adc_val_from_engine = (g_tmpbuff >> 4); // 除以16，取平均值
                cur_adc_status = ADC_STATUS_SEL_ENGINE_DONE;

                // 重新初始化使用到的变量：
                i = 0;
                g_adcmax = 0;
                g_adcmin = 0xFFFF;
                g_tmpbuff = 0;
                // printf("1 engine scan done\n");
            }
        }
        else if (ADC_STATUS_SEL_KNOB == cur_adc_status)
        {
            // 更新旋钮检测一端的ad值
            adc_val_from_knob = adc_val;
            // printf("2 knob scan done\n");
        }
        else if (ADC_STATUS_SEL_GET_TEMP == cur_adc_status)
        {
            // 更新热敏电阻检测一端的ad值
            adc_val_from_temp = adc_val;
            // printf("3 temp scan done\n");
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(ADC_IRQn);
}