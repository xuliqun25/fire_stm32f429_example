#include "MICO.h"
#include "sensor\DHT11\DHT11.h"

#define ext_temp_hum_log(M, ...) custom_log("EXT", M, ##__VA_ARGS__)

int application_start( void )
{
    OSStatus err = kNoErr;
    uint8_t dht11_temp_data = 0;
    uint8_t dht11_hum_data = 0;

    err = DHT11_Init();
    require_noerr_action( err, exit, ext_temp_hum_log("ERROR: Unable to Init DHT11") );

    while(1)
    {
        /* DHT11 Read Data Delay must >= 1s*/
        mico_thread_sleep(1);
        err = DHT11_Read_Data(&dht11_temp_data, &dht11_hum_data);
        require_noerr_action( err, exit, ext_temp_hum_log("ERROR: Can't Read Data") );
        ext_temp_hum_log("DHT11  T: %3.1fC  H: %3.1f%%", (float)dht11_temp_data, (float)dht11_hum_data);
    }
 exit:
    return err;
}
