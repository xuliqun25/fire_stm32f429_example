#ifndef __RHEOSTAT_H_
#define __RHEOSTAT_H_

#define RHEOSTAT_ADC_SAMPLE_CYCLE    (3)


extern OSStatus rheostat_init(void);
extern OSStatus rheostat_read(uint16_t *data);


#endif

