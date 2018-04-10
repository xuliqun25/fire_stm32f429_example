/**
******************************************************************************
* @file    platform.c
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides all MICO Peripherals mapping table and platform
*          specific funcgtions.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "mico_platform.h"
#include "platform.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "PlatformLogging.h"
#include "spi_flash_platform_interface.h"
#include "wlan_platform_common.h"
#include "CheckSumUtils.h"
#include "keypad/gpio_button/button.h"

#ifdef USE_MiCOKit_EXT
#include "MiCOKit_EXT/micokit_ext.h"
#endif

/******************************************************
*                      Macros
******************************************************/

/******************************************************
*                    Constants
******************************************************/

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/
extern WEAK void PlatformEasyLinkButtonClickedCallback(void);
extern WEAK void PlatformEasyLinkButtonLongPressedCallback(void);
extern WEAK void bootloader_start(void);

/******************************************************
*               Variables Definitions
******************************************************/

const platform_gpio_t platform_gpio_pins[] =
{
	/* Common GPIOs for internal use */
	[MICO_SYS_LED]                      = { GPIOD,  12}, //�ɰ���PD11  �°���PD12
//	[MICO_RF_LED]                       = { GPIOD,  12},
	[BOOT_SEL]                          = { GPIOB,  2 },
//	[MFG_SEL]                           = { GPIOE,  4 },
	[EasyLink_BUTTON]                   = { GPIOC,  13 },
	[STDIO_UART_RX]                     = { GPIOA,  10},
	[STDIO_UART_TX]                     = { GPIOA,  9 },
	[FLASH_PIN_SPI_CS  ]                = { GPIOF,  6 },
	[FLASH_PIN_SPI_CLK ]                = { GPIOF,  7 },
	[FLASH_PIN_SPI_MOSI]                = { GPIOF,  9 },
	[FLASH_PIN_SPI_MISO]                = { GPIOF,  8 },


	/* GPIOs for external use */
    [MICO_GPIO_1]                       = { GPIOH,  10 }, //RGB-R
    [MICO_GPIO_2]                       = { GPIOH,  11 }, //RGB-G
    [MICO_GPIO_3]                       = { GPIOH,  12 }, //RGB-B

    [MICO_GPIO_4]                       = { GPIOI,  11 }, //beep

    [MICO_GPIO_5]                       = { GPIOD,  5 }, //USART2_TX
    [MICO_GPIO_6]                       = { GPIOD,  6 }, //USART2_RX

    [MICO_GPIO_7]                       = { GPIOE,  2 }, //DHT11

    [MICO_GPIO_8]                       = { GPIOA,  4 }, //���յ���
    [MICO_GPIO_9]                       = { GPIOC,  3 }, //����������
};

platform_i2c_driver_t platform_i2c_drivers[MICO_I2C_MAX];
const platform_adc_t platform_adc_peripherals[MICO_ADC_MAX];
const platform_i2c_t platform_i2c_peripherals[MICO_I2C_MAX];


const platform_adc_t platform_adc_peripherals[] =
{
    [MICO_ADC_1] = { ADC1, ADC_Channel_4, RCC_APB2Periph_ADC1, 1, (platform_gpio_t*)&platform_gpio_pins[MICO_GPIO_8] },
    [MICO_ADC_2] = { ADC1, ADC_Channel_13, RCC_APB2Periph_ADC1, 1, (platform_gpio_t*)&platform_gpio_pins[MICO_GPIO_9] },
};

const platform_pwm_t platform_pwm_peripherals[] =
{
    [MICO_PWM_R] =
    {
        .tim = TIM5,
        .channel = 1,
        .tim_peripheral_clock = RCC_APB1Periph_TIM5,
        .gpio_af = GPIO_AF_TIM5,
        .pin = &platform_gpio_pins[MICO_GPIO_1]
    },
    [MICO_PWM_G] =
    {
        .tim = TIM5,
        .channel = 2,
        .tim_peripheral_clock = RCC_APB1Periph_TIM5,
        .gpio_af = GPIO_AF_TIM5,
        .pin = &platform_gpio_pins[MICO_GPIO_2]
    },

    [MICO_PWM_B] =
    {
        .tim = TIM5,
        .channel = 3,
        .tim_peripheral_clock = RCC_APB1Periph_TIM5,
        .gpio_af = GPIO_AF_TIM5,
        .pin = &platform_gpio_pins[MICO_GPIO_3]
    }
};


