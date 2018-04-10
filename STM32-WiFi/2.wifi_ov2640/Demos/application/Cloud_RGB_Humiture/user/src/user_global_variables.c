#include "user_global_variables.h"


uint8_t user_uart_tx_buff[USER_UART_BUFF_LEN] = {0};	//串口发送数据缓冲数组
uint8_t user_uart_rx_buff[USER_UART_BUFF_LEN] = {0};	//串口接收数据缓冲数组


int32_t temperature = 0, humidity = 0, wind_velocity = 0, wind_direction = 0;//温度，湿度，风速，风向