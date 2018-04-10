#include "MICO.h"
#include "MICOAppDefine.h"
#include "SocketUtils.h"
#include "tcp_server.h"
#include "camera_data_queue.h"
#include "camera_server.h"
#include "bsp_sdram.h"
#include "malloc.h"
#include "fogcloud_down_thread.h"
#include "fogcloud_up_thread.h"
#include "uart_recv_thread.h"

#define fire_demo_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

OSStatus user_main( app_context_t * const app_context );


void user_ov2640_thread( void *arg )
{
    OSStatus err = kNoErr;

    SDRAM_Init();

//    err = SDRAM_Test();
//    require_noerr( err, exit );

    sdram_heap_init();

    err = camera_queue_init();
    require_noerr( err, exit );

    err = open_camera((uint32_t *)cam_global_buf, CAMERA_QUEUE_DATA_LEN);
    require_noerr_string( err, exit, "ERROR: can not ov2640." );

    /* Start camera thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "camera server", camera_server_thread, 0x2000, NULL );
	require_noerr_string( err, exit, "ERROR: Unable to start the camera server thread." );

    /* Start UDP server thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "TCP_server", tcp_server_thread, 0x2000, NULL );
	require_noerr_string( err, exit, "ERROR: Unable to start the tcp server thread." );

    while(1)
    {
        mico_thread_sleep(1);
        fire_demo_log("frame_count = %d", frame_counter);
        frame_counter = 0;
    }

 exit:
	mico_rtos_delete_thread(NULL);
	return;
}


OSStatus user_main( app_context_t * const app_context )
{
    OSStatus err = kNoErr;
#if 1
    /*Start fog downstream thread*/
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "fogclod downstream", user_downstream_thread, 0x2000, app_context );
	require_noerr_string( err, exit, "ERROR: Unable to start the fogclod downstream thread." );

    /* Start fog upstream thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY - 1, "fogclod upstream", user_upstream_thread, 0x2000, app_context );
	require_noerr_string( err, exit, "ERROR: Unable to start the fogclod upstream thread." );

    /* Start uart_recv thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "user uart recv", user_uart_recv_thread, 0x2000, app_context );
	require_noerr_string( err, exit, "ERROR: Unable to start the user uart recv thread" );
#else
    /* Start ov2640 thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "camera server", user_ov2640_thread, 0x1000, NULL );
	require_noerr_string( err, exit, "ERROR: Unable to start the camera server thread." );
#endif
    while(1)
    {
        mico_thread_sleep(1);
    }

 exit:
	mico_rtos_delete_thread(NULL);
	return err;
}


/* JSON��ʽ����

//����״̬�ϱ�json��ʽ
{
	"data_type":"sensor",
	"temperature":23,		//�¶�	��Χ0-50,��λ:�� ��������:����
	"humidity":100,			//ʪ�� 	��Χ:20-90  ��λ:%RH ��������:����
	"adc":2.5,				//ADC������ѹ ��������float ��Χ0-3.3 ��λ:V
	"beep":0,				//������״̬ 0�� 1�� ��������:����
	"RGB_LED_R":30,			//R:0-255 ��������:����
	"RGB_LED_G":30,			//G:0-255 ��������:����
	"RGB_LED_B":30,			//B:0-255 ��������:����
}
//����״̬�ϱ�json��ʽ
{
	"data_type":"uart_data",
	"uart_data":"AAAAAAA",	//��������,��������:�ַ���
}

//�������ݿ���json��ʽ
{
	"data_type":"beep_ctr",
	"beep_ctr":0           //0�� 1�� ��������:����
}
//�������ݿ���json��ʽ
{
	"data_type":"uart_data",
	"uart_dat":"AAAAAA"	  //��������:�ַ���
}
//�������ݿ���json��ʽ
{
	"data_type":"RGB_LED",
	"switch":true, 		//��������:bool
	"R":30,				//��������:����  ��Χ:0-255
	"G":30,
	"B":30
}
*/