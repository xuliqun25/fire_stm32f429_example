/**
  ******************************************************************************
  * @file    user_config.h
  * @author  Eshen Wang
  * @version V1.0.0
  * @date    17-Mar-2015
  * @brief   This file contains user configuration.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */

#ifndef __USER_CONFIG_H_
#define __USER_CONFIG_H_

#include "mico_config.h"


/*******************************************************************************
 *                              APP INFO
 ******************************************************************************/
/* product type
 * Replace them with your own product id/secure from fogcloud.io
 */
#ifdef MICO_FIRE_F429
  #define PRODUCT_ID                      "24a57d07"
  #define PRODUCT_KEY                     "3b8879d8-fb94-489e-a351-f0d5ee9caea7"
#endif


/*******************************************************************************
 *                             CONNECTING
 ******************************************************************************/

/* MICO cloud service type */
#define MICO_CLOUD_TYPE                  CLOUD_FOGCLOUD

// disalbe FogCloud OTA check when system start
#define DISABLE_FOGCLOUD_OTA_CHECK


/*******************************************************************************
 *                             RESOURCES
 ******************************************************************************/
#define STACK_SIZE_USER_MAIN_THREAD         0x1000
#define STACK_SIZE_USER_MSG_HANDLER_THREAD  0x800
#define STACK_SIZE_NOTIFY_THREAD            0x800
#define MICO_PROPERTIES_NOTIFY_INTERVAL_MS  1500


/*User provided configurations*/
#define CONFIGURATION_VERSION               0x00000004 // if your configuration is changed, update this number

#endif  // __USER_CONFIG_H_
