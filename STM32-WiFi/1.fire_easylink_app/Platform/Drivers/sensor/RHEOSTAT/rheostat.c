#include "mico_platform.h"
#include "rheostat.h"

#define rheostat_log(M, ...) custom_log("RHEOSTAT", M, ##__VA_ARGS__)

OSStatus rheostat_init(void);
OSStatus rheostat_read(uint16_t *data);


/*------------------------------ USER INTERFACES -----------------------------*/

OSStatus rheostat_init(void)
{
  OSStatus err = kUnknownErr;

  err = MicoAdcInitialize(RHEOSTAT_ADC, RHEOSTAT_ADC_SAMPLE_CYCLE);

  return err;
}

OSStatus rheostat_read(uint16_t *data)
{
    OSStatus err = kUnknownErr;

    // init ADC
    err = MicoAdcInitialize(RHEOSTAT_ADC, RHEOSTAT_ADC_SAMPLE_CYCLE);
    if(kNoErr != err)
    {
        goto exit;
    }

    // get ADC data
    err = MicoAdcTakeSample(RHEOSTAT_ADC, data);

 exit:
    return err;
}