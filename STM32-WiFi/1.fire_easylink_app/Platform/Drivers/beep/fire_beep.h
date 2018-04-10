#ifndef __FIRE_BEEP_H_
#define __FIRE_BEEP_H_

#define FIRE_BEEP_PIN   (MICO_GPIO_4)

#define BEEP_ON         (1)
#define BEEP_OFF         (0)


extern int beep_set(int value);
extern int beep_init(void);


#endif

