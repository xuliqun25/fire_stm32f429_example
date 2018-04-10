#include "mico.h"
#include "fogcloud_down_thread.h"
#include "MICOAppDefine.h"
#include "MicoFogCloud.h"
#include "json_c/json.h"
#include "fire_rgb_led.h"
#include "fire_beep.h"
#include "fogcloud_up_thread.h"

#define user_log(M, ...) custom_log("USER_DOWNSTREAM", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER_DOWNSTREAM")

void user_downstream_thread(void* arg);


void get_json_beep(json_object *recv_json_object)
{
    uint8_t beep_ctr = 0;

    json_object_object_foreach(recv_json_object, key, val)
    {
        if(0 == strcmp(key, "beep_ctr"))
        {
            beep_ctr = json_object_get_int(val);
            beep_set(beep_ctr);
            ext_mod_data.beep = beep_ctr;
        }
    }

    return;
}

void get_json_uart(json_object *recv_json_object)
{
    const char* uart_str = NULL;

    json_object_object_foreach(recv_json_object, key, val)
    {
        if(0 == strcmp(key, "uart_data"))
        {
            uart_str = json_object_get_string(val);
            MicoUartSend(UART_FOR_APP, uart_str, strlen(uart_str));
            MicoUartSend(UART_FOR_APP, "\r\n", 2);  //Ä©Î²Ìí¼Ó»»ÐÐ
        }
    }

    return;
}

void get_json_RGB(json_object *recv_json_object)
{
    bool rgb_switch = false;
    uint8_t r = 0, g = 0, b = 0;

    json_object_object_foreach(recv_json_object, key, val)
    {
        if(0 == strcmp(key, "switch"))
        {
            rgb_switch = json_object_get_boolean(val);
        }else if(0 == strcmp(key, "R"))
        {
              r =  json_object_get_int(val);
        }else if(0 == strcmp(key, "G"))
        {
              g =  json_object_get_int(val);
        }else if(0 == strcmp(key, "B"))
        {
              b =  json_object_get_int(val);
        }

        ext_mod_data.RGB_LED_R = r;
        ext_mod_data.RGB_LED_G = g;
        ext_mod_data.RGB_LED_B = b;
    }

    if(rgb_switch == false)
    {
        close_reg_led();
    }else
    {
        set_rgb_data(r, g, b);
    }

    return;
}


void user_downstream_thread(void* arg)
{
    user_log_trace();
    OSStatus err = kUnknownErr;
    app_context_t *app_context = (app_context_t *)arg;
    fogcloud_msg_t *recv_msg = NULL;
    json_object *recv_json_object = NULL;
    const char *ctl_type = NULL;

    require(app_context, exit);

    /* thread loop to handle cloud message */
    while(1)
    {
        // check fogcloud connect status
        if(app_context->appStatus.fogcloudStatus.isCloudConnected == false)
        {
            mico_thread_msleep(50);
            continue;
        }

        /* get a msg pointer, points to the memory of a msg:
        * msg data format: recv_msg->data = <topic><data>
        */
        err = MiCOFogCloudMsgRecv(app_context, &recv_msg, FOGCLOUD_RECV_TIMOUT);
        if(kNoErr == err)
        {
            // debug log in MICO dubug uart
            user_log("Cloud => Module: topic[%d]=[%.*s]\tdata[%d]=[%.*s]....", recv_msg->topic_len, recv_msg->topic_len, recv_msg->data,recv_msg->data_len, recv_msg->data_len, recv_msg->data + recv_msg->topic_len);

            if(*(recv_msg->data + recv_msg->topic_len) != '{')
            {
                user_log("json format error!");
                continue;
            }

            // parse json data from the msg, get led control value
            recv_json_object = json_tokener_parse((const char*)(recv_msg->data + recv_msg->topic_len));
            if (NULL != recv_json_object)
            {
                json_object_object_foreach(recv_json_object, key, val)
                {
                    if(0 == strcmp(key, "data_type"))
                    {
                        ctl_type = json_object_get_string(val);

                        if(0 == strcmp(ctl_type, "beep_ctr"))
                        {
                             get_json_beep(recv_json_object);
                        }else if(0 == strcmp(ctl_type, "uart_data"))
                        {
                             get_json_uart(recv_json_object);
                        }else if(0 == strcmp(ctl_type, "RGB_LED"))
                        {
                             get_json_RGB(recv_json_object);
                        }else
                        {
                            user_log("JSON message error");
                        }
                    }

                }

                json_object_put(recv_json_object);          // free memory of json object
                recv_json_object = NULL;
            }


            if(NULL != recv_msg)   // NOTE: must free msg memory after been used.
            {
                free(recv_msg);
                recv_msg = NULL;
            }
        }
    }

 exit:
    user_log("ERROR: user_downstream_thread exit with err=%d", err);
}
