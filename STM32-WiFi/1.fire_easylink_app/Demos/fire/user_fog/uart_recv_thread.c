#include "MICO.h"
#include "uart_recv_thread.h"
#include "MICOAppDefine.h"
#include "MicoFogCloud.h"

#define user_uart_log(format, ...)  custom_log("uart_demo", format, ##__VA_ARGS__)

void user_uart_recv_thread(void* arg);

mico_uart_config_t user_uart_config =
{
	.baud_rate    = USER_UART_BAUD_RATE,
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

void user_uart_recv_thread(void* arg)
{
	OSStatus err = kUnknownErr;
    uint32_t recv_len = 0, recv_len_before = 0, recv_len_next = 0;
	app_context_t *app_context = (app_context_t *)arg;
    uint8_t *uart_recv_buff;
    json_object *send_json_object = NULL;
    const char *upload_data = NULL;


    uart_recv_buff = malloc(USER_UART_BUFF_LEN);
    require(app_context, exit);


	user_uart_log("------------uart data recv thread start------------");
	require(app_context, exit);

    err = user_uart_init(); //用户串口初始化
    require_noerr_action( err, exit, user_uart_log("ERROR: Unable to Init user uart") );

	while(1)
	{
        memset(uart_recv_buff, 0, USER_UART_RECV_BUFF_LEN);

        if((recv_len_before = MicoUartGetLengthInBuffer(USER_UART_PORT)) == 0)
        {
            mico_thread_msleep(USER_UART_RECV_TIMEOUT);
            continue;
        }

        mico_thread_msleep(10);

        recv_len_next = MicoUartGetLengthInBuffer(USER_UART_PORT);

        if((recv_len_next > recv_len_before))
        {
            continue;
        }
        else if((recv_len_next < recv_len_before))
        {
            user_uart_log("ringbuff is full!!");   //底层唤醒缓冲区有bug
        }

        err = MicoUartRecv(USER_UART_PORT, uart_recv_buff, recv_len_next, 0);
		if(err != kNoErr)
            continue;

		recv_len = strlen((char *)uart_recv_buff);
		if(recv_len == 0)
			continue;

        if(app_context->appStatus.fogcloudStatus.isCloudConnected == FALSE)
        {
            user_uart_log("disconnect fogclod, ingore uart recv data");
            continue;
        }


        // create json object to format upload data
        send_json_object = json_object_new_object();
        require_action_string(send_json_object, exit, err = kNoMemoryErr, "create json object error!");

        json_object_object_add(send_json_object, "data_type", json_object_new_string("uart_data"));
        json_object_object_add(send_json_object, "uart_data", json_object_new_string((const char *)uart_recv_buff));

        upload_data = json_object_to_json_string(send_json_object);
        require_action_string(upload_data, exit, err = kNoMemoryErr, "create upload data string error!");

        if(app_context->appStatus.fogcloudStatus.isCloudConnected == true)
        {
            MiCOFogCloudMsgSend(app_context, "uart", (unsigned char *)upload_data, strlen((const char *)upload_data));
        }
	}

 exit:
	if(kNoErr != err)
	{
		user_uart_log("ERROR: user_upstream_thread exit with err=%d", err);
	}
	mico_rtos_delete_thread(NULL);  // delete current thread

	return;
}


