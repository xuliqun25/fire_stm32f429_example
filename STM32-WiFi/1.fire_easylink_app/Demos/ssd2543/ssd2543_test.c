#include "MICO.h"
#include "SSD2543.h"

#define user_log(M, ...) 	custom_log("ssd2543_test", M, ##__VA_ARGS__)

int application_start( void )
{
	OSStatus err = kNoErr;
	uint8_t reg_buff[10] = {0};
	uint8_t write_buff[2] = {0x00, 0xFF};
	
	cli_init();
	
	/*init DC Motor*/ 
	err = ssd2543_Init();
	require_noerr_action( err, exit, user_log("ERROR: Unable to Init ssd2543") );
	
	while(1)
	{    
		ssd2543_ts_work();
		mico_thread_msleep(50);
	}
 exit:
	return err; 
}



