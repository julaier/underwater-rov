# STM32H723 水下机器人固件读码总结

对应当前 `STM32H723ZGT6_ROV-RTOS` 工程。文里的源码位置均以固件工程根目录为起点。时间以源码配置为准，周期和动作时长是设定值或按配置计算的近似值。

## 1. 上电后怎么跑

```text
main()
  → 初始化 HAL、系统时钟、GPIO、DMA、定时器、ADC 和各串口
  → Gamepad_Init() 和 BSP_JY901_Init() 开始挂接收
  → Water_Tank_Init() 让前后水舱排空，建立空端零点
  → 启动 ADC3 进水检测、GPS 接收和两路无刷推进器的 FOC
  → osKernelInitialize() 初始化内核
  → MX_FREERTOS_Init() 创建任务、队列和互斥锁
  → osKernelStart() 开始调度
```

`main()` 有两处显式延时，分别是 **500 ms** 和 **50 ms**。还要加上各项初始化和水舱排空所用的时间。水舱排空流程要等**前、后两个水舱都判定为空**才返回。如果一直等不到空端，程序就可能卡在启动阶段。此时已开启的串口、步进定时器和限位中断仍可工作，五个业务任务要等调度器启动才开始跑。

源码入口 `Core/Src/main.c`，水舱归零在 `UserCode/APP/WaterTank.c` 的 `Draining_Test()`。

## 2. 五个任务的优先级、周期和职责

下表按优先级从高到低排列。五个任务都创建成功时，`FocTask` 首先运行。它进入延时后，调度器从剩下的就绪任务中继续挑优先级最高的。

| 任务 | 优先级 | 栈大小 | 外层目标周期 | 实际做什么 |
| --- | --- | ---: | ---: | --- |
| `FocTask` | High | 4096 B | **10 ms（0.01 秒）** | 入口启动 TIM3。之后反复调用 `Gamepad_Control()`，用处理后的 `leftX` 和 `rightY` 分别更新 1 号和 2 号无刷推进器的目标转速。 |
| `BuoyancyTask` | AboveNormal | 3072 B | **50 ms（0.05 秒）**，内部等队列 | 从水舱队列接收按键，再下达前舱或后舱进水、排水 1 mL 的指令。 |
| `NavigationTask` | Normal | 4096 B | **100 ms（0.1 秒）** | 调 `JY901_Task()`，有新数据时解析并更新 `roll`、`pitch`、`yaw`。 |
| `CommTask` | BelowNormal | 4096 B | **500 ms（0.5 秒）** | 读取 `gps_data`，把经纬度格式化成一行文字，调用 `HAL_UART_Transmit()` 经 USART6 发出。 |
| `SystemTask` | Low | 2048 B | **20 ms（0.02 秒）** | 检查手柄更新标志并发送按键消息，检查进水，解析 GPS，更新前、后水舱状态和估算水量。 |

- CubeMX 生成的 `osThreadNew()` 是 CMSIS-RTOS2 接口，适配层最终调用 FreeRTOS 的任务创建函数。任务各有自己的栈，也可以通过句柄被其他代码引用。
- `vTaskDelayUntil()` 用上一轮的**计划唤醒时刻**推算下一轮。实际开始执行还要看当前有无更高优先级任务、中断和其他阻塞。
- `BuoyancyTask` 在空队列上会阻塞等待。外层的 50 ms 只是周期设置，收到消息才会进一步判断要不要驱动水舱。
- `FocTask` 负责给目标速度。下方 TIM1 和 TIM8 的中断负责执行底层 FOC 控制环。

任务属性和延时位于 `Core/Src/freertos.c`，各任务调用的业务分别位于 `UserCode/APP/user_task.c`。

## 3. 定时器做什么