const platform_uart_t platform_uart_peripherals[] =
{
	[MICO_UART_1] =
	{
		.port                         = USART1,
		.pin_tx                       = &platform_gpio_pins[STDIO_UART_TX],
		.pin_rx                       = &platform_gpio_pins[STDIO_UART_RX],
		.pin_cts                      = NULL,
		.pin_rts                      = NULL,
		.tx_dma_config =
		{
		  .controller                 = DMA2,
		  .stream                     = DMA2_Stream7,
		  .channel                    = DMA_Channel_4,
		  .irq_vector                 = DMA2_Stream7_IRQn,
		  .complete_flags             = DMA_HISR_TCIF7,
		  .error_flags                = ( DMA_HISR_TEIF7 | DMA_HISR_FEIF7 ),
		},
		.rx_dma_config =
		{
		  .controller                 = DMA2,
		  .stream                     = DMA2_Stream2,
		  .channel                    = DMA_Channel_4,
		  .irq_vector                 = DMA2_Stream2_IRQn,
		  .complete_flags             = DMA_LISR_TCIF2,
		  .error_flags                = ( DMA_LISR_TEIF2 | DMA_LISR_FEIF2 | DMA_LISR_DMEIF2 ),
		},
	},
    [MICO_UART_2] =
	{
		.port                         = USART2,
		.pin_tx                       = &platform_gpio_pins[MICO_GPIO_5],
		.pin_rx                       = &platform_gpio_pins[MICO_GPIO_6],
		.pin_cts                      = NULL,
		.pin_rts                      = NULL,
		.tx_dma_config =
		{
		  .controller                 = DMA1,
		  .stream                     = DMA1_Stream6,
		  .channel                    = DMA_Channel_4,
          .irq_vector                 = DMA1_Stream6_IRQn,
          .complete_flags             = DMA_HISR_TCIF6,
          .error_flags                = ( DMA_HISR_TEIF6 | DMA_HISR_FEIF6 ),
		},
		.rx_dma_config =
		{
		  .controller                 = DMA1,
		  .stream                     = DMA1_Stream5,
		  .channel                    = DMA_Channel_4,
          .irq_vector                 = DMA1_Stream5_IRQn,
          .complete_flags             = DMA_HISR_TCIF5,
          .error_flags                = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
		},
	},

};
platform_uart_driver_t platform_uart_drivers[MICO_UART_MAX];

const platform_spi_t platform_spi_peripherals[] =
{
	[MICO_SPI_1]  =
	{
		.port                         = SPI5,
		.gpio_af                      = GPIO_AF_SPI5,
		.peripheral_clock_reg         = RCC_APB2Periph_SPI5,
		.peripheral_clock_func        = RCC_APB2PeriphClockCmd,
		.pin_mosi                     = &platform_gpio_pins[FLASH_PIN_SPI_MOSI],
		.pin_miso                     = &platform_gpio_pins[FLASH_PIN_SPI_MISO],
		.pin_clock                    = &platform_gpio_pins[FLASH_PIN_SPI_CLK],
		.tx_dma =
		{
			.controller                 = DMA2,
			.stream                     = DMA2_Stream6,
			.channel                    = DMA_Channel_7,
			.irq_vector                 = DMA2_Stream6_IRQn,
			.complete_flags             = DMA_HISR_TCIF6,
			.error_flags                = (DMA_HISR_TEIF6 | DMA_HISR_FEIF6),
		},
		.rx_dma =
		{
			.controller                 = DMA2,
			.stream                     = DMA2_Stream5,
			.channel                    = DMA_Channel_7,
			.irq_vector                 = DMA2_Stream5_IRQn,
			.complete_flags             = DMA_HISR_TCIF5,
			.error_flags                = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
		},
	}
};

platform_spi_driver_t platform_spi_drivers[MICO_SPI_MAX];

/* Flash memory devices */
const platform_flash_t platform_flash_peripherals[] =
{
	[MICO_FLASH_EMBEDDED] =
	{
		.flash_type                   = FLASH_TYPE_EMBEDDED,
		.flash_start_addr             = 0x08000000,
		.flash_length                 = 0x100000,
	},
	[MICO_FLASH_SPI] =
	{
		.flash_type                   = FLASH_TYPE_SPI,
		.flash_start_addr             = 0x000000,
		.flash_length                 = 0x1000000,
	},
};

