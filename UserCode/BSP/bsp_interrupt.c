#include "bsp_interrupt.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"

extern osThreadId_t FocTaskHandle;

// 定时器中断的回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim == &htim3) //10ms进来一次
  {
    
    // 
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // vTaskNotifyGiveFromISR(FocTaskHandle, &xHigherPriorityTaskWoken);
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);   
    static uint16_t cnt = 0;
    cnt++;
    if(cnt >= 10)
    {
      Scheduler_Set_DroneBalanceControlFlag(); // 100ms的时候，把需要调用平衡函数置1，也就是触发一次
      cnt = 0;
    }
  }
  else if(htim == motor_tim[1] || htim == motor_tim[2])// 定时器2和定时器5
  {
    for (int i = 1; i <= 2; i++)
    {
        if (htim == motor_tim[i])
        {
            if (Motor[i].mode == Constant_step && Motor[i].target_step)// 如果是定步模式且有目标步数
            {
                Motor[i].current_step++;// 现在的步数多执行一步
                if (Motor[i].current_step >= Motor[i].target_step)//如果要多执行一步大于或等于目标步数
                {
                    // Motor[i].current_step = 0;
                    Motor[i].target_step = 0;
                    // HAL_GPIO_WritePin(en_ports[i], en_pins[i], GPIO_PIN_RESET);
                    HAL_TIM_Base_Stop_IT(motor_tim[i]);// 关闭该定时器
                    HAL_TIM_PWM_Stop(motor_tim[i], motor_channel[i]);// 关闭步进电机的pwm
                }
                // else
                // {
                //     Motor[i].hz = get_step_speed(Motor[i].current_step, Motor[i].steps, Motor[i].velocity);
                //     uint16_t arr = (TIMER_CLK_HZ / Motor[i].hz) - 1;
                //     __HAL_TIM_SET_AUTORELOAD(motor_tim[i], arr);
                //     __HAL_TIM_SET_COMPARE(motor_tim[i], motor_channel[i], arr / 2);
                //     __HAL_TIM_SET_COUNTER(motor_tim[i], 0);
                // }
            }
            break;
        }
    }
  }

  if(htim->Instance == TIM1)// 如果是无刷电机的foc定时器
  {
      Foc_Loop(2);// 执行电机 2 的一轮 FOC 控制。
      // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET);
  }
  if(htim->Instance == TIM8)
  {
      Foc_Loop(1);// 执行电机 1 的一轮 FOC 控制。
      // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET);
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == jy901_uart.huart->Instance)
    {
        jy901_uart.data_ready = 1; // 
        jy901_uart.received_byte = Size; //

        HAL_UARTEx_ReceiveToIdle_DMA(jy901_uart.huart, JY_RxBuffer, JY_Buffer_Size);// 此函数用于接收jy901串口的数据
        
    }
}

/**
 * @brief 
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &gamepad_huart)
    {
        #if 0
        if(rx_data == 0x02)
            Water_Tank_Draining(&tank_front, 1.0f);
        else if(rx_data == 0x01)
            Water_Tank_Filling(&tank_front, 1.0f);
        else if(rx_data == 0x03)
            Water_Tank_Filling(&tank_rear, 1.0f);
        else if(rx_data == 0x04)
            Water_Tank_Draining(&tank_rear, 1.0f);
        //OLED_ShowNum(64, 32, 0, 1, OLED_8X16);
        HAL_UART_Receive_IT(&huart3, &rx_data, 1); //
        #endif
        // 
        Gamepad_RxCallback(gamepad_rxByte);// 把串口刚收到的那个字节，交给手柄解析函数处理。

        // 
        HAL_UART_Receive_IT(huart, &gamepad_rxByte, 1);// 让串口通过中断接收下一个字节，并把收到的字节存进 gamepad_rxByte
    }

    if (huart->Instance == bsp_gps_state.huart->Instance)
    {
        /*  */
        bsp_gps_state.received_byte = 1U;
        BSP_GPS_WriteFifo(bsp_gps_rx_byte);// 将接收到的单个字节写入软件FIFO。
        HAL_UART_Receive_IT(bsp_gps_state.huart, &bsp_gps_rx_byte, 1U);// // 让串口通过中断接收下一个字节，并把收到的字节存进 gamepad_rxByte
    }
}

/**
 * @brief 
 * @param huart 
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == bsp_gps_state.huart->Instance)// 如果是gps接收失误，就抛弃这一次数据，重新接收
    {
        BSP_GPS_UART_RxCallback();
    }
}