| 定时器 | 输出、触发和回调 | 时间 |
| --- | --- | --- |
| **TIM1** | CH1～CH3 输出 2 号无刷推进器的三相 PWM。更新事件触发 ADC2，更新中断调用 `Foc_Loop(2)`。 | 约 **40 µs** 一轮，约 **25 kHz** |
| **TIM8** | CH1～CH3 输出 1 号无刷推进器的三相 PWM。更新事件触发 ADC1，更新中断调用 `Foc_Loop(1)`。 | 约 **40 µs** 一轮，约 **25 kHz** |
| **TIM2** | CH1 给前舱步进驱动器发 STEP 脉冲，定步模式下更新中断累计步数。 | 前舱动作时运行，当前默认约 **3200 脉冲/秒** |
| **TIM5** | CH1 给后舱步进驱动器发 STEP 脉冲，定步模式下更新中断累计步数。 | 后舱动作时运行，当前默认约 **3200 脉冲/秒** |
| **TIM3** | 由 `vFocTask()` 启动。回调累计 10 次更新中断后，把调平标志置 1。 | **10 ms** 进一次中断，**100 ms** 置一次标志 |
| **SysTick** | Cortex-M 系统滴答。先更新 HAL 时基，调度器启动后，同一次中断也推进 FreeRTOS 的 tick。 | **1 ms** 一次 |

TIM3 的 10 ms 来自 `PSC = 274`、`ARR = 9999`。275 MHz 时钟先除以 275，得到 1 MHz，再从 0 数到 9999，合计 10,000 次计数。

### 步进电机怎么算频率和步数

TIM2、TIM5 的预分频 `PSC = 0`。由 `Motor_Set()` 按本次设定的脉冲频率算出 ARR。

```text
ARR = 275,000,000 / 目标脉冲频率 - 1
CCR1 = ARR / 2
```

硬件计数器 `CNT` 从 0 数到 ARR，合计 `ARR + 1` 个计数，走完一个 PWM 周期。在本工程的向上计数、PWM 模式 1、高电平有效配置下，`CNT < CCR1` 时输出高电平，到了 CCR1 切为低电平。因此 ARR 决定周期，CCR1 决定高电平宽度，占空比约 50%。

定步运动时，更新回调执行 `current_step++`，达到 `target_step` 就停止更新中断和 PWM。假设上回已记 100 步，本回读到 120 步，水舱任务就把新增 **20 步**换算成进水或排水量。按默认换算，**3200 步约对应 1 mL**，以默认脉冲频率走完约需 **1 秒**。步数来自定时器脉冲，电机卡住或丢步时计数仍可能增加。

`stepper_init()` 会计算加速、匀速和减速各占多少步。当前按步数调频的 `get_step_speed()` 调用被注释了，所以一次水舱动作按传给 `Motor_Set()` 的频率发脉冲。

源码位置 `Core/Src/tim.c`、`UserCode/MID/H_Tmc2209.c`、`UserCode/BSP/bsp_interrupt.c`。

## 4. 中断入口、HAL 和业务回调怎么连

TIM2 的更新中断沿下面这条调用链执行。

```text
TIM2 到达更新点
  → TIM2_IRQHandler()
  → HAL_TIM_IRQHandler(&htim2)
  → HAL 检查并清除更新标志
  → HAL_TIM_PeriodElapsedCallback(htim)
  → 识别为前舱电机，定步模式下计步，到目标后停
```

入口在 `Core/Src/stm32h7xx_it.c`，应用层的定时器和串口回调在 `UserCode/BSP/bsp_interrupt.c`。HAL 提供了带 `__weak` 的默认回调，工程里的同名实现会在链接时被选用。进入工程回调时仍在同一次中断里。GPS 解析和水舱状态更新则在任务中执行。

工程开启了 **15 条外设 IRQ**。包括上一节的 5 条定时器中断，以及下表的 10 条。表中入口名都带 `_IRQHandler` 后缀。

