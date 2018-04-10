#include "MICO.h"
#include "sensor\RHEOSTAT\rheostat.h"

#define ext_rheostat_log(M, ...) custom_log("EXT", M, ##__VA_ARGS__)


int application_start( void )
{
    OSStatus err = kNoErr;
    uint16_t rheostat_data = 0;

    cli_init();

    /*init Light sensor*/
    err = rheostat_init();
    require_noerr_action( err, exit, ext_rheostat_log("ERROR: Unable to Init rheostat") );

    while(1)
    {
        err = rheostat_read(&rheostat_data);
        require_noerr_action( err, exit, ext_rheostat_log("read rheostat data") );
        ext_rheostat_log("rheostat data: %d", rheostat_data);
        mico_thread_msleep(1000);
    }

 exit:
    return err;
}