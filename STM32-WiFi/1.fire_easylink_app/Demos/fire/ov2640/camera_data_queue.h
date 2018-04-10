#ifndef __CAMERA_DATA_QUEUE_H_
#define __CAMERA_DATA_QUEUE_H_

#include "mico.h"

#define CAMERA_QUEUE_NUM        (5)               //缓冲队列的大小
#define CAMERA_QUEUE_DATA_LEN   (1024 * 100)       //单帧图片的缓冲区大小

extern uint8_t *cam_global_buf;

extern int32_t find_jpeg_tail(uint8_t *data);
extern void camera_queue_free(void);
extern OSStatus camera_queue_init(void);
extern ssize_t find_usable_index(void);
extern OSStatus push_data_to_queue(void);
extern OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p, uint8_t *array_index);
extern void clear_array_flag(uint8_t index);

#endif

