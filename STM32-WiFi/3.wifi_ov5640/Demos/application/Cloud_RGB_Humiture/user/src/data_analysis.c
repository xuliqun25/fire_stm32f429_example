#include "mico.h"
#include "data_analysis.h"
#include "user_uart.h"
#include "user_global_variables.h"

#define user_log(M, ...) custom_log("DATA_ANALYSIS", M, ##__VA_ARGS__)

/*------------函数定义-----------*/
uint16_t fcs_check(uint8_t *start, uint32_t len);
void uart_ask_data(void);

uint16_t fcs_check(uint8_t *start, uint32_t len)
{
	uint32_t sum = 0, i = 0;
		
	for(i = 0; i < len; i = i + 2)
	{
		sum += ((uint16_t)(*(start + i)) << 8);
	}
	
	for(i = 1; i < len; i =i + 2)
	{
		sum += (uint16_t)(*(start + i));
	}
	
	return (uint16_t)sum;
}

void uart_ask_data(void)
{
	uint8_t ask_data_buff[ASK_DATA_BUFF_LEN] = {0};
	uint16_t Sum = 0;
	
	ask_data_buff[0] = ';';   //start 1
	ask_data_buff[1] = '2';   //start 2
    ask_data_buff[2] = 'A';   //function 
	ask_data_buff[3] = ASK_DATA_BUFF_LEN;  //lenth
	
    //Calculate the checksum
	Sum = fcs_check(ask_data_buff, ASK_DATA_BUFF_LEN - 4);
        
    ask_data_buff[ASK_DATA_BUFF_LEN - 4] = (uint8_t)(Sum >> 8);    //And check high
	ask_data_buff[ASK_DATA_BUFF_LEN - 3] = (uint8_t)(Sum & 0xff);  //And check low 
	ask_data_buff[ASK_DATA_BUFF_LEN - 2] = 0x0D; //end 1
    ask_data_buff[ASK_DATA_BUFF_LEN - 1] = 0x0A; //end 2
	
	MicoUartSend(USER_UART_PORT, ask_data_buff, ASK_DATA_BUFF_LEN);  //send command
	
	return;
}


