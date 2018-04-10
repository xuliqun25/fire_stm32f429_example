#include "MICO.h"
#include "user_uart.h"

#define os_uart_log(format, ...)  custom_log("uart_demo", format, ##__VA_ARGS__)

mico_uart_config_t user_uart_config =
{
	.baud_rate    = 115200,
	.data_width   = DATA_WIDTH_8BIT,
	.parity       = NO_PARITY,
	.stop_bits    = STOP_BITS_1,
	.flow_control = FLOW_CONTROL_DISABLED,
	.flags        = UART_WAKEUP_DISABLE,
};

volatile ring_buffer_t  	rx_buffer;//必须是全部变量，局部变量会出现问题!!!

OSStatus user_uart_init(void)
{
	uint8_t*       	rx_data;
	OSStatus 		err = kNoErr;

	rx_data = malloc(USER_UART_BUFF_LEN);
	if(rx_data == NULL)
		return kNoMemoryErr;

	memset(rx_data, 0, USER_UART_BUFF_LEN);
	ring_buffer_init( (ring_buffer_t *)&rx_buffer, (uint8_t *)rx_data, USER_UART_BUFF_LEN );
	err = MicoUartInitialize( USER_UART_PORT, &user_uart_config, (ring_buffer_t *)&rx_buffer );

	return err;
}

int application_start( void )
{
	uint32_t recv_len = 0, len = 0;
	OSStatus err = kNoErr;
	uint8_t uart_recv_buff[USER_UART_RECV_BUFF_LEN] = {0};

    cli_init();

	/* Output on debug serial port */
	os_uart_log( "user uart echo demo~" );

	user_uart_init();

	while(1)
	{
		memset(uart_recv_buff, 0, USER_UART_RECV_BUFF_LEN);

        err = MicoUartRecv(USER_UART_PORT, uart_recv_buff, 1, MICO_WAIT_FOREVER);
		require_noerr_action(err, EXIT, os_uart_log("MicoUartRecv() error, err = %d.", err));

		recv_len = strlen((char *)uart_recv_buff);
		if(recv_len == 0)
			continue;

		MicoUartSend(USER_UART_PORT, uart_recv_buff, recv_len);
	}

//    while(1)
//    {
//        len = MicoUartGetLengthInBuffer(USER_UART_PORT);
//
//        msleep(300);
//        os_uart_log("len = %d", len);
//    }

 EXIT:

	return kNoErr;
}



