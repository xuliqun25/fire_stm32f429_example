#ifndef __MALLOC_H
#define __MALLOC_H

#include "stm32f4xx.h"
#include "bsp_sdram.h"


//���������ڴ��
#define SRAMIN	    0		//�ڲ��ڴ��
#define SRAMEX      1		//�ⲿ�ڴ��(SDRAM)

#define SRAMBANK 	2	//����֧�ֵ�SRAM����.

//mem1�ڴ�����趨.mem1��ȫ�����ڲ�SRAM����.
#define MEM1_BLOCK_SIZE			64  	  						//�ڴ���СΪ64�ֽ�
#define MEM1_MAX_SIZE			1*1024  						//�������ڴ� 160K
#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//�ڴ���С

//mem2�ڴ�����趨.mem2���ڴ�ش����ⲿSDRAM����
#define MEM2_BLOCK_SIZE			64  	  						//�ڴ���СΪ64�ֽ�
#define MEM2_MAX_SIZE			7 * 1024 *1024  				//�������ڴ�7M Byte
#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//�ڴ���С

//�ڴ���������
struct _m_mallco_dev
{
	void (*init)(uint8_t);					//��ʼ��
	uint16_t (*perused)(uint8_t);		  	//�ڴ�ʹ����
	uint8_t 	*membase[SRAMBANK];			//�ڴ�� ����SRAMBANK��������ڴ�
	uint32_t *memmap[SRAMBANK]; 			//�ڴ����״̬��
	uint8_t  memrdy[SRAMBANK]; 				//�ڴ�����Ƿ����
};

////////////////////////////////////////////////////////////////////////////////
//�û����ú���
extern void sdram_heap_init(void);
extern void *sdram_malloc(uint32_t size);
extern void sdram_free(void *ptr);
#endif