platform_flash_driver_t platform_flash_drivers[MICO_FLASH_MAX];

/* Logic partition on flash devices */
const mico_logic_partition_t mico_partitions[] =
{
	[MICO_PARTITION_BOOTLOADER] =
	{
		.partition_owner           = MICO_FLASH_EMBEDDED,
		.partition_description     = "Bootloader",
		.partition_start_addr      = 0x08000000,
		.partition_length          =     0x8000,    //32k bytes
		.partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	},
    [MICO_PARTITION_PARAMETER_1] =
	{
		.partition_owner           = MICO_FLASH_SPI,
		.partition_description     = "PARAMETER1",
		.partition_start_addr      = 1347*4096,
		.partition_length          = 0x1000, //4KB
		.partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
	},
	[MICO_PARTITION_PARAMETER_2] =
	{
		.partition_owner           = MICO_FLASH_SPI,
		.partition_description     = "PARAMETER2",
		.partition_start_addr      = 1348*4096,
		.partition_length          = 0x1000, //4KB
		.partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
	},
	[MICO_PARTITION_APPLICATION] =
	{
		.partition_owner           = MICO_FLASH_EMBEDDED,
		.partition_description     = "Application",
		.partition_start_addr      = 0x8010000,
		.partition_length          = (0x100000 - 0x10000),   //
		.partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	},
	[MICO_PARTITION_RF_FIRMWARE] =
	{
		.partition_owner           = MICO_FLASH_SPI,
		.partition_description     = "RF Firmware",
		.partition_start_addr      = 1284*4096,     //0x80B0000
		.partition_length          = 0x3E000,     //0x40000 -> 256KB
		.partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	},
    [MICO_PARTITION_OTA_TEMP] =
    {
        .partition_owner           = MICO_FLASH_NONE,
        .partition_description     = "OTA Storage",
        .partition_start_addr      = 0,
        .partition_length          = 0, //
        .partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    }
};

#if defined ( USE_MICO_SPI_FLASH )
const mico_spi_device_t mico_spi_flash =
{
	.port        = MICO_SPI_1,
	.chip_select = FLASH_PIN_SPI_CS,
	.speed       = 40000000,
	.mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_USE_DMA | SPI_MSB_FIRST ), //SPI_USE_DMA
	.bits        = 8
};
#endif

/* Wi-Fi control pins. Used by platform/MCU/wlan_platform_common.c
* SDIO: EMW1062_PIN_BOOTSTRAP[1:0] = b'00
* gSPI: EMW1062_PIN_BOOTSTRAP[1:0] = b'01
*/
const platform_gpio_t wifi_control_pins[] =
{
  [WIFI_PIN_RESET]			= { GPIOG, 9 },
};

/* Wi-Fi SDIO bus pins. Used by platform/MCU/STM32F2xx/EMW1062_driver/wlan_SDIO.c */
const platform_gpio_t wifi_sdio_pins[] =
{
  [WIFI_PIN_SDIO_OOB_IRQ] 	= { GPIOA,  0 },
  [WIFI_PIN_SDIO_CLK    ] 	= { GPIOC,  12},
  [WIFI_PIN_SDIO_CMD    ] 	= { GPIOD,  2 },
  [WIFI_PIN_SDIO_D0     ] 	= { GPIOC,  8 },
  [WIFI_PIN_SDIO_D1     ] 	= { GPIOC,  9 },
  [WIFI_PIN_SDIO_D2     ] 	= { GPIOC,  10},
  [WIFI_PIN_SDIO_D3     ] 	= { GPIOC,  11},
};

/******************************************************
*           Interrupt Handler Definitions
******************************************************/

MICO_RTOS_DEFINE_ISR( USART1_IRQHandler )
{
	platform_uart_irq( &platform_uart_drivers[MICO_UART_1] );
}

MICO_RTOS_DEFINE_ISR( DMA2_Stream7_IRQHandler )
{
	platform_uart_tx_dma_irq( &platform_uart_drivers[MICO_UART_1] );
}

MICO_RTOS_DEFINE_ISR( DMA2_Stream2_IRQHandler )
{
	platform_uart_rx_dma_irq( &platform_uart_drivers[MICO_UART_1] );
}

MICO_RTOS_DEFINE_ISR( USART2_IRQHandler )
{
	platform_uart_irq( &platform_uart_drivers[MICO_UART_2] );
}

