#ifndef __FOGCLOUD_UP_THREAD_H_
#define __FOGCLOUD_UP_THREAD_H_

typedef struct _ext_module_data
{
    uint8_t temperature; //�¶�
    uint8_t humidity;    //ʪ��
    float   adc;         //adcֵ
    bool    beep;        //������
    uint8_t RGB_LED_R;   //��ɫLED-R
    uint8_t RGB_LED_G;   //��ɫLED-G
    uint8_t RGB_LED_B;   //��ɫLED-B
}ext_module_data;

extern void user_upstream_thread(void* arg);

extern ext_module_data ext_mod_data;

#endif

