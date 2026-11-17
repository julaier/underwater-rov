/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  // 使能GPIO端口时钟，确保GPIO端口可以正常工作,这里全部配置为可以用
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
   // 设置GPIO输出电平，gpioE,pin4为低电平，用于控制485芯片的收发模式，低电平为接收，高电平为发送
  HAL_GPIO_WritePin(USART10_485_GPIO_Port, USART10_485_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // gpioF,pin0和pin1为低电平，用于OLED的I2C通信，设置为开漏输出，外部上拉电阻
  HAL_GPIO_WritePin(GPIOF, OLED_SDA_Pin|OLED_SCL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // gpioA,pin1和pin7为低电平，用于控制电机驱动器的使能和方向
  HAL_GPIO_WritePin(GPIOA, EN2OUT_Pin|DIR2OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // pin4,pin5,pin10为低电平，用于控制电机驱动器的使能和方向,BM_ENB_Pin为电机使能，DIR1OUT_Pin为电机方向，EN1OUT_Pin为电机使能
  HAL_GPIO_WritePin(GPIOC, DIR1OUT_Pin|BM_ENB_Pin|EN1OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // gpioB,pin0 pin2为低电平，用于控制电机驱动器的使能和MPU6050的I2C通信，pin0为电机使能，pin2为I2C数据线
  HAL_GPIO_WritePin(GPIOB, BM_ENA_Pin|MPU_IIC_SDA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
   // gpioG,pin1和pin7为低电平，用于控制MPU6050的I2C通信和LED灯，pin1为I2C时钟线，pin13为LED灯
  HAL_GPIO_WritePin(GPIOG, MPU_IIC_SCL_Pin|LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USART10_485_Pin */
  
  GPIO_InitStruct.Pin = USART10_485_Pin;// PE4
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;// 设置为推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;// 不使用上拉或下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// 设置输出速度为低速
  HAL_GPIO_Init(USART10_485_GPIO_Port, &GPIO_InitStruct);// 初始化GPIOE的pin4为输出模式，用于控制485芯片的收发模式


  /*Configure GPIO pins : OLED_SDA_Pin OLED_SCL_Pin */
  GPIO_InitStruct.Pin = OLED_SDA_Pin|OLED_SCL_Pin;// oled的I2C通信引脚，PF0和PF1
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;// 设置为开漏输出模式
  GPIO_InitStruct.Pull = GPIO_PULLUP;// 使用上拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;// 设置输出速度为高速
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : EN2OUT_Pin DIR2OUT_Pin */
  GPIO_InitStruct.Pin = EN2OUT_Pin|DIR2OUT_Pin;// 电机驱动器的使能和方向引脚，PA1和PA7
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;// 设置为推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;// 不使用上拉或下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// 设置输出速度为低速
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DIR1OUT_Pin BM_ENB_Pin EN1OUT_Pin */
  GPIO_InitStruct.Pin = DIR1OUT_Pin|BM_ENB_Pin|EN1OUT_Pin;// 电机驱动器的使能和方向引脚，PC4,PC5,PC10
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;// 设置为推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;// 不使用上拉或下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// 设置输出速度为低速
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BM_ENA_Pin */
  GPIO_InitStruct.Pin = BM_ENA_Pin;// 电机驱动器的使能引脚，PC3
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;// 设置为推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;// 不使用上拉或下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// 设置输出速度为低速
  HAL_GPIO_Init(BM_ENA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MPU_IIC_SDA_Pin */
  GPIO_InitStruct.Pin = MPU_IIC_SDA_Pin;// MPU6050的I2C通信引脚，PG0
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;// 设置为开漏输出模式
  GPIO_InitStruct.Pull = GPIO_PULLUP;// 使用上拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;// 设置输出速度为高速
  HAL_GPIO_Init(MPU_IIC_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MPU_IIC_SCL_Pin LED_Pin */
  GPIO_InitStruct.Pin = MPU_IIC_SCL_Pin|LED_Pin;// MPU6050的I2C通信引脚，PG1和PG13，PG13为led灯
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;// 设置为推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;// 不使用上拉或下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// 设置输出速度为低速
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : tank_front_full_Pin tank_front_empty_Pin tank_rear_full_Pin tank_rear_empty_Pin */
  GPIO_InitStruct.Pin = tank_front_full_Pin|tank_front_empty_Pin|tank_rear_full_Pin|tank_rear_empty_Pin;// 水箱液位传感器引脚，PG2,PG3,PG4,PG5
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;// 设置为下降沿触发的中断模式
  GPIO_InitStruct.Pull = GPIO_PULLUP;// 使用上拉电阻
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);// 配置模拟开关，关闭PA1的模拟开关，确保PA1可以正常工作

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);// 设置外部中断优先级，EXTI2_IRQn为水箱液位传感器的中断，优先级为5，子优先级为0
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);// 设置外部中断优先级，EXTI3_IRQn为水箱液位传感器的中断，优先级为5，子优先级为0
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);// 设置外部中断优先级，EXTI4_IRQn为水箱液位传感器的中断，优先级为5，子优先级为0
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);// 设置外部中断优先级，EXTI9_5_IRQn为水箱液位传感器的中断，优先级为5，子优先级为0
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