MICO_RTOS_DEFINE_ISR( DMA1_Stream6_IRQHandler )
{
	platform_uart_tx_dma_irq( &platform_uart_drivers[MICO_UART_2] );
}

MICO_RTOS_DEFINE_ISR( DMA1_Stream5_IRQHandler )
{
	platform_uart_rx_dma_irq( &platform_uart_drivers[MICO_UART_2] );
}

/******************************************************
*               Function Definitions
******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
	/* Interrupt priority setup. Called by MiCO/platform/MCU/STM32F2xx/platform_init.c */
	NVIC_SetPriority( RTC_WKUP_IRQn    ,  1 ); /* RTC Wake-up event   */
	NVIC_SetPriority( SDIO_IRQn        ,  2 ); /* WLAN SDIO           */
	NVIC_SetPriority( DMA2_Stream3_IRQn,  3 ); /* WLAN SDIO DMA       */
//	NVIC_SetPriority( DMA2_Stream6_IRQn,  3 ); /* WLAN SPI DMA        */ //WLAN�õ���SPI�ӿ�������ʽ
	NVIC_SetPriority( USART1_IRQn      ,  6 ); /* MICO_UART_1         */
    NVIC_SetPriority( USART2_IRQn      ,  6 ); /* MICO_UART_2         */
	NVIC_SetPriority( DMA2_Stream7_IRQn,  7 ); /* MICO_UART_1 TX DMA  */
	NVIC_SetPriority( DMA2_Stream2_IRQn,  7 ); /* MICO_UART_1 RX DMA  */
    NVIC_SetPriority( DMA1_Stream6_IRQn,  7 ); /* MICO_UART_2 TX DMA  */
	NVIC_SetPriority( DMA1_Stream5_IRQn,  7 ); /* MICO_UART_2 RX DMA  */
	NVIC_SetPriority( EXTI0_IRQn       , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI1_IRQn       , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI2_IRQn       , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI3_IRQn       , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI4_IRQn       , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI9_5_IRQn     , 14 ); /* GPIO                */
	NVIC_SetPriority( EXTI15_10_IRQn   , 14 ); /* GPIO                */
}

void init_platform( void )
{
	button_init_t init;

	MicoGpioInitialize( (mico_gpio_t)MICO_SYS_LED, OUTPUT_PUSH_PULL );
	MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
	MicoGpioInitialize( (mico_gpio_t)MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
	MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );

	MicoGpioInitialize((mico_gpio_t)BOOT_SEL, INPUT_PULL_UP);
	MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_PULL_UP);

	init.gpio = EasyLink_BUTTON;
	init.pressed_func = PlatformEasyLinkButtonClickedCallback;
	init.long_pressed_func = PlatformEasyLinkButtonLongPressedCallback;
	init.long_pressed_timeout = 5000;

	button_init( IOBUTTON_EASYLINK, init );

#ifdef USE_MiCOKit_EXT
	dc_motor_init( );
	dc_motor_set( 0 );

	rgb_led_init();
	rgb_led_open(0, 0, 0);
#endif
}

#ifdef BOOTLOADER

#if 0
#define SizePerRW                   (4096)
static uint8_t data[SizePerRW];
#endif

void init_platform_bootloader( void )
{
	OSStatus err = kNoErr;
	mico_logic_partition_t *rf_partition = MicoFlashGetInfo( MICO_PARTITION_RF_FIRMWARE );

	MicoGpioInitialize( (mico_gpio_t)MICO_SYS_LED, OUTPUT_PUSH_PULL );
	MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
	MicoGpioInitialize( (mico_gpio_t)MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
	MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );

	MicoGpioInitialize((mico_gpio_t)BOOT_SEL, INPUT_PULL_UP);
	MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_PULL_UP);

#ifdef USE_MiCOKit_EXT
	dc_motor_init( );
	dc_motor_set( 0 );

	rgb_led_init();
	rgb_led_open(0, 0, 0);
#endif

#if 0
	/* Specific operations used in EMW3165 production */
