#ifndef __UART_RECV_H_
#define __UART_RECV_H_

#define USER_UART_BUFF_LEN			(1024*5)
#define USER_UART_RECV_BUFF_LEN		(1024*5)
#define USER_UART_RECV_TIMEOUT		(50)

#define USER_UART_PORT				UART_FOR_APP
#define USER_UART_BAUD_RATE         (115200)

extern void user_uart_recv_thread(void* arg);

#endif

