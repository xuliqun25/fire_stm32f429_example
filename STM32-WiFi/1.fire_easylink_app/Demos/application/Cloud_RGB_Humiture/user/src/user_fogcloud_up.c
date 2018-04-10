#include "mico.h"
#include "user_global_variables.h"
#include "MiCOFogCloudDef.h"
#include "MICOAppDefine.h"
#include "json_c/json_object.h"
#include "MicoFogCloud.h"
#include "user_fogcloud_down.h"
#include "user_uart.h"
#include "data_analysis.h"


#define user_log(M, ...) custom_log("USER_UPSTREAM", M, ##__VA_ARGS__)

/*------------函数定义-----------*/
void user_upstream_thread(void* arg);
void send_data_to_fogcloud(app_context_t *arg);

#define IMG_INFO_LEN	(1024 - 300)
uint8_t img_info[IMG_INFO_LEN] = {0};

void send_data_to_fogcloud(app_context_t *arg)
{
	app_context_t *app_context = (app_context_t *)arg;
	json_object *send_json_object = NULL;
	const char *upload_data = NULL; //记得释放
	int32_t i = 0;
	
	send_json_object = json_object_new_object();
	if(NULL == send_json_object)
	{
		user_log("create json object error!");
		return;
	}
	
	for(i = 0; i < IMG_INFO_LEN; i ++)
	{
		img_info[i] = 'A';
	}
	
	json_object_object_add(send_json_object, "img_name", json_object_new_string("test.img")); 
	json_object_object_add(send_json_object, "group_num", json_object_new_int(0)); 
	json_object_object_add(send_json_object, "member_total", json_object_new_int(0)); 
	json_object_object_add(send_json_object, "member_num", json_object_new_int(0)); 
	json_object_object_add(send_json_object, "ELF", json_object_new_boolean(false));
	json_object_object_add(send_json_object, "img_mem_len", json_object_new_int(1024));
	json_object_object_add(send_json_object, "img_mem_info", json_object_new_string((const char *)img_info));
	
	upload_data = json_object_to_json_string(send_json_object);
	if(NULL == upload_data)
	{
		user_log("create upload data string error!");
		goto exit;
	}else{
		// check fogcloud connect status
		if(app_context->appStatus.fogcloudStatus.isCloudConnected)
		{
			// upload data string to fogcloud, the seconde param(NULL) means send to defalut topic: '<device_id>/out'
			MiCOFogCloudMsgSend(app_context, NULL, (unsigned char*)upload_data, strlen(upload_data));
			user_log("upload data success!****************************");
		}
	}
	
 exit:
	json_object_put(send_json_object);// free json object memory
	send_json_object = NULL;
	
	return;
}

//阻塞性接收stm32f103发来的数据并解析，将解析结果转换格式后发送到云端
void user_upstream_thread(void* arg)
{
	OSStatus err = kUnknownErr;
	app_context_t *app_context = (app_context_t *)arg;
	
	user_log("-----------------upstream_thread start------------");
	require(app_context, exit);
		
	while(1)
	{		
		if(app_context->appStatus.fogcloudStatus.isCloudConnected == FALSE)
		{
			mico_thread_msleep(200);
			user_log("Module is not connected to the fogcloud!!!");
			continue;
		}
		
		//send_data_to_fogcloud(arg);
		mico_thread_msleep(5);
	}
	
 exit:
	if(kNoErr != err)
	{
		user_log("ERROR: user_upstream_thread exit with err=%d", err);
	}	
	mico_rtos_delete_thread(NULL);  // delete current thread
	
	return;
}
