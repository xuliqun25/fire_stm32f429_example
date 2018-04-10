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

#define wifi_station_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

static char tcp_remote_ip[16] = "172.21.10.1"; /*remote ip address*/
static int tcp_remote_port = 3000;               /*remote port*/
static mico_semaphore_t wait_sem = NULL;


static void micoNotify_ConnectFailedHandler(OSStatus err, void* inContext)
{
	wifi_station_log("join Wlan failed Err: %d", err);
}

static void micoNotify_WifiStatusHandler(WiFiEvent event,  void* inContext)
{
    switch (event)
	{
	 case NOTIFY_STATION_UP:
		wifi_station_log("Station up");
		mico_rtos_set_semaphore( &wait_sem );
		break;
	 case NOTIFY_STATION_DOWN:
		wifi_station_log("Station down");
		break;
	 default:
		break;
	}
}


/*when client connected wlan success,create socket*/
void tcp_client_thread( void *arg )
{
	UNUSED_PARAMETER(arg);

	OSStatus err;
	struct sockaddr_t addr;
	int tcp_fd = -1;
	char *buf = NULL;

    while(1)
    {
        tcp_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        require_action(IsValidSocket( tcp_fd ), exit, err = kNoResourcesErr );

        addr.s_ip = inet_addr( tcp_remote_ip );
        addr.s_port = tcp_remote_port;

        wifi_station_log( "Connecting to server: ip=%s  port=%d! tcp_fd = %d\r\n", tcp_remote_ip,tcp_remote_port, tcp_fd);
        err = connect( tcp_fd, &addr, sizeof( addr ) );
        require_noerr( err, exit );
        wifi_station_log( "Connect success!" );

        buf = (char*)malloc( 1024 );
        require_action( buf, exit, err = kNoMemoryErr );

        memset(buf, 1, 1024);

        while(1)
        {
            if((err = SocketSend(tcp_fd, (const uint8_t *)buf, 1024)) != kNoErr)
                break;
//            mico_thread_msleep(2);
        }

     exit:
        if( err != kNoErr )
            wifi_station_log( "TCP client thread  err: %d", err );

        if( buf != NULL )
        {
            free( buf );
            buf = NULL;
        }
        SocketClose( &tcp_fd) ;
        mico_thread_sleep(1);
    }

	mico_rtos_delete_thread( NULL );
}

int application_start( void )
{
	OSStatus err = kNoErr;
	network_InitTypeDef_adv_st  wNetConfigAdv={0};

	MicoInit( );

	/* Register user function when wlan connection status is changed */
	err = mico_system_notify_register( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler, NULL );
	require_noerr( err, exit );

	/* Register user function when wlan connection is faile in one attempt */
	err = mico_system_notify_register( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler, NULL );
	require_noerr( err, exit );

	cli_init();
	mico_rtos_init_semaphore( &wait_sem,1);

	/* Initialize wlan parameters */
	strcpy((char*)wNetConfigAdv.ap_info.ssid, "111111");          /* wlan ssid string */
	strcpy((char*)wNetConfigAdv.key, "12345678");                 /* wlan key string or hex data in WEP mode */
	wNetConfigAdv.key_len = strlen("12345678");                   /* wlan key length */

	wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;          /* wlan security mode */
	wNetConfigAdv.ap_info.channel = 1;                            /* Select channel automatically */
    wNetConfigAdv.dhcpMode = DHCP_Client;                         /* Fetch Ip address from DHCP server */
	wNetConfigAdv.wifi_retry_interval = 100;                      /* Retry interval after a failure connection */

	/* Connect Now! */
	wifi_station_log("connecting to %s...", wNetConfigAdv.ap_info.ssid);
	micoWlanStartAdv(&wNetConfigAdv);

	/* Wait for wlan connection*/
	mico_rtos_get_semaphore( &wait_sem, MICO_WAIT_FOREVER );
	wifi_station_log( "wifi connected successful" );

	/* Start TCP client thread */
	err = mico_rtos_create_thread(NULL, 5, "TCP_client", tcp_client_thread, 0x1000, NULL );
	require_noerr_string( err, exit, "ERROR: Unable to start the tcp client thread." );

 exit:
	mico_rtos_delete_thread(NULL);
	return err;
}