#define NEED_RF_DRIVER_COPY_BASE    ((uint32_t)0x08008000)
#define TEMP_RF_DRIVER_BASE         ((uint32_t)0x08040000)
#define TEMP_RF_DRIVER_END          ((uint32_t)0x0807FFFF)

	const uint8_t isDriverNeedCopy = *(uint8_t *)(NEED_RF_DRIVER_COPY_BASE);
	const uint32_t totalLength = rf_partition->partition_length;
	const uint8_t crcResult = *(uint8_t *)(TEMP_RF_DRIVER_END);
	uint8_t targetCrcResult = 0;

	uint32_t copyLength;
	uint32_t destStartAddress_tmp = rf_partition->partition_start_addr;
	uint32_t sourceStartAddress_tmp = TEMP_RF_DRIVER_BASE;
	uint32_t i;

	if ( isDriverNeedCopy != 0x0 )
		return;

	platform_log( "Bootloader start to copy RF driver..." );
	/* Copy RF driver to SPI flash */

	err = platform_flash_init( &platform_flash_peripherals[ MICO_FLASH_SPI ] );
	require_noerr(err, exit);
	err = platform_flash_init( &platform_flash_peripherals[ MICO_FLASH_EMBEDDED ] );
	require_noerr(err, exit);
	err = platform_flash_erase( &platform_flash_peripherals[ MICO_FLASH_SPI ],
							   rf_partition->partition_start_addr, rf_partition->partition_start_addr + rf_partition->partition_length - 1 );
	require_noerr(err, exit);
	platform_log( "Time: %d", mico_get_time_no_os() );

	for(i = 0; i <= totalLength/SizePerRW; i++){
		if( i == totalLength/SizePerRW ){
			if(totalLength%SizePerRW)
				copyLength = totalLength%SizePerRW;
			else
				break;
		}else{
			copyLength = SizePerRW;
		}
		printf(".");
		err = platform_flash_read( &platform_flash_peripherals[ MICO_FLASH_EMBEDDED ], &sourceStartAddress_tmp, data , copyLength );
		require_noerr( err, exit );
		err = platform_flash_write( &platform_flash_peripherals[ MICO_FLASH_SPI ], &destStartAddress_tmp, data, copyLength );
		require_noerr(err, exit);
	}

	printf("\r\n");
	/* Check CRC-8 check-sum */
	platform_log( "Bootloader start to verify RF driver..." );
	sourceStartAddress_tmp = TEMP_RF_DRIVER_BASE;
	destStartAddress_tmp = rf_partition->partition_start_addr;

	for(i = 0; i <= totalLength/SizePerRW; i++){
		if( i == totalLength/SizePerRW ){
			if(totalLength%SizePerRW)
				copyLength = totalLength%SizePerRW;
			else
				break;
		}else{
			copyLength = SizePerRW;
		}
		printf(".");
		err = platform_flash_read( &platform_flash_peripherals[ MICO_FLASH_SPI ], &destStartAddress_tmp, data, copyLength );
		require_noerr( err, exit );

		targetCrcResult = mico_CRC8_Table(targetCrcResult, data, copyLength);
	}

	printf("\r\n");
	//require_string( crcResult == targetCrcResult, exit, "Check-sum error" );
	if( crcResult != targetCrcResult ){
		platform_log("Check-sum error");
		while(1);
	}
	/* Clear RF driver from temperary storage */
	platform_log("Bootloader start to clear RF driver temporary storage...");

	/* Clear copy tag */
	err = platform_flash_erase( &platform_flash_peripherals[ MICO_FLASH_EMBEDDED ], NEED_RF_DRIVER_COPY_BASE, NEED_RF_DRIVER_COPY_BASE);
	require_noerr(err, exit);

#endif

 exit:
	return;
}

#endif

void MicoSysLed(bool onoff)
{
	if (onoff) {
		MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
	} else {
		MicoGpioOutputHigh( (mico_gpio_t)MICO_SYS_LED );
	}
}

void MicoRfLed(bool onoff)
{
	if (onoff) {
		MicoGpioOutputLow( (mico_gpio_t)MICO_RF_LED );
	} else {
		MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );
	}
}

#ifdef USE_MiCOKit_EXT
// add test mode for MiCOKit-EXT board,check Arduino_D5 pin when system startup
bool MicoExtShouldEnterTestMode(void)
{
	if( MicoGpioInputGet((mico_gpio_t)Arduino_D5)==false ){
		return true;
	}
	else{
		return false;
	}
}
#endif

bool MicoShouldEnterMFGMode(void)
{
	if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==false)
		return true;
	else
		return false;
}

bool MicoShouldEnterBootloader(void)
{
	if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==true)
		return true;
	else
		return false;
}