#ifndef __MALLOC_H
#define __MALLOC_H

#include "stm32f4xx.h"
#include "bsp_sdram.h"


//定义三个内存池
#define SRAMIN	    0		//内部内存池
#define SRAMEX      1		//外部内存池(SDRAM)

#define SRAMBANK 	2	//定义支持的SRAM块数.

//mem1内存参数设定.mem1完全处于内部SRAM里面.
#define MEM1_BLOCK_SIZE			64  	  						//内存块大小为64字节
#define MEM1_MAX_SIZE			1*1024  						//最大管理内存 160K
#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//内存表大小

//mem2内存参数设定.mem2的内存池处于外部SDRAM里面
#define MEM2_BLOCK_SIZE			64  	  						//内存块大小为64字节
#define MEM2_MAX_SIZE			7 * 1024 *1024  				//最大管理内存7M Byte
#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//内存表大小

//内存管理控制器
struct _m_mallco_dev
{
	void (*init)(uint8_t);					//初始化
	uint16_t (*perused)(uint8_t);		  	//内存使用率
	uint8_t 	*membase[SRAMBANK];			//内存池 管理SRAMBANK个区域的内存
	uint32_t *memmap[SRAMBANK]; 			//内存管理状态表
	uint8_t  memrdy[SRAMBANK]; 				//内存管理是否就绪
};

////////////////////////////////////////////////////////////////////////////////
//用户调用函数
extern void sdram_heap_init(void);
extern void *sdram_malloc(uint32_t size);
extern void sdram_free(void *ptr);
#endif













