#include "MICO.h"
#include "stm32f4xx.h"
#include "stm32f4xx_fmc.h"
#include "stm32f429i_sdram.h"
#include "malloc.h"

#define user_log(M, ...) custom_log("SDRAM_TEST", M, ##__VA_ARGS__)
#define IS42S16400J_SIZE             0x400000


void sdram_test(void)
{
	uint32_t count = 0;
	uint8_t *p = NULL;
	uint32_t temp = 0;
	
	for(count = 0; count < IS42S16400J_SIZE / 4; count ++)
	{
		*((uint32_t *)(SDRAM_BANK_ADDR+ 4*count)) = (uint32_t)0x0;
	}
	
	p = mymalloc(SRAMEX, 1024*1024);
	
	temp = mem_perused(SRAMEX);
	user_log("temp = %d", temp);	
	
	for(count = 0; count < 100; count ++)
	{
		*(p + count) = 0xAA;
	}
	
	myfree(SRAMEX, p);
	
	return;
}

int application_start(void)
{	
	/* SDRAM Initialization */  
	SDRAM_Init();
	
	/* FMC SDRAM GPIOs Configuration */
	SDRAM_GPIOConfig();
	
	/* Disable write protection */
	FMC_SDRAMWriteProtectionConfig(FMC_Bank2_SDRAM,DISABLE); 

	mymem_init(SRAMIN);
	mymem_init(SRAMEX);
	mymem_init(SRAMCCM);
	
	while(1)
	{    
		sdram_test();	
	}
	
	return 0;
}