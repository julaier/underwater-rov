/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_task.h"
#include "tim.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for SystemTask */
osThreadId_t SystemTaskHandle;
const osThreadAttr_t SystemTask_attributes = {
  .name = "SystemTask",// 任务名称
  .stack_size = 512 * 4,// 512 * 4 = 2048 字节
  .priority = (osPriority_t) osPriorityLow,// 优先级为-2
};
/* Definitions for FocTask */
osThreadId_t FocTaskHandle;
const osThreadAttr_t FocTask_attributes = {
  .name = "FocTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for NavigationTask */
osThreadId_t NavigationTaskHandle;
const osThreadAttr_t NavigationTask_attributes = {
  .name = "NavigationTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for BuoyancyTask */
osThreadId_t BuoyancyTaskHandle;
const osThreadAttr_t BuoyancyTask_attributes = {
  .name = "BuoyancyTask",
  .stack_size = 768 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for CommTask */
osThreadId_t CommTaskHandle;
const osThreadAttr_t CommTask_attributes = {
  .name = "CommTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for GamepadQueue */
osMessageQueueId_t GamepadQueueHandle;
const osMessageQueueAttr_t GamepadQueue_attributes = {
  .name = "GamepadQueue"
};
/* Definitions for WaterTankQueue */
osMessageQueueId_t WaterTankQueueHandle;
const osMessageQueueAttr_t WaterTankQueue_attributes = {
  .name = "WaterTankQueue"
};
/* Definitions for DebugUsartMutex */
osMutexId_t DebugUsartMutexHandle;
const osMutexAttr_t DebugUsartMutex_attributes = {
  .name = "DebugUsartMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void vSystemTask(void *argument);
void vFocTask(void *argument);
void vNavigationTask(void *argument);
void vBuoyancyTask(void *argument);
void vCommTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of DebugUsartMutex */
  DebugUsartMutexHandle = osMutexNew(&DebugUsartMutex_attributes);//创建一把供调试串口使用的锁，把这把锁的“标识”保存到 DebugUsartMutexHandle
  // 也就是每个任务打印调试信息前都要先获取这把锁，打印完后再释放这把锁，这样就不会出现多个任务同时打印调试信息导致的串口输出混乱的问题
  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of GamepadQueue */
  GamepadQueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &GamepadQueue_attributes);// 创建消息队列，返回消息队列句柄

  /* creation of WaterTankQueue */
  /* 最多存放 10 条消息
  * 每条消息占 2 字节
  * 队列属性
  */
  WaterTankQueueHandle = osMessageQueueNew (10, sizeof(uint16_t), &WaterTankQueue_attributes);// 创建消息队列，返回消息队列句柄

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
 // 优先级：SystemTask < CommTask < NavigationTask < BuoyancyTask < FocTask
 // 任务调度：FocTask（10ms周期）> BuoyancyTask（50ms周期）
 // > NavigationTask（100ms周期）> CommTask（500ms周期）> SystemTask（20周期）
  /* Create the thread(s) */
  /* creation of SystemTask */
  // 创建system任务线程，用于系统管理，优先级最低，周期20ms
  SystemTaskHandle = osThreadNew(vSystemTask, NULL, &SystemTask_attributes);// 创建线程，返回线程句柄

  /* creation of FocTask */
  // 创建foc任务线程，用于电机控制，优先级最高，周期10ms
  FocTaskHandle = osThreadNew(vFocTask, NULL, &FocTask_attributes);// 创建线程，返回线程句柄

  /* creation of NavigationTask */
  // 导航任务线程，用于导航控制，，周期100ms，优先级第3
  NavigationTaskHandle = osThreadNew(vNavigationTask, NULL, &NavigationTask_attributes);// 创建线程，返回线程句柄

  /* creation of BuoyancyTask */
  // 水舱任务线程，用于水舱控制，周期50ms，优先级仅次于foc
  BuoyancyTaskHandle = osThreadNew(vBuoyancyTask, NULL, &BuoyancyTask_attributes);// 创建线程，返回线程句柄

  /* creation of CommTask */
  // 通信任务线程，用于通信控制，周期500ms，优先级仅高于system
  CommTaskHandle = osThreadNew(vCommTask, NULL, &CommTask_attributes);// 创建线程，返回线程句柄

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vSystemTask */
/**
* @brief Function implementing the SystemTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vSystemTask */
void vSystemTask(void *argument)
{
  /* USER CODE BEGIN vSystemTask */
  /* Infinite loop */
  // 优先级：SystemTask < CommTask < NavigationTask < BuoyancyTask < FocTask
  // 任务调度：FocTask（10ms周期）> BuoyancyTask（50ms周期）> NavigationTask（100ms周期）> CommTask（500ms周期）> SystemTask（20周期）
  TickType_t xLastWakeTime = xTaskGetTickCount();// 得到滴答数
  for(;;)
  {
    // 任务运行
    System_Task();

    // 每20ms检查一次
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
  }
  /* USER CODE END vSystemTask */
}

/* USER CODE BEGIN Header_vFocTask */
/**
* @brief Function implementing the FocTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vFocTask */
void vFocTask(void *argument)
{
  /* USER CODE BEGIN vFocTask */
  /* Infinite loop */
  TickType_t xLastWakeTime = xTaskGetTickCount();
  HAL_TIM_Base_Start_IT(&htim3);
  for(;;)
  {
    // TIM3中断中每10ms释放一次
    // 等待中断通知
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Foc任务运行
    Foc_Task();

    // 每10ms检查一次
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
  /* USER CODE END vFocTask */
}

/* USER CODE BEGIN Header_vNavigationTask */
/**
* @brief Function implementing the NavigationTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vNavigationTask */
void vNavigationTask(void *argument)
{
  /* USER CODE BEGIN vNavigationTask */
  /* Infinite loop */
  TickType_t xLastWakeTime = xTaskGetTickCount(); // 获取当前系统时间作为基准时间
  for(;;)
  {
    // 导航任务运行
    Navigation_Task();

    // 每100ms执行一次
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
  }
  /* USER CODE END vNavigationTask */
}

/* USER CODE BEGIN Header_vBuoyancyTask */
/**
* @brief Function implementing the BuoyancyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vBuoyancyTask */
void vBuoyancyTask(void *argument)
{
  /* USER CODE BEGIN vBuoyancyTask */
  /* Infinite loop */
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    // 水舱任务运行
    Buoyancy_Task();

    // 每50ms检查一次
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
  }
  /* USER CODE END vBuoyancyTask */
}

/* USER CODE BEGIN Header_vCommTask */
/**
* @brief Function implementing the CommTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vCommTask */
void vCommTask(void *argument)
{
  /* USER CODE BEGIN vCommTask */
  /* Infinite loop */

  // 任务挂起
  // vTaskSuspend(NULL);
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  for(;;)
  {
    // 通信任务运行
    Communication_Task();

    // 每500ms打印一次
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
  }
  /* USER CODE END vCommTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

