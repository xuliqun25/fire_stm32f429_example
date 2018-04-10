/**
******************************************************************************
* @file    ov2640_Init.c
* @author  fire
* @version V1.0
* @date    2015-xx-xx
* @brief   OV2640摄像头驱动
******************************************************************************
* @attention
*
* 实验平台:秉火  STM32 F429 开发板
* 论坛    :http://www.chuxue123.com
* 淘宝    :http://firestm32.taobao.com
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ov2640_Init.h"
#include "camera_data_queue.h"
#include "camera_server.h"

uint32_t frame_counter = 0;
uint32_t line_counter = 0;
uint32_t vs_counter = 0;
uint32_t err_counter = 0;

mico_semaphore_t camera_copy_data_sem = NULL;

/** @addtogroup DCMI_Camera
* @{
*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
void OV2640_HW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /*** Configures the DCMI GPIOs to interface with the OV2640 camera module ***/
    /* Enable DCMI GPIOs clocks */
    RCC_AHB1PeriphClockCmd(DCMI_PWDN_GPIO_CLK|DCMI_VSYNC_GPIO_CLK | DCMI_HSYNC_GPIO_CLK | DCMI_PIXCLK_GPIO_CLK|
                           DCMI_D0_GPIO_CLK| DCMI_D1_GPIO_CLK| DCMI_D2_GPIO_CLK| DCMI_D3_GPIO_CLK|
                               DCMI_D4_GPIO_CLK| DCMI_D5_GPIO_CLK| DCMI_D6_GPIO_CLK| DCMI_D7_GPIO_CLK, ENABLE);

    /*摄像头信号线*/
    GPIO_InitStructure.GPIO_Pin = DCMI_VSYNC_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_VSYNC_GPIO_PORT, DCMI_VSYNC_PINSOURCE, DCMI_VSYNC_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_HSYNC_GPIO_PIN ;
    GPIO_Init(DCMI_HSYNC_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_HSYNC_GPIO_PORT, DCMI_HSYNC_PINSOURCE, DCMI_HSYNC_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_PIXCLK_GPIO_PIN ;
    GPIO_Init(DCMI_PIXCLK_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_PIXCLK_GPIO_PORT, DCMI_PIXCLK_PINSOURCE, DCMI_PIXCLK_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_PWDN_GPIO_PIN ;
    GPIO_Init(DCMI_PWDN_GPIO_PORT, &GPIO_InitStructure);

    GPIO_ResetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN);
    //	  GPIO_PinAFConfig(DCMI_PWDN_GPIO_PORT, DCMI_HSYNC_PINSOURCE, DCMI_HSYNC_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D0_GPIO_PIN ;
    GPIO_Init(DCMI_D0_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D0_GPIO_PORT, DCMI_D0_PINSOURCE, DCMI_D0_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D1_GPIO_PIN ;
    GPIO_Init(DCMI_D1_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D1_GPIO_PORT, DCMI_D1_PINSOURCE, DCMI_D1_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D2_GPIO_PIN ;
    GPIO_Init(DCMI_D2_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D2_GPIO_PORT, DCMI_D2_PINSOURCE, DCMI_D2_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D3_GPIO_PIN ;
    GPIO_Init(DCMI_D3_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D3_GPIO_PORT, DCMI_D3_PINSOURCE, DCMI_D3_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D4_GPIO_PIN ;
    GPIO_Init(DCMI_D4_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D4_GPIO_PORT, DCMI_D4_PINSOURCE, DCMI_D4_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D5_GPIO_PIN ;
    GPIO_Init(DCMI_D5_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D5_GPIO_PORT, DCMI_D5_PINSOURCE, DCMI_D5_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D6_GPIO_PIN ;
    GPIO_Init(DCMI_D6_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D6_GPIO_PORT, DCMI_D6_PINSOURCE, DCMI_D6_AF);

    GPIO_InitStructure.GPIO_Pin = DCMI_D7_GPIO_PIN ;
    GPIO_Init(DCMI_D7_GPIO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(DCMI_D7_GPIO_PORT, DCMI_D7_PINSOURCE, DCMI_D7_AF);

}

//void dcmi_addr_configuration(uint32_t *BufferSRC, uint32_t BufferSize)
//{
//    DMA_InitTypeDef DMA_InitStructure;
//
//    DMA_Cmd(DMA2_Stream1, DISABLE);
//
//	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
//	DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_HTIF1);
//
//    DMA_StructInit(&DMA_InitStructure);
//
//	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
//	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(DCMI->DR);
//	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
//	DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;    //..... DMA_MemoryDataSize_HalfWord
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
//	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
//	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
//	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
//	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
//
//	DMA_ITConfig(DMA2_Stream1, /*DMA_IT_TC|*/DMA_IT_TE/*|DMA_IT_HT*/, ENABLE);
//
//}

void DMA_DCMIConfiguration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_HTIF1);

	DMA_Cmd(DMA2_Stream1, DISABLE);
	//memset(&DMA_InitStructure, 0, sizeof(DMA_InitTypeDef));

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(DCMI->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;    //..... DMA_MemoryDataSize_HalfWord
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream1, /*DMA_IT_TC|*/DMA_IT_TE/*|DMA_IT_HT*/, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;//DMA2_Stream1_IRQn_Priority  TBD----- by Ken
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(DMA2_Stream1, ENABLE);

}

/**
* @brief  初始化摄像头，初始化mcu的dcmi接口，以及摄像头，但是这里还没开始捕获图像
* @param
* @param
*/
OSStatus open_camera(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DCMI_InitTypeDef DCMI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//DMA2
	OV2640_HW_Init();

	DCMI_DeInit();
	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
	DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_Low;
	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;
	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_Low;

	DCMI_Init(&DCMI_InitStructure);
	DMA_DCMIConfiguration(BufferSRC, BufferSize);
	DCMI_ITConfig(DCMI_IT_FRAME | DCMI_IT_OVF | DCMI_IT_ERR |DCMI_IT_VSYNC | DCMI_IT_LINE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;//DCMI_IRQn_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	if (ov2640_SCCB_Init(JPEG_800x600) != kNoErr)
        return  kGeneralErr;

	DCMI_JPEGCmd(ENABLE); //for ov2640

    start_capture_img();

    DCMI_Cmd(ENABLE); //数据开关

	return kNoErr;
}

/**
* @brief  关闭摄像头
* @param
* @param
*/
int close_camera()
{
	DCMI_Cmd(DISABLE);

	DMA_Cmd(DMA2_Stream1, DISABLE);

	DCMI_DeInit();

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, DISABLE);

	return 0;
}

/**
* @brief  开始捕获图像，如果是连续获取模式，图片会源源不断的发往img_send_thread线程，等待发送出去
* @param
* @param
*/
void start_capture_img()
{
	DCMI_CaptureCmd(ENABLE);
}

/**
* @brief  停止捕获图像
* @param
* @param
*/
void stop_capture_img()
{
	DCMI_CaptureCmd(DISABLE);
}

void DCMI_Start(void)
{
    DMA_Cmd(DMA2_Stream1,DISABLE);
    DMA_DCMIConfiguration((uint32_t *)cam_global_buf, CAMERA_QUEUE_DATA_LEN);
	DMA_Cmd(DMA2_Stream1, ENABLE);
	DCMI_CaptureCmd(ENABLE);
    DCMI_Cmd(ENABLE);
}

void DCMI_Stop(void)
{
    DMA_Cmd(DMA2_Stream1,DISABLE);

	while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);

    DCMI_CaptureCmd(DISABLE);

    DCMI_Cmd(DISABLE);
}



void DMA2_Stream1_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA2_Stream1,DMA_FLAG_TCIF1)==SET)
	{
		 DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1);
	}
}

void DCMI_IRQHandler(void)
{
	if(DCMI_GetITStatus(DCMI_IT_FRAME) == SET )
	{
		DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        frame_counter ++;

        //1.停止DCMI传输
        DCMI_Stop();

        //2.进入处理函数
        mico_rtos_set_semaphore(&camera_copy_data_sem);
	}

    if(DCMI_GetITStatus(DCMI_IT_LINE) == SET )
	{
        DCMI_ClearITPendingBit(DCMI_IT_LINE);
        line_counter ++;
	}

    if(DCMI_GetITStatus(DCMI_IT_VSYNC) == SET )
	{
        DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
        vs_counter ++;
	}

    if(DCMI_GetITStatus(DCMI_IT_ERR) == SET )
	{
        err_counter ++;
	}

}

