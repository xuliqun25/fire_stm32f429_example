#include "mico.h"
#include "user_global_variables.h"
#include "MiCOFogCloudDef.h"
#include "MICOAppDefine.h"
#include "json_c/json_object.h"
#include "MicoFogCloud.h"
#include "user_fogcloud_down.h"
#include "data_analysis.h"

#define user_log(M, ...) custom_log("USER_DOWNSTREAM", M, ##__VA_ARGS__)

/*------------函数定义-----------*/
void user_downstream_thread(void* arg);


//从云端接收数据并处理后发送数据包到stm32f103的串口
void user_downstream_thread(void* arg)
{
	OSStatus err = kUnknownErr;
	app_context_t *app_context = (app_context_t *)arg;
	fogcloud_msg_t *recv_msg = NULL;

	
	user_log("-----------------downstream_thread start------------");	
	require(app_context, exit);
	
	while(1)
	{
		// check fogcloud connect status
		if(app_context->appStatus.fogcloudStatus.isCloudConnected == FALSE)
		{
			mico_thread_msleep(10);
//			user_log("Module is not connected to the fogcloud!!!");
			continue;
		}
		
		//1.从云端接收数据(快速轮询)
		err = MiCOFogCloudMsgRecv(app_context, &recv_msg, USER_FOGCLOUD_DOWN_TIMWOUT); 
		if(kNoErr != err)//如果从云端接收数据错误则打印log信息
		{
			if(kUnderrunErr != err)//将不是缓冲区无数据的错误打印出来
				user_log("MiCOFogCloudMsgRecv err! err = %d\n", err);
			continue;
		}
		
		user_log("Cloud => Module: topic[%d]=[%.*s]\tdata[%d]=[%.*s]",recv_msg->topic_len, recv_msg->topic_len, recv_msg->data, 
				 recv_msg->data_len, recv_msg->data_len, recv_msg->data + recv_msg->topic_len);
		
		
		//2.解析接收到的数据
		//3.转换格式

		
		//4.发送数据包到stm32f103

		
		//5.释放云消息内存空间
		if(NULL != recv_msg)
		{
			free(recv_msg);
			recv_msg = NULL;
		}
	}
	
exit:
	if(kNoErr != err)
	{
		user_log("ERROR: user_downstream_thread exit with err=%d", err);
	}	
	mico_rtos_delete_thread(NULL);  // delete current thread
	
	return;
}
