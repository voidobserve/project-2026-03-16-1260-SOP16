#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include "include.h" // 芯片官方提供的头文件
#include "pwm.h"
#include "adc.h"
#include "time0.h" // 定时器0
#include "timer1.h"
#include "timer2.h"
#include "knob_dimming.h" // 旋钮调光头文件

#define U32_MAX_VAL ((u32)0xFFFFFFFF)
#define U16_MAX_VAL ((u16)0xFFFF)

// 是否使用打印调试
// 使用 P00 作为 串口打印输出 （实际用到的芯片的第4脚）
#define USE_MY_DEBUG 0

#if USE_MY_DEBUG
#include <stdio.h>
#endif // USE_MY_DEBUG

/*
    测试引脚：（当前工程暂无）

*/
#define USE_MY_TEST_PIN 0 // 是否使用测试用的引脚（开发板没有相关的引脚，用其他空闲的引脚来代替）

// tmr1配置成每10ms产生一次中断，计数值加一，
// 这里定义时间对应的计数值

#define ARRAY_SIZE(n) (sizeof(n) / sizeof(n[0]))

// #define TMR1_CNT_30_MINUTES 180000UL // 30min（这个是可以在30min后调节PWM占空比的）

#define TMR1_CNT_5_MINUTES 30000UL // 5min

// // 测试用的计数值
// #define TMR1_CNT_30_MINUTES 1000 // 10s
// #define TMR1_CNT_30_MINUTES 18000UL // 3min

// 在热敏电阻端检测的电压值与温度对应的关系，电压值单位：mV
#define VOLTAGE_TEMP_75 (3050) // 这一个值在客户那边测试出来是74摄氏度,对应的电压是3.1V（实际测得是在2.73V左右）

// #define VOLTAGE_TEMP_75 (2990) // 75摄氏度对应的电压值
// #define VOLTAGE_TEMP_65 (3388) // 65摄氏度对应的电压值
// #define VOLTAGE_TEMP_50 (3924) // 50摄氏度对应的电压值

// 温度状态定义
enum
{
    TEMP_NORMAL, // 正常温度
    TEMP_75,     // 超过75摄氏度（±5摄氏度）
    // TEMP_75_30MIN, // 超过75摄氏度（±5摄氏度）30min
    // TEMP_75_5_MIN, // 超过75摄氏度（±5摄氏度）5min
};

#define ADC_VAL_WHEN_UNSTABLE (2243) // 9脚检测到电压不稳定、发动机功率不足时，对应的的阈值，大于该值就认为不稳定
// #define ADC_VAL_WHEN_UNSTABLE (2438) // 9脚检测到电压不稳定、发动机功率不足时，对应的的阈值，大于该值就认为不稳定
// #define ADC_VAL_WHEN_UNSTABLE (2600) // 9脚检测到电压不稳定、发动机功率不足时，对应的的阈值，大于该值就认为不稳定
// #define ADC_VAL_WHEN_UNSTABLE (2730) // 9脚检测到电压不稳定、发动机功率不足时，对应的的阈值，大于该值就认为不稳定

#endif
