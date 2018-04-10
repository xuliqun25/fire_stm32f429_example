#include "mico.h"
#include "MicoFogCloud.h"
#include "user_uart.h"
#include "user_global_variables.h"
#include "user_fogcloud_down.h"
#include "user_fogcloud_up.h"
#include "data_analysis.h"

#define user_log(M, ...) custom_log("USER_MAIN", M, ##__VA_ARGS__)

/*------------变量定义-----------*/
static mico_thread_t user_downstrem_thread_handle = NULL;
static mico_thread_t user_upstream_thread_handle = NULL;

mico_Context_t* app_context_test = NULL;

OSStatus user_main( app_context_t * const app_context )
{
	OSStatus err = kUnknownErr;

	require(app_context, exit);
	
	user_log("-----------------user_main start------------");
	
	/*------硬件初始化------*/
//	user_uart_init();//串口初始化
	
//	err = mico_rtos_create_thread(&user_downstrem_thread_handle, MICO_APPLICATION_PRIORITY, "user_downstream", user_downstream_thread, STACK_SIZE_USER_DOWNSTREAM_THREAD, app_context);
//	require_noerr_action(err, exit, user_log("ERROR: create user_downstream thread failed!"));
//	
//	err = mico_rtos_create_thread(&user_upstream_thread_handle, MICO_APPLICATION_PRIORITY, "user_upstream", user_upstream_thread, STACK_SIZE_USER_UPSTREAM_THREAD, app_context);
//	require_noerr_action(err, exit, user_log("ERROR: create user_uptream thread failed!"));
	
//	err = mico_rtos_create_thread(&user_lcd_refresh_thread_handle, MICO_APPLICATION_PRIORITY, "user_upstream", user_lcd_refresh_thread, STACK_SIZE_USER_LCD_REFRESH_THREAD, app_context);
//	require_noerr_action(err, exit, user_log("ERROR: create user_uptream thread failed!"));
	
	while(1)
	{
		if(app_context->appStatus.fogcloudStatus.isCloudConnected == FALSE)
		{
			mico_thread_msleep(10);
			continue;
		}
	}  
	
 exit:
	if(kNoErr != err)
	{
		user_log("ERROR: user_main thread exit with err = %d", err);
	}
	mico_rtos_delete_thread(NULL);  // delete current thread
	
	return err;
}


