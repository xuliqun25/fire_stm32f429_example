#include "MICO.h"
#include "ov2640_init.h"
#include "camera_data_queue.h"
#include "camera_server.h"

#define camera_server_log(M, ...) custom_log("camera_server", M, ##__VA_ARGS__)


void camera_server_thread( void *arg )
{
    OSStatus err = kNoErr;

    err = mico_rtos_init_semaphore(&camera_copy_data_sem, 1);
    require_noerr( err, exit );

    while(1)
    {
        err = mico_rtos_get_semaphore(&camera_copy_data_sem, 5);
        if(err == kNoErr)
        {
            push_data_to_queue();
        }
    }

 exit:
    camera_server_log( "camera server thread exit with err: %d", err );

    mico_rtos_delete_thread(NULL );
}


