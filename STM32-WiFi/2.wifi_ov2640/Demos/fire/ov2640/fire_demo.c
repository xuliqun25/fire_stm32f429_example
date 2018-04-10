/**
******************************************************************************
* @file    wifi_station.c
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   First MiCO application to say hello world!
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "MICO.h"
#include "SocketUtils.h"
#include "tcp_server.h"
#include "camera_data_queue.h"
#include "bsp_sdram.h"

#define fire_demo_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

//路由名
const char SSID_NAME[] = "BHLINK";
//路由密码
const char SSID_KEY[] = "cancore2015";


static mico_semaphore_t wait_sem = NULL;

static void micoNotify_ConnectFailedHandler(OSStatus err, void* inContext)
{
	fire_demo_log("join Wlan failed Err: %d", err);
}

static void micoNotify_WifiStatusHandler(WiFiEvent event,  void* inContext)
{
    switch (event)
	{
	 case NOTIFY_STATION_UP:
		fire_demo_log("Station up");
		mico_rtos_set_semaphore( &wait_sem );
		break;
	 case NOTIFY_STATION_DOWN:
		fire_demo_log("Station down");
		break;
	 default:
		break;
	}
}

OSStatus wifi_station(void)
{
    OSStatus err = kNoErr;
	network_InitTypeDef_adv_st  wNetConfigAdv={0};

    MicoInit();  //初始化网络协议栈

	/* Register user function when wlan connection status is changed */
	err = mico_system_notify_register( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler, NULL );
	require_noerr( err, exit );

	/* Register user function when wlan connection is faile in one attempt */
	err = mico_system_notify_register( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler, NULL );
	require_noerr( err, exit );

	cli_init();
	mico_rtos_init_semaphore( &wait_sem,1);

	/* Initialize wlan parameters */
	strcpy((char*)wNetConfigAdv.ap_info.ssid, SSID_NAME);          /* wlan ssid string */
	strcpy((char*)wNetConfigAdv.key, SSID_KEY);                 /* wlan key string or hex data in WEP mode */
	wNetConfigAdv.key_len = strlen(SSID_KEY);                   /* wlan key length */

	wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;          /* wlan security mode */
	wNetConfigAdv.ap_info.channel = 0;                            /* Select channel automatically */
    wNetConfigAdv.dhcpMode = DHCP_Client;                         /* Fetch Ip address from DHCP server */
	wNetConfigAdv.wifi_retry_interval = 100;                      /* Retry interval after a failure connection */

	/* Connect Now! */
	fire_demo_log("connecting to %s...", wNetConfigAdv.ap_info.ssid);
	micoWlanStartAdv(&wNetConfigAdv);

	/* Wait for wlan connection*/
	mico_rtos_get_semaphore( &wait_sem, MICO_WAIT_FOREVER );
	fire_demo_log( "wifi connected successful" );

 exit:
	return err;
}

extern CircularBuffer cam_circular_buff;

int application_start( void )
{
   camera_data * cambuf;

	OSStatus err = kNoErr;

    SDRAM_Init();

//    err = SDRAM_Test();
//    require_noerr( err, exit );

//    sdram_heap_init();

    err = wifi_station();
    require_noerr( err, exit );

    err = camera_queue_init();
    require_noerr( err, exit );
    
    
    cambuf = cbWrite(&cam_circular_buff);
    require_noerr( !cambuf, exit );	//cambuf=NULL时表示有错误
	
    err = open_camera((uint32_t *)cambuf->head, CAMERA_QUEUE_DATA_LEN);

    require_noerr( err, exit );

    /* Start camera thread */
//	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "camera server", camera_server_thread, 0x1000, NULL );
//	require_noerr_string( err, exit, "ERROR: Unable to start the camera server thread." );

    /* Start UDP server thread */
	err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "TCP_server", tcp_server_thread, 0x1000, NULL );
	require_noerr_string( err, exit, "ERROR: Unable to start the tcp server thread." );

    while(1)
    {
        mico_thread_sleep(1);
        fire_demo_log("camera capture frame counter： frame_count = %d", frame_counter);
        fire_demo_log("DMA capture frame counter：dma_start_counter= %d", dma_start_counter);
       
        frame_counter=0;
        dma_start_counter=0;
    }

 exit:
	mico_rtos_delete_thread(NULL);
	return err;
}
