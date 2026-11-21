/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "H_Tmc2209.h"
#include "bsp_delay.h"
// #include "OLED.h"
// #include "mpu6050.h"
#include "bsp_jy901.h"
#include "control.h"
#include "WaterADC.h"
#include "WaterTank.h"
#include "gamepad.h"
#include "app_gps.h"
#include "app_thrusters.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern osMutexId_t DebugUsartMutexHandle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
   //mpu内存保护配置
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   // 初始化 HAL 库及基础时基；。滴答定时器，会创建一个1ms的中断，调用HAL_IncTick()函数，uwTick自增1，还有freertos的时间管理，创建一个1ms的中断，调用osSystickHandler()函数，freertos的系统节拍自增1
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  // 配置时钟树
  SystemClock_Config();

  /* Configure the peripherals common clocks */
   // 配置公共外设时钟；当前实现主要设置 ADC 时钟来源。
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Delay(500);  //加个延时等其他设备启动

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();// 使能 DMA 控制器时钟，确保 DMA 可以正常工作，这里使能了Ahb1的dma1时钟，dma1用于ADC3的DMA传输,dma2用于ADC2的DMA传输
  MX_TIM2_Init();// 设置TIM2的时钟分频、计数模式、周期等参数，TIM2用于定时器中断，进行任务调度，于滴答定时器不同，滴答定时器是系统级的，TIM2是应用级的
  MX_TIM5_Init();// 设置TIM5的时钟分频、计数模式、周期等参数，TIM5用于PWM输出，可以控制电机的速度和方向，三个电机分别使用TIM2的通道1、2、3输出PWM信号
  MX_ADC3_Init();// 设置ADC3的采样时间、分辨率、通道等参数，ADC3用于采集水箱液位传感器的模拟信号，并通过DMA传输到内存中
  MX_ADC2_Init();// 设置ADC2的采样时间、分辨率、通道等参数，ADC2用于采集水箱液位传感器的模拟信号，并通过DMA传输到内存中
  MX_TIM1_Init();// 设置TIM1的时钟分频、计数模式、周期等参数，TIM1用于PWM输出，可以控制电机的速度和方向，三个电机分别使用TIM1的通道1、2、3输出PWM信号
  MX_TIM8_Init();// 设置TIM8的时钟分频、计数模式、周期等参数，TIM8用于PWM输出，可以控制电机的速度和方向，三个电机分别使用TIM8的通道1、2、3输出PWM信号
  MX_TIM3_Init();// 设置TIM3的时钟分频、计数模式、周期等参数，TIM3用于PWM输出，可以控制电机的速度和方向，三个电机分别使用TIM3的通道1、2、3输出PWM信号
  MX_USART1_UART_Init();// 设置USART1的波特率、数据位、停止位、校验位等参数，
  // 波特率为9600，数据位为8位，停止位为1位，无校验位，使用DMA接收数据，接收缓冲区为USART1_RX_BUF，大小为USART1_RX_BUF_SIZE
  MX_USART3_UART_Init();// 设置USART3的波特率、数据位、停止位、校验位等参数，
  // 波特率为115200，数据位为8位，停止位为1位，无校验位，使用DMA接收数据，接收缓冲区为USART3_RX_BUF，大小为USART3_RX_BUF_SIZE
  MX_USART2_UART_Init();// 设置USART2的波特率、数据位、停止位、校验位等参数，
  // 波特率为115200，数据位为8位，停止位为1位，无校验位，使用DMA接收数据，接收缓冲区为USART2
  MX_ADC1_Init();// 设置ADC1的采样时间、分辨率、通道等参数，ADC1用于采集水箱液位传感器的模拟信号，并通过DMA传输到内存中
  MX_TIM8_Init();// 设置TIM8的时钟分频、计数模式、周期等参数，TIM8用于PWM输出，可以控制电机的速度和方向，三个电机分别使用TIM8的通道1、2、3输出PWM信号
  MX_USART6_UART_Init();// 设置USART6的波特率、数据位、停止位、校验位等参数，
  // 波特率为9600，数据位为8位，停止位为1
  MX_USART10_UART_Init();// 设置USART10的波特率、数据位、停止位、校验位等参数，
  // 波特率为9600，数据位为8位，停止位为1

  /* USER CODE BEGIN 2 */
  Gamepad_Init(&gamepad_huart);  // 手柄初始化
  // OLED_Init();
  // MPU6050_Init();
  BSP_JY901_Init();// 陀螺仪初始化
  Water_Tank_Init(); // 初始化排水，会卡死
  WaterADC_Init();// 水位传感器初始化
  APP_GPS_Init();// GPS初始化
  APP_Thrusters_Init(); // 无刷电机初始化
  
  HAL_Delay(50);

  HAL_GPIO_WritePin(USART10_485_GPIO_Port, USART10_485_Pin, GPIO_PIN_RESET); // 485接收使能,pe4拉低，接收使能
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();// freertos初始化，创建任务、队列、信号量等对象。

  /* Start scheduler */
  osKernelStart();// 内核开启

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 32;
  PeriphClkInitStruct.PLL2.PLL2N = 129;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Debug_USART_Show(const char* str)
{
  // 通过互斥锁保护串口资源，防止多任务同时访问导致数据混乱
  xSemaphoreTake(DebugUsartMutexHandle, portMAX_DELAY);
  HAL_UART_Transmit(&DEBUG_huart, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
  xSemaphoreGive(DebugUsartMutexHandle);
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  // mpu内存保护单元配置结构体
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
 // 配置MPU区域0，覆盖整个4GB地址空间，禁止访问,也就是说所有的内存访问都将被禁止，
  //除非在后续的代码中重新配置MPU区域以允许访问特定的内存区域，这段只能读取，不能写入和执行指令
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;// 启用MPU区域
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;// 配置MPU区域0
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;// 禁止访问子区域0、1、2、3、7，允许访问子区域4、5、6
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;// 配置内存类型扩展字段为0，表示该区域为普通内存
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;// 配置访问权限为禁止访问
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;// 禁止执行指令
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;// 配置为可共享内存
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;// 配置为不可缓存内存
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;// 配置为不可缓冲内存

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);// MPU使能

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
 // 错误处理函数，当HAL库函数返回错误状态时调用该函数，通常用于调试和错误处理，但是这里直接进入死循环，
 // 用户可以根据需要修改该函数的实现，比如打印错误信息、重启系统等
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
