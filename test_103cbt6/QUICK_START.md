# 快速开始 - 串口 DMA+中断 使用指南

## 1. 硬件连接

```
STM32F103        USB 转 TTL
PA9 (TX)   <-->  RX
PA10 (RX)  <-->  TX
GND        <-->  GND
```

## 2. 编译烧录

```bash
# 编译项目
make clean
make all

# 烧录程序（使用你的烧录工具）
```

## 3. 串口调试

打开串口调试助手，配置：
- **端口**: COMx (根据实际设备)
- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: None
- **换行符**: \r\n

## 4. 测试步骤

### 测试 1: Echo 功能

1. 修改 `Core/Src/main_user.c` 中的回调函数：

```c
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    // 原样返回
    UART_SendData(data, size);
}
```

2. 编译烧录
3. 在串口助手中发送任意字符
4. 应该能看到返回的相同字符

### 测试 2: LED 控制

1. 修改回调函数：

```c
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    if (size == 1)
    {
        switch(data[0])
        {
            case '1':
                // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 点亮 LED
                break;
            case '0':
                // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // 熄灭 LED
                break;
            default:
                // 回复帮助信息
                char help[] = "Send '1' for ON, '0' for OFF\r\n";
                UART_SendData((uint8_t*)help, strlen(help));
                break;
        }
    }
}
```

2. 编译烧录
3. 发送 '1' 或 '0' 控制 LED

### 测试 3: 数据帧接收

发送不定长数据帧，观察回调函数的 `size` 参数变化：

```c
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    char info[50];
    sprintf(info, "Received %d bytes\r\n", size);
    UART_SendData((uint8_t*)info, strlen(info));
    
    // 打印接收到的数据（HEX 格式）
    for(int i = 0; i < size; i++)
    {
        sprintf(info, "%02X ", data[i]);
        UART_SendData((uint8_t*)info, strlen(info));
    }
    UART_SendData((uint8_t*)"\r\n", 2);
}
```

## 5. 常见问题排查

### 问题 1: 无输出
- 检查系统时钟配置是否正确（72MHz）
- 确认 USART1 时钟已使能
- 检查 PA9/PA10 引脚配置

### 问题 2: 乱码
- 确认波特率匹配（115200）
- 检查晶振频率配置
- 确保共地良好

### 问题 3: 无法进入中断
- 检查是否调用了 `UART_DMA_Receive_IT_Init()`
- 确认 NVIC 中断已使能
- 检查全局中断状态

## 6. 性能优化建议

### 降低中断频率
对于高速数据流，可适当增大数据包间隔或使用定时器辅助检测。

### 双缓冲区
如需连续接收多帧数据，可使用双缓冲机制：

```c
uint8_t rx_buffer[2][UART_RX_BUFFER_SIZE];
volatile uint8_t current_buffer = 0;

void UART_RxCallback(uint8_t *data, uint16_t size)
{
    // 处理当前缓冲区数据
    ProcessData(rx_buffer[current_buffer], size);
    
    // 切换缓冲区
    current_buffer = !current_buffer;
    
    // 重启 DMA 到新缓冲区
    HAL_UART_Receive_DMA(&huart1, rx_buffer[current_buffer], UART_RX_BUFFER_SIZE);
}
```

## 7. 扩展功能

### 添加超时检测
使用定时器检测接收超时，处理不完整的数据帧。

### 协议解析
实现 Modbus、自定义协议等。

### DMA 循环模式
修改为真正的循环缓冲，适合音频流等连续数据。

---

**祝开发顺利！** 🚀
