#include <stdio.h>
#include "mico.h"
#include "user_uart.h"
#include "RingBufferUtils.h"
#include "data_analysis.h"
#include "user_global_variables.h"

#define user_log(M, ...) custom_log("USER_UART", M, ##__VA_ARGS__)

/*------------变量定义-----------*/
static ring_buffer_t  user_rx_buffer; //环形缓冲区必须是全局变量，请注意！
static uint8_t        user_rx_data[USER_UART_RING_BUFFER_LENGTH] = {0};


/*------------函数定义-----------*/
OSStatus user_uart_init(void);
void user_uart_read_data(void);


OSStatus user_uart_init(void)
{
	mico_uart_config_t uart_config;
	
	memset(&uart_config, 0, sizeof(mico_uart_config_t));
	
	uart_config.baud_rate    	= 115200;
	uart_config.data_width   	= DATA_WIDTH_8BIT;
	uart_config.parity       	= NO_PARITY;
	uart_config.stop_bits    	= STOP_BITS_1;
	uart_config.flow_control 	= FLOW_CONTROL_DISABLED;
	uart_config.flags 			= UART_WAKEUP_DISABLE;
	
	ring_buffer_init ((ring_buffer_t *)&user_rx_buffer, (uint8_t *)user_rx_data, USER_UART_RING_BUFFER_LENGTH);
	MicoUartInitialize(USER_UART_PORT, &uart_config, (ring_buffer_t *)&user_rx_buffer);
	
	return kNoErr;
}


OSStatus user_uart_read_one_Byte(mico_uart_t uart, uint32_t timeout, uint8_t *read_one_Byte)
{
	OSStatus err = kNoErr;
	
	err = MicoUartRecv(USER_UART_PORT, read_one_Byte, 1, timeout);
	if(err != kNoErr)
	{
		user_log("MicoUartRecv() error, err = %d.", err);
	}
		
	return err;
}

//从串口缓冲区读取数据并解析
void user_uart_read_data(void)
{
	OSStatus err = kNoErr;
	uint8_t read_Byte = 0;
	uint16_t my_check_sum = 0;
	uint32_t index = 0, packet_len = 0;
	
	while(1)
	{
		memset(user_uart_rx_buff, 0, USER_UART_BUFF_LEN);//清除
		read_Byte = 0;
		my_check_sum = 0;
		index = 0;
		packet_len = 0;
		
		//从串口读取第一字节
		err = user_uart_read_one_Byte(USER_UART_PORT, MICO_WAIT_FOREVER, &read_Byte);
		if(err != kNoErr)
			continue;
		if(read_Byte != USER_UART_PACKET_HEAD1) //头1 --> ';'
			continue;
		user_uart_rx_buff[index ++] = read_Byte;
		
		//从串口读取第二字节
		err = user_uart_read_one_Byte(USER_UART_PORT, MICO_WAIT_FOREVER, &read_Byte);
		if(err != kNoErr)
			continue;
		if(read_Byte != USER_UART_PACKET_HEAD2) //头2 --> '2'
			continue;
		user_uart_rx_buff[index ++] = read_Byte;
		
		//从串口读取第三字节
		err = user_uart_read_one_Byte(USER_UART_PORT, MICO_WAIT_FOREVER, &read_Byte);
		if(err != kNoErr)
			continue;
//		if((read_Byte != 'W') && (read_Byte != 'P') && (read_Byte != 'T') && (read_Byte != 'R')) //功能码 --> 'W' || 'P' || 'T' || 'R'
//			continue;
		user_uart_rx_buff[index ++] = read_Byte;
		
		//从串口读取第四字节
		err = user_uart_read_one_Byte(USER_UART_PORT, MICO_WAIT_FOREVER, &read_Byte);//读取长度字节
		if(err != kNoErr)	
			continue;
		if(read_Byte < USER_UART_PACKET_MIN_PACKET_LEN) //最小长度是8字节
			continue;
		packet_len = user_uart_rx_buff[index ++] = read_Byte;
		
		//从串口读取正文数据
		MicoUartRecv(USER_UART_PORT, user_uart_rx_buff + index, packet_len - 4, MICO_WAIT_FOREVER);
		if(err != kNoErr)
		{
			user_log("MicoUartRecv() error, err = %d.", err);
		}
		
		my_check_sum = fcs_check(user_uart_rx_buff, packet_len - 4);
		
		//判断checksum和尾部2字节
		if(user_uart_rx_buff[packet_len - 2] != USER_UART_PACKET_TAIL_1)
		{
			user_log("packet tail error, tail1 = 0x%04X", user_uart_rx_buff[packet_len - 2]);
			continue;
		}
		if(user_uart_rx_buff[packet_len - 1] != USER_UART_PACKET_TAIL_2)
		{
			user_log("packet tail error, tail2 = 0x%04X", user_uart_rx_buff[packet_len - 1]);
			continue;
		}
		if(my_check_sum != (user_uart_rx_buff[packet_len - 4] << 8 | user_uart_rx_buff[packet_len - 3]))
		{
			user_log("fcs_check() error, my_check_sum = 0x%04X.", my_check_sum);
			continue;
		}else
		{
			break; //所有校验都通过，跳出循环
		}
	}
}
