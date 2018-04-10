#include "mico_platform.h"
#include "fire_beep.h"

#define beep_log(M, ...)    custom_log("BEEP", M, ##__VA_ARGS__)


int beep_init(void);
int beep_set(int value);

int beep_init(void)
{
    MicoGpioInitialize(FIRE_BEEP_PIN, OUTPUT_PUSH_PULL);
    return 0;
}

int beep_set(int value)
{
    beep_init();

    if (BEEP_OFF == value)
    {
        MicoGpioOutputLow( FIRE_BEEP_PIN );
    }
    else
    {
        MicoGpioOutputHigh( FIRE_BEEP_PIN );
    }

    return 0;
}