#ifndef __FOGCLOUD_UP_THREAD_H_
#define __FOGCLOUD_UP_THREAD_H_

typedef struct _ext_module_data
{
    uint8_t temperature; //温度
    uint8_t humidity;    //湿度
    float   adc;         //adc值
    bool    beep;        //蜂鸣器
    uint8_t RGB_LED_R;   //三色LED-R
    uint8_t RGB_LED_G;   //三色LED-G
    uint8_t RGB_LED_B;   //三色LED-B
}ext_module_data;

extern void user_upstream_thread(void* arg);

extern ext_module_data ext_mod_data;

#endif

