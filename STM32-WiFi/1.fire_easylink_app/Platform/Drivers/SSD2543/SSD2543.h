
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SSD2543_H_
#define __SSD2543_H_

/* Includes ------------------------------------------------------------------*/
#include "mico_platform.h"
#ifndef SSD2543_I2C_PORT
#define SSD2543_I2C_PORT    (MICO_I2C_1) //¥•√˛∆¡”√
#endif

#define FINGERNO						10
#define TOUCH_STATUS                  0x79
#define FINGER01_REG                  0x7c



extern OSStatus ssd2543_Init(void);

#endif  /* __SSD2543_H */
