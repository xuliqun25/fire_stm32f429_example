#include "MICO.h"
#include "stm32f4xx.h"
#include "stm32f4xx_fmc.h"

#define user_log(M, ...) custom_log("SDRAM_TEST", M, ##__VA_ARGS__)

int application_start(void)
{	
	cli_init();
	
	BSP_LCDInit();
	gui_init();
	
	while(1)
	{    
		mico_thread_msleep(5);
		GDUI_refesh();
//		user_log("used in:%d", BSP_GetMemUseIn());	
//		user_log("used ex:%d", BSP_GetMemUseEx());	
	}
	
	return 0;
}