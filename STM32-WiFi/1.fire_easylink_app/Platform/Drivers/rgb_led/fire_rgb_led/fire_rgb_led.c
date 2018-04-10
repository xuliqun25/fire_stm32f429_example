#include "mico_platform.h"
#include "fire_rgb_led.h"

#define fire_reb_led_log(M, ...)    custom_log("FIRE_GEB_LED", M, ##__VA_ARGS__)

#ifndef fire_RGB_LED_R
#define fire_RGB_LED_R MICO_PWM_NONE
#endif

#ifndef fire_RGB_LED_G
#define fire_RGB_LED_B MICO_PWM_NONE
#endif

#ifndef fire_RGB_LED_B
#define fire_RGB_LED_B MICO_PWM_NONE
#endif


void close_reg_led(void);
void set_rgb_data(uint8_t red, uint8_t green, uint8_t blue);


//关闭RGB灯
void close_reg_led(void)
{
    MicoPwmInitialize(fire_RGB_LED_R, RGB_LED_FREQUENCY, 100);
    MicoPwmStart(MICO_PWM_R);
    MicoPwmStop(MICO_PWM_R);
    MicoPwmInitialize(fire_RGB_LED_G, RGB_LED_FREQUENCY, 100);
    MicoPwmStart(MICO_PWM_G);
    MicoPwmStop(MICO_PWM_G);
    MicoPwmInitialize(fire_RGB_LED_B, RGB_LED_FREQUENCY, 100);
    MicoPwmStart(MICO_PWM_B);
    MicoPwmStop(MICO_PWM_B);
}

//设置RGB值 R:0-0xFF  G:0-0xFF  B:0-0xFF
void set_rgb_data(uint8_t red, uint8_t green, uint8_t blue)
{
    MicoPwmInitialize(fire_RGB_LED_R, RGB_LED_FREQUENCY, (float)(100 - red*100/255));
    MicoPwmStart(MICO_PWM_R);
    MicoPwmInitialize(fire_RGB_LED_G, RGB_LED_FREQUENCY, (float)(100 - green*100/255));
    MicoPwmStart(MICO_PWM_G);
    MicoPwmInitialize(fire_RGB_LED_B, RGB_LED_FREQUENCY, (float)(100 - blue*100/255));
    MicoPwmStart(MICO_PWM_B);
}

////测试代码
//OSStatus application_start(void)
//{
//    OSStatus status = kNoErr;
//
//    int i = 0;
//
//    close_reg_led();
//
//    while(1)
//    {
//       mico_thread_msleep(20);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, i, i);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    //----------------------
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, 0, 0);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, i, 0);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, 0, i);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    //---------------
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, i, 0);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, 0, i);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, i, i);
//        mico_thread_msleep(20);
//        fire_reb_led_log("i = %d", i);
//    }
//
//    while(1)
//    {
//        mico_thread_msleep(100);
//    }
//
//    return status;
//}