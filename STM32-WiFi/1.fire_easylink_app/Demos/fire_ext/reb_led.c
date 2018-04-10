#include "mico.h"
#include "fire_rgb_led.h"
#include "fire_beep.h"
#include "sensor\RHEOSTAT\rheostat.h"

#define pwm_log(M, ...)    custom_log("PWM", M, ##__VA_ARGS__)


//OSStatus application_start(void)
//{
//    OSStatus err = kNoErr;
//    uint16_t rheostat_data = 0;
//
//    MicoPwmInitialize(fire_RGB_LED_R, 1000, 1);
//    MicoPwmStart(MICO_PWM_R);
//    MicoPwmInitialize(fire_RGB_LED_G, 1000, 1);
//    MicoPwmStart(MICO_PWM_G);
//    MicoPwmInitialize(fire_RGB_LED_B, 1000, 1);
//    MicoPwmStart(MICO_PWM_B);
//
//
//    while(1)
//    {
//        MicoPwmStop(MICO_PWM_R);
//        MicoPwmStop(MICO_PWM_G);
//        MicoPwmStop(MICO_PWM_B);
//
//        err = rheostat_read(&rheostat_data);
//        require_noerr_action( err, exit, pwm_log("read rheostat data") );
//        pwm_log("rheostat data: %d", rheostat_data);
//
//        mico_thread_msleep(1000);
//    }
//
// exit:
//    return 0;
//}



//OSStatus application_start(void)
//{
//    OSStatus status = kNoErr;
//
//    int i = 0;
//
//    while(1)
//    {
//        beep_set(BEEP_ON);
//
//        mico_thread_msleep(20);
//
//        beep_set(BEEP_OFF);
//
//        mico_thread_sleep(5);
//    }
//    close_reg_led();
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, i, i);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    //----------------------
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, 0, 0);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, i, 0);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, 0, i);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    //---------------
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, i, 0);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(i, 0, i);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    for(i = 0; i < 0xFF; i ++)
//    {
//        set_rgb_data(0, i, i);
//        mico_thread_msleep(20);
//        pwm_log("i = %d", i);
//    }
//
//    while(1)
//    {
//        mico_thread_msleep(100);
//    }
//
//    return status;
//}