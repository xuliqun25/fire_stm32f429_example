#include "MICO.h"
#include "sensor\light_adc\light_sensor.h"

#define ext_light_sensor_log(M, ...) custom_log("EXT", M, ##__VA_ARGS__)

int application_start( void )
{
    OSStatus err = kNoErr;
    uint16_t light_sensor_data = 0;

    /*init Light sensor*/
    err = light_sensor_init();
    require_noerr_action( err, exit, ext_light_sensor_log("ERROR: Unable to Init light sensor") );

    cli_init();

    while(1)
    {
        err = light_sensor_read(&light_sensor_data);
        require_noerr_action( err, exit, ext_light_sensor_log("ERROR: Can't light sensor read data") );
        ext_light_sensor_log("light data: %d", light_sensor_data);
        mico_thread_msleep(100);
    }
 exit:
    return err;
}