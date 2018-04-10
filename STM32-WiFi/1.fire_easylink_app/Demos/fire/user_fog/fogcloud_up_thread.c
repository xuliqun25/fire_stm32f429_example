#include "mico.h"
#include "MiCOFogCloudDef.h"
#include "MICOAppDefine.h"
#include "json_c/json_object.h"
#include "MicoFogCloud.h"
#include "fire_rgb_led.h"
#include "fire_beep.h"
#include "sensor\RHEOSTAT\rheostat.h"
#include "sensor\DHT11\DHT11.h"
#include "uart_recv_thread.h"
#include "json_c/json.h"
#include "fogcloud_up_thread.h"

#define fogcloud_up_log(M, ...) custom_log("FOG_UP_DATA", M, ##__VA_ARGS__)


ext_module_data ext_mod_data;


void user_upstream_thread(void* arg);


//外设初始化
void ext_module_init(void)
{
    OSStatus err = kNoErr;

    close_reg_led();        //RGB LED初始化

    beep_init();            //BEEP 蜂鸣器
    beep_set(BEEP_OFF);

    err = DHT11_Init();     //DHT11
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init DHT11") );

    err = rheostat_init();  //滑动变阻器
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init rheostat") );

    memset(&ext_mod_data, 0, sizeof(ext_mod_data));
    ext_mod_data.beep = false;
    beep_set(ext_mod_data.beep);

    ext_mod_data.RGB_LED_R = 0x00;
    ext_mod_data.RGB_LED_G = 0x00;
    ext_mod_data.RGB_LED_B = 0x00;

    set_rgb_data(ext_mod_data.RGB_LED_R, ext_mod_data.RGB_LED_G, ext_mod_data.RGB_LED_B);

 exit:
    return;
}

OSStatus ext_moudule_read(void)
{
    OSStatus err = kUnknownErr;
    uint16_t rheostat_data = 0;

    err = DHT11_Read_Data(&ext_mod_data.temperature, &ext_mod_data.humidity);
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Can't Read DTH11 Data") );
    fogcloud_up_log("DHT11  T: %3.1fC  H: %3.1f%%", (float)ext_mod_data.temperature, (float)ext_mod_data.humidity);

    err = rheostat_read(&rheostat_data);
    require_noerr_action( err, exit, fogcloud_up_log("read rheostat data") );

    ext_mod_data.adc = (float)(rheostat_data) / (float)(0xFFF) * (float)(3.3);
//    fogcloud_up_log("rheostat data: %d, adc data: %1.5f V", rheostat_data, ext_mod_data.adc);

 exit:
    return err;
}

OSStatus ext_moudule_upload(app_context_t *app_context)
{
    json_object *send_json_object = NULL;
    const char *upload_data = NULL;
    OSStatus err = kUnknownErr;

    // create json object to format upload data
    send_json_object = json_object_new_object();
    require_action_string(send_json_object, exit, err = kNoMemoryErr, "create json object error!");

    json_object_object_add(send_json_object, "data_type", json_object_new_string("sensor"));
    json_object_object_add(send_json_object, "temperature", json_object_new_int(ext_mod_data.temperature));
    json_object_object_add(send_json_object, "humidity", json_object_new_int(ext_mod_data.humidity));
    json_object_object_add(send_json_object, "adc", json_object_new_double(ext_mod_data.adc));
    json_object_object_add(send_json_object, "beep", json_object_new_boolean(ext_mod_data.beep));
    json_object_object_add(send_json_object, "RGB_LED_R", json_object_new_int(ext_mod_data.RGB_LED_R));
    json_object_object_add(send_json_object, "RGB_LED_G", json_object_new_int(ext_mod_data.RGB_LED_G));
    json_object_object_add(send_json_object, "RGB_LED_B", json_object_new_int(ext_mod_data.RGB_LED_B));


    upload_data = json_object_to_json_string(send_json_object);
    require_action_string(upload_data, exit, err = kNoMemoryErr, "create upload data string error!");

    if(app_context->appStatus.fogcloudStatus.isCloudConnected == true)
    {
        MiCOFogCloudMsgSend(app_context, "sensor", (unsigned char *)upload_data, strlen(upload_data));
    }

    // free json object memory
    json_object_put(send_json_object);
    send_json_object = NULL;

 exit:
    return err;
}

void user_upstream_thread(void* arg)
{
	OSStatus err = kUnknownErr;
	app_context_t *app_context = (app_context_t *)arg;

	fogcloud_up_log("------------upstream_thread start------------");
	require(app_context, exit);

    ext_module_init();

	while(1)
	{
		if(app_context->appStatus.fogcloudStatus.isCloudConnected == FALSE)
		{
			mico_thread_msleep(500);
			fogcloud_up_log("Module is not connected to the fogcloud!!!");
			continue;
		}

        err = ext_moudule_read();
        if(err != kNoErr)
        {
            mico_thread_sleep(1);
            continue;
        }

        ext_moudule_upload(app_context);

        mico_thread_sleep(1);
	}

 exit:
	if(kNoErr != err)
	{
		fogcloud_up_log("ERROR: user_upstream_thread exit with err=%d", err);
	}
	mico_rtos_delete_thread(NULL);  // delete current thread

	return;
}