| IRQ 入口 | 触发后做什么 |
| --- | --- |
| **USART1** | **9600 波特，JY901S 姿态传感器。** 用 `HAL_UARTEx_ReceiveToIdle_DMA()` 收数据。收到一段数据后出现串口空闲等接收事件，`HAL_UARTEx_RxEventCallback()` 记录长度、置 `data_ready`、重新开启接收。 |
| **USART2** | **115200 波特，预留 Wi-Fi 串口。** 已有初始化和 IRQ 入口。GPS 的 Wi-Fi 发送函数已定义，当前任务流程未调用，也未挂起 USART2 的中断接收。 |
| **USART3** | **115200 波特，GPS。** 收满 1 字节进入 `HAL_UART_RxCpltCallback()`，把 `bsp_gps_rx_byte` 写进软件 FIFO，再开启下一字节接收。出错时，`HAL_UART_ErrorCallback()` 中止本轮 GPS 接收并重新挂接收。 |
| **USART6** | **9600 波特，手柄。** 收满 1 字节后，回调调用 `Gamepad_RxCallback()` 拼接和校验。整帧正确后更新摇杆、按键和 `isUpdated`。 |
| **USART10** | **9600 波特，调试串口。** IRQ 入口已配置。调试函数通过 `HAL_UART_Transmit()` 发送，当前应用未挂起 USART10 的中断接收。 |
| **EXTI2** | PG2，前舱满端微动开关，设置前舱的 `event_full`。 |
| **EXTI3** | PG3，前舱空端微动开关，设置前舱的 `event_empty`。 |
| **EXTI4** | PG4，后舱满端微动开关，设置后舱的 `event_full`。 |
| **EXTI9_5** | 本工程在这个共享入口处理 PG5，后舱空端微动开关，设置后舱的 `event_empty`。 |
| **DMA1_Stream0** | 处理 USART1 接收 DMA 的传输事件，和 USART1 串口空闲中断一起服务 JY901S 的接收。 |

USART3 和 USART6 的接收流程中，`HAL_UART_Receive_IT(..., 1)` 会先设置好缓冲地址、接收计数和函数指针 `huart->RxISR`。下一个字节到达后，`HAL_UART_IRQHandler()` 通过这个指针调用 `UART_RxISR_8BIT()`。它把字节写进缓冲区，把剩余计数从 1 减到 0，再调完成回调。回调处理完后重新挂接收。

除了外设 IRQ，内核的 `SVC_Handler()` 用于进入首个任务，`PendSV_Handler()` 用于 FreeRTOS 任务切换。工程也保留 `NMI`、`HardFault`、`MemManage`、`BusFault`、`UsageFault` 和 `DebugMon` 等异常入口。

## 5. 微动开关怎么让水舱停机

**微动开关确认的是水舱机构有没有走到空端或满端。** 步进电机带动机构移动，到位时机构压到开关，STM32 收到引脚变化。

```text
以前舱满端为例

进水机构走到满端，压到微动开关
  → PG2 由高变低，产生 EXTI2
  → HAL_GPIO_EXTI_Callback() 设置 tank_front.event_full = 1
  → SystemTask 调用 Water_Tank_Update_Handler()
  → 处理满端事件，将状态设为 TANK_FULL，再调用 Motor_Stop()
```

四个限位输入都配置了**上拉、下降沿触发**。开关维持被压住时，单凭持续低电平不会反复触发。空端事件的处理还会将水舱设成 `TANK_EMPTY`，把估算水量归零。平时靠脉冲数估算行程，走到端点时可以靠微动开关确认到位。目标步数先到时由定时器回调停机，限位信号先到时由水舱逻辑处理停机。

源码位置 `Core/Src/gpio.c` 和 `UserCode/APP/WaterTank.c`。

## 6. ADC 怎么采样，DMA 把数据放哪里

| 外设 | 触发方式 | DMA 和缓冲区 | 数据用途 |
| --- | --- | --- | --- |
| **ADC1** | TIM8 更新事件 | DMA1 Stream3 循环写入 `adc1_buf[]` | `Foc_Loop(1)` 读取两路模拟采样，作为 1 号推进器的电流反馈。 |
| **ADC2** | TIM1 更新事件 | DMA1 Stream1 循环写入 `adc2_buf[]` | `Foc_Loop(2)` 读取两路模拟采样，作为 2 号推进器的电流反馈。 |
| **ADC3** | 软件启动后连续采样 | DMA1 Stream2 循环写入 `adc_value[]` | `Water_Check()` 读取 PF4（通道 9）、PC1（通道 11）的进水检测电压。 |
| **USART1 RX** | 串口收到字节 | DMA1 Stream0 普通模式写入 `JY_RxBuffer[]` | JY901S 的原始字节，任务解析成姿态数据。 |

