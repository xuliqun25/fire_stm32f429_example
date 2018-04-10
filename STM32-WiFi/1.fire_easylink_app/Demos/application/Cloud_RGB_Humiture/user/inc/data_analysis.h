#ifndef _DATA_ANALYSIS_H_
#define _DATA_ANALYSIS_H_

#include "MicoFogCloudDef.h"

/*------串口数据帧的头尾字符定义--------*/
#define USER_UART_PACKET_HEAD1				(';')
#define USER_UART_PACKET_HEAD2				('2')
#define USER_UART_PACKET_TAIL_1 			(0x0D)
#define USER_UART_PACKET_TAIL_2 			(0x0A)

#define USER_UART_PACKET_FUNCTION_INDEX 	(2) //功能ID的数组下标,第三个字节
#define USER_UART_PACKET_MIN_PACKET_LEN		(8)

#define ASK_DATA_BUFF_LEN					(8)


/*------------函数定义-----------*/
extern uint16_t fcs_check(uint8_t *start, uint32_t len);
extern void uart_ask_data(void);
extern bool up_stream_data_analysis(void);

#endif