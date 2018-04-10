#ifndef __CAMERA_DATA_QUEUE_H_
#define __CAMERA_DATA_QUEUE_H_

#include "mico.h"

#ifdef   OV5640
#include "OV5640_init.h"
#endif
#ifdef   OV2640
#include "OV2640_init.h"
#endif


//缓冲队列的个数需要为2的幂
#define CAMERA_QUEUE_NUM        (2)               //缓冲队列的大小
#define CAMERA_QUEUE_DATA_LEN   (1024 * 1024*4)       //单帧图片的缓冲区大小


typedef struct _camera_data
{
	uint8_t *head;			//缓冲区头指针		
	int img_dma_len;	//dma计算得的文件长度
	uint8_t *img_real_start;	//文件头指针
	int img_real_len;					//文件长度
}camera_data;

typedef struct {
	int         size;  /* 缓冲区大小          */
	int         start; /* 读指针              */
	int         end;   /* 写指针  */
	int start_using;	/*正在读取的缓冲区指针*/
	int end_using;		/*正在写入的缓冲区指针*/
	camera_data    *elems[CAMERA_QUEUE_NUM];  /* 缓冲区地址                   */
} CircularBuffer;


extern uint8_t *cam_global_buf;
extern int32_t find_jpeg_tail(uint8_t *data,uint8_t *jpeg_start,int32_t search_point) ;
extern void camera_queue_free(void);
extern OSStatus camera_queue_init(void);
extern ssize_t find_usable_index(void);
//extern OSStatus push_data_to_queue(void);
extern OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p);
extern void clear_array_flag(uint8_t index);

extern int32_t find_jpeg(camera_data *cambuf);



extern camera_data* cbWrite(CircularBuffer *cb);
extern camera_data* cbRead(CircularBuffer *cb);
extern void cbReadFinish(CircularBuffer *cb);
extern void cbWriteFinish(CircularBuffer *cb);
extern void cbPrint(CircularBuffer *cb);
extern camera_data* cbWriteUsing(CircularBuffer *cb) ;
extern int cbIsFull(CircularBuffer *cb) ; 
extern int cbIsEmpty(CircularBuffer *cb) ;

#endif