- ADC1/2 的同一个定时器更新事件既启动 ADC 转换，也触发 FOC 中断。FOC 读的是 DMA **已经写好的样本**，本轮转换还没写完时可能读到上一轮的值。
- ADC3 的输入是模拟电压。如果探头利用水的导电性，检测电路会把导电变化转换成电压变化，ADC 再将电压转换成数字。水舱水量由步进脉冲另行估算。
- `Water_Check()` 按 `ADC 值 / 4095 × 3.3` 换算电压，然后低通滤波。同一路滤波电压连续 **10 次高于 0.5 V** 后锁存进水。按 20 ms 的任务节拍，确认窗口约 **0.2 秒**，滤波和调度也会影响实际响应。
- 当前确认进水后，`SystemTask` 调用前舱排到空端的函数，随后进入 `while (1)`。之后这个任务的 GPS 解析和日常水舱状态更新停止执行。

源码位置 `Core/Src/adc.c`、`Core/Src/dma.c`、`UserCode/foc_lib/src/user_foc_hal.c`、`UserCode/APP/WaterADC.c`。

## 7. 消息队列、GPS FIFO 和互斥锁

| 对象 | 用法 |
| --- | --- |
| **`WaterTankQueueHandle`** | 容量 **10 条**，每条一个 **`uint16_t`（2 字节）**。发送端为 `SystemTask`，接收端为 `BuoyancyTask`。当前 `buttons` 接收变量也是 `uint16_t`。 |
| **`GamepadQueueHandle`** | 容量 **1 条 × 1 字节**。当前业务只创建了它。摇杆数据由 `FocTask` 直接读取手柄解析结果，按键消息走水舱队列。 |
| **GPS 软件 FIFO** | USART3 中断按到达顺序写入原始字节，`SystemTask` 按顺序取出并拼接 NMEA 报文。 |
| **RMC 缓存** | 保留最近一条完整的 RMC 文本。应用层复制出来解析经纬度，新 RMC 会覆盖旧的。缓存就绪标志表示已有报文，定位是否有效和是否过期需要另行判断，当前应用层未做这两项检查。 |
| **`DebugUsartMutexHandle`** | `Debug_USART_Show()` 拿锁、发 USART10、再释放锁。当前任务里的调用已经注释。 |

发送水舱消息时用 `xQueueSend(..., 0)`，队列满就立即返回，当前代码没检查发送结果。水舱任务用 `xQueueReceive(..., portMAX_DELAY)`，队列空时可以阻塞等消息。收到的值为 **1、2、4、8** 时，分别执行**前舱进水、前舱排水、后舱进水、后舱排水**。

源码位置 `Core/Src/freertos.c`、`UserCode/APP/user_task.c`、`UserCode/BSP/bsp_gps.c`。

## 8. 记住三条链路就能重新读回来

```text
拨摇杆
  USART6 收完整手柄帧
  → FocTask 更新目标转速
  → TIM1 / TIM8 的中断执行 FOC

按 A 键
  USART6 收完整手柄帧
  → SystemTask 把按键送到水舱队列
  → BuoyancyTask 下达前舱进水 1 mL 指令
  → TIM2 发 STEP 脉冲并在中断中计步
  → SystemTask 估算水量，处理限位或完成后的状态

GPS 模块输出经纬度
  USART3 逐字节接收并写进 FIFO
  → SystemTask 拼接 RMC 并更新 gps_data
  → CommTask 把坐标经 USART6 TX 发出去
```

JY901S 姿态链路目前能跑到姿态解析。TIM3 也会定期置调平标志。代码里虽然有按角度调前、后水舱的 `Drone_Balance_Control()`，当前任务流程没有调用它，自动调平尚未接通。
