# STM32F103 串口 DMA+中断 收发功能说明

## 功能概述

本项目实现了基于 STM32 HAL 库的串口通信功能，支持以下特性：
- ✅ **DMA 发送**：硬件自动传输，不占用 CPU
- ✅ **DMA 接收**：循环缓冲模式，支持连续数据流
- ✅ **IDLE 中断**：检测总线空闲，实现不定长数据帧接收
- ✅ **RXNE 中断**：单字节接收中断
- ✅ **回调机制**：用户友好的数据处理接口

## 硬件配置

- **USART**: USART1
- **TX**: PA9 (复用推挽输出)
- **RX**: PA10 (浮空输入)
- **DMA TX**: DMA1 Channel 4
- **DMA RX**: DMA1 Channel 5
- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无

## API 接口

### 1. 初始化函数

```c
void UART_DMA_Receive_IT_Init(void);
```
**功能**: 初始化 DMA 接收和 IDLE 中断，启动循环接收  
**调用时机**: 在 `main()` 函数的初始化部分调用  
**示例**:
```c
int main(void)
{
    // ... 其他初始化 ...
    MX_USART1_UART_Init();
    UART_DMA_Receive_IT_Init();  // 启动 DMA 接收
    // ...
}
```

### 2. 发送函数

```c
HAL_StatusTypeDef UART_SendData(uint8_t *pData, uint16_t Size);
```
**参数**:
- `pData`: 发送数据缓冲区指针
- `Size`: 数据长度

**返回值**: 
- `HAL_OK`: 发送成功
- `HAL_BUSY`: 上一次发送未完成

**示例**:
```c
uint8_t tx_data[] = "Hello World";
UART_SendData(tx_data, sizeof(tx_data));
```

### 3. 接收回调函数

```c
void UART_RxCallback(uint8_t *data, uint16_t size);
```
**参数**:
- `data`: 接收到的数据指针
- `size`: 数据长度

**说明**: 此函数在中断中调用，应保持快速执行。复杂处理建议在主循环中进行。

**示例**:
```c
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    // 原样返回（Echo）
    UART_SendData(data, size);
    
    // 或者处理特定协议
    if (data[0] == 0xAA) {
        // 处理命令
    }
}
```

## 使用示例

### 示例 1: Echo 测试

```c
// 在 main_user.c 或其他文件中实现
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    // 接收到的数据原样返回
    UART_SendData(data, size);
}
```

### 示例 2: 命令解析

```c
#define CMD_LED_ON  0x01
#define CMD_LED_OFF 0x02

void UART_RxCallback(uint8_t *data, uint16_t size)
{
    if (size >= 2 && data[0] == 0xAA)  // 命令头
    {
        switch(data[1])
        {
            case CMD_LED_ON:
                // 打开 LED
                break;
            case CMD_LED_OFF:
                // 关闭 LED
                break;
        }
        
        // 回复确认
        uint8_t ack[3] = {0xCC, data[1], 0x01};
        UART_SendData(ack, 3);
    }
}
```

### 示例 3: 主循环处理模式

```c
// 全局变量
volatile uint8_t g_data_ready = 0;
volatile uint8_t g_rx_buffer[256];
volatile uint16_t g_rx_len = 0;

// 回调函数（快速设置标志）
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    g_data_ready = 1;
    memcpy((void*)g_rx_buffer, (void*)data, size);
    g_rx_len = size;
}

// 主循环
while (1)
{
    if (g_data_ready)
    {
        g_data_ready = 0;
        
        // 处理接收到的数据
        ProcessData((uint8_t*)g_rx_buffer, g_rx_len);
    }
}
```

## 工作原理

### DMA 接收流程

1. **初始化**: `UART_DMA_Receive_IT_Init()` 启动 DMA 循环接收
2. **数据接收**: DMA 自动将 USART_DR 的数据搬运到内存缓冲区
3. **IDLE 检测**: 当总线出现空闲（一帧数据结束），触发 IDLE 中断
4. **计算长度**: 通过 DMA 剩余计数器计算接收到的数据长度
5. **回调通知**: 调用 `UART_RxCallback()` 处理数据
6. **重启 DMA**: 重置 DMA 指针，继续接收下一帧

### DMA 发送流程

1. **调用发送**: `UART_SendData()` 启动 DMA 发送
2. **硬件传输**: DMA 自动将内存数据搬运到 USART_DR
3. **完成中断**: 发送完成触发 DMA TC 中断
4. **清除标志**: 中断中清除完成标志，允许下次发送

## 注意事项

### 1. 中断优先级

当前配置中：
- USART1 中断优先级：0（最高）
- DMA1 Channel4/5 中断优先级：由 HAL 库默认配置

如需修改，可在 `MX_USART1_UART_Init()` 的 `HAL_NVIC_SetPriority()` 中调整。

### 2. 缓冲区大小

DMA 接收缓冲区大小定义为 `UART_RX_BUFFER_SIZE` (默认 256 字节):
```c
#define UART_RX_BUFFER_SIZE 256
```
根据实际应用调整，避免溢出。

### 3. 线程安全

`UART_SendData()` 使用了简单的忙检查机制，在多任务环境中可能需要添加互斥锁：

```c
// 简单改进示例
if (uart_tx_complete == 0)
{
    // 等待上次发送完成（带超时）
    uint32_t start = HAL_GetTick();
    while (uart_tx_complete == 0 && (HAL_GetTick() - start < 100));
    
    if (uart_tx_complete == 0)
        return HAL_TIMEOUT;
}
```

### 4. IDLE 中断兼容性

IDLE 中断使用了直接寄存器操作，因为 HAL 库未提供标准宏：
```c
__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);  // 使能 IDLE 中断
```

### 5. 调试技巧

如果无法进入中断，检查以下几点：
1. 全局中断是否使能：`__enable_irq()`
2. NVIC 是否正确配置：`HAL_NVIC_EnableIRQ(USART1_IRQn)`
3. 外设中断是否使能：`__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE)`
4. 是否有数据接收（可用示波器或逻辑分析仪验证）

## 常见问题

### Q1: 为什么收不到数据？
- 检查 RX 引脚连接（PA10）
- 检查波特率是否匹配（115200）
- 确认共地良好
- 使用串口助手发送数据测试

### Q2: 为什么会进入 HardFault？
- 检查数组是否越界
- 确认 DMA 缓冲区地址有效
- 检查堆栈大小配置

### Q3: 如何同时支持多种接收模式？
- DMA 模式适合大数据流
- 中断模式适合小数据量
- 可根据实际需求选择，不建议同时使用

## 文件结构

```
Core/
├── Inc/
│   ├── usart.h          # USART 接口声明
│   └── main.h           # 主头文件
├── Src/
│   ├── usart.c          # USART 实现（DMA+ 中断）
│   ├── stm32f1xx_it.c   # 中断服务函数
│   ├── main.c           # 系统初始化
│   └── main_user.c      # 用户应用代码（示例）
└── Makefile
```

## 修改历史

- 2026-03-21: 初始版本，支持 DMA 收发 + IDLE 中断
- 增加回调机制，方便用户扩展
- 添加完整示例代码

## 参考资料

- STM32F103 Reference Manual
- STM32CubeF1 HAL Library Documentation
- UART Idle Interrupt + DMA Reception Application Note
