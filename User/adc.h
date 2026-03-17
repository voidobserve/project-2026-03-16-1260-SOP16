#ifndef _ADC_H
#define _ADC_H

#include <stdio.h>
#include <include.h>
#include "pwm.h"
#include <string.h>

// ADC 参考电压 VCCA，单位：mV ，客户测得是4.87V
#define ADC_REF_VOLTAGE_VAL ((u16)4870) // 单位：mV

// 定义检测adc的通道:
enum
{
    ADC_SEL_PIN_NONE = 0,

    ADC_SEL_PIN_ENGINE,
    ADC_SEL_PIN_KNOB,
    ADC_SEL_PIN_TEMP,
};

enum
{
    ADC_STATUS_NONE = 0,

    ADC_STATUS_SEL_ENGINE_WAITING, // 等待adc稳定
    ADC_STATUS_SEL_ENGINE,         // 切换至检测发动机的通道

    ADC_STATUS_SEL_ENGINE_DONE, // 检测发动机的通道期间要连续检测，这里表示连续检测完成

    ADC_STATUS_SEL_KNOB_WAITING, // 等待adc稳定
    ADC_STATUS_SEL_KNOB,         // 切换至检测旋钮的通道

    ADC_STATUS_SEL_GET_TEMP_WAITING, // 等待adc稳定
    ADC_STATUS_SEL_GET_TEMP,         // 切换至检测热敏电阻的通道
};

extern volatile u8 cur_adc_status; // 状态机，表示当前adc的状态

extern volatile u16 adc_val_from_engine; // 存放 从发动机一侧 检测到的ad值
extern volatile u16 adc_val_from_knob;   // 存放 从旋钮一侧 采集到的ad值
extern volatile u16 adc_val_from_temp;   // 存放 从热敏电阻一侧 采集到的ad值

void adc_pin_config(void); // adc相关的引脚配置，调用完成后，还未能使用adc
void adc_config(void);

void adc_channel_sel(const u8 pin); // 切换adc采集的引脚，并配置好adc

u32 get_voltage_from_pin(void); // 从引脚上采集滤波后的电压值

void temperature_scan(void);
void set_duty(void);

#endif