#ifndef _USER_GLOBAL_VARIABLES_
#define _USER_GLOBAL_VARIABLES_

#include "mico.h"

/*------------宏定义-----------*/
#define USER_UART_BUFF_LEN					(256)

/*------------函数定义-----------*/
extern uint8_t user_uart_tx_buff[USER_UART_BUFF_LEN];	//串口发送数据缓冲数组
extern uint8_t user_uart_rx_buff[USER_UART_BUFF_LEN];	//串口接收数据缓冲数组
extern int32_t temperature, humidity, wind_velocity, wind_direction;//温度，湿度，风速，风向

#endif

