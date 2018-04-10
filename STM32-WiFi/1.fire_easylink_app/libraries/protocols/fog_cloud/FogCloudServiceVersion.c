/**
******************************************************************************
* @file    FogCloudServiceVersion.c
* @author  Eshen Wang
* @version V0.2.0
* @date    23-Nov-2014
* @brief   This file contains the release version of the Easycloud service. 
  operation
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

#include "FogCloudService.h"

//#define  debug_out 

//#ifdef debug_out
//#define  _debug_out debug_out
//#else
#define _debug_out(format, ...) do {;}while(0)

#define easycloud_service_version_log(M, ...) custom_log("FogCloudService", M, ##__VA_ARGS__)
#define easycloud_service_version_log_trace() custom_log_trace("FogCloudService")
//#endif

/*******************************************************************************
 * DEFINES
 ******************************************************************************/
#define FOGCLOUD_SERVCIE_VERSION_MAIN        0x01
#define FOGCLOUD_SERVCIE_VERSION_SUB         0x00
#define FOGCLOUD_SERVCIE_VERSION_REV         0x05

#define FOGCLOUD_SERVCIE_VERSION             (FOGCLOUD_SERVCIE_VERSION_MAIN << 16 | \
                                               FOGCLOUD_SERVCIE_VERSION_SUB << 8 | \
                                               FOGCLOUD_SERVCIE_VERSION_REV)

/*******************************************************************************
 * IMPLEMENTATIONS
 ******************************************************************************/

int FogCloudServiceVersion(easycloud_service_context_t* const context)
{
  if (NULL == context){
    return kParamErr;
  }
  return (int)FOGCLOUD_SERVCIE_VERSION;
}
