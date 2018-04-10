#ifndef __USER_CONFIG_H_
#define __USER_CONFIG_H_

#include "mico_config.h"

/*******************************************************************************
 *                             APP INFO
 ******************************************************************************/

/*------------------------------ product -------------------------------------*/
#ifdef MICOKIT_3288
// wes' product, replace it with your own product
  #define PRODUCT_ID                   "dd21c259"
  #define PRODUCT_KEY                  "668dd324-8a2f-424f-ad7d-d43102973a57"
#elif  MICOKIT_3165
  #define PRODUCT_ID                   "dd21c259"
  #define PRODUCT_KEY                  "668dd324-8a2f-424f-ad7d-d43102973a57"
#elif  MICOKIT_G55
  #define PRODUCT_ID                   "dd21c259"
  #define PRODUCT_KEY                  "668dd324-8a2f-424f-ad7d-d43102973a57"
#elif DISPLAY_BOARD
  #define PRODUCT_ID                   "1f3d4061"
  #define PRODUCT_KEY                  "84295242-51b1-4fe5-813a-7950ec35abf2"
#endif

/*******************************************************************************
 *                             CONNECTING
 ******************************************************************************/

/* Firmware update check
 * If need to check new firmware on server after wifi on, comment out this macro
 */
#define DISABLE_FOGCLOUD_OTA_CHECK							//设置禁止OTA检查升级

#define MICO_CLOUD_TYPE    					CLOUD_FOGCLOUD	//设置连接fogcloud
#define STACK_SIZE_USER_MAIN_THREAD    		0x2000			//设置的主线程栈大小


/*******************************************************************************
 *                             RESOURCES
 ******************************************************************************/
#define STACK_SIZE_USER_DOWNSTREAM_THREAD 		0x400
#define STACK_SIZE_USER_UPSTREAM_THREAD   		0x400
#define STACK_SIZE_USER_LCD_REFRESH_THREAD   	0x1500


/* User provided configurations seed
 * If user configuration(params in flash) is changed, update this number to
 * indicate the bootloader to clean params in flash next time restart.
 */
#define CONFIGURATION_VERSION          		0x00000005

#endif  // __USER_CONFIG_H_

