#ifndef __USER_UART_H_
#define __USER_UART_H_

#include "mico.h"

/*------------宏定义-----------*/
#define USER_UART_PORT					MICO_UART_1	//串口1是DEGUG口，用于和气象站的stm32f103通信
													//串口2是扩展板上面的用户串口			
#define USER_UART_RING_BUFFER_LENGTH 		(512)

/*------------函数定义-----------*/
extern OSStatus user_uart_init(void);
extern void user_uart_read_data(void);


#endif
