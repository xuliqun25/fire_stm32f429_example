#include "mico.h"
#include "camera_data_queue.h"
#include "ov2640_Init.h"
#include "malloc.h"

#define camera_data_queue_log(M, ...) custom_log("camera_data_queue", M, ##__VA_ARGS__)

typedef struct _camera_data
{
    uint8_t index;
    mico_mutex_t  mutex;
    bool    empty; //是否为空. true为空，可以插入数据，false为不空，不能插入数据
    uint8_t *head;//头指针
}camera_data;

uint8_t *cam_global_buf = NULL;
camera_data  cam_data[CAMERA_QUEUE_NUM];
static mico_queue_t camera_os_queue = NULL;

int32_t find_jpeg_tail(uint8_t *data);
void camera_queue_free(void);
OSStatus camera_queue_init(void);
ssize_t find_usable_index(void);
OSStatus push_data_to_queue(void);
OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p, uint8_t *array_index);
void clear_array_flag(uint8_t index);

//摄像头队列的指针指向的缓冲区全部销毁
void camera_queue_free(void)
{
    uint32_t i = 0;

    for(i = 0; i < CAMERA_QUEUE_NUM; i ++)
    {
        if(cam_data[i].head != NULL)
        {
            free(cam_data[i].head);
            cam_data[i].head = NULL;
        }
    }

    return;
}

//缓冲队列初始化，分配内存
OSStatus camera_queue_init(void)
{
    uint32_t i = 0;
    OSStatus err = kNoErr;

    cam_global_buf = sdram_malloc(CAMERA_QUEUE_DATA_LEN);
    if(cam_global_buf == NULL)
    {
        camera_data_queue_log("malloc camera buff error!");
        return  kGeneralErr;
    }
    memset(cam_global_buf, 0, CAMERA_QUEUE_DATA_LEN);

    memset(cam_data, 0, sizeof(cam_data));

    err = mico_rtos_init_queue(&camera_os_queue, "camera_os_queue", sizeof(camera_data),  CAMERA_QUEUE_NUM);
    if(err != kNoErr)
    {
        camera_data_queue_log("create camera os queue error");
        return  kGeneralErr;
    }

    for(i = 0; i < CAMERA_QUEUE_NUM; i ++)
    {
        cam_data[i].empty = true; //初始化时,该值为true,表明可插入数据，但是无有效数据可读

        cam_data[i].index = i;
        cam_data[i].head = sdram_malloc(CAMERA_QUEUE_DATA_LEN);
        camera_data_queue_log("cam_data[%d].head = 0x%08X", i, cam_data[i].head);

        if(i >= 1)
        {
            camera_data_queue_log("%d",cam_data[i - 1].head - cam_data[i].head);    //偏差
        }

        if(cam_data[i].head == NULL)
        {
            camera_data_queue_log("malloc %d queue error!", i);
            camera_queue_free();
            return  kGeneralErr;
        }
        mico_rtos_init_mutex(&(cam_data[i].mutex));
        memset(cam_data[i].head, 0, CAMERA_QUEUE_DATA_LEN);
    }

    return kNoErr;
}

//寻找规则：从前往后遍历未用的成员，返回成员下标,寻找不到则返回-1
ssize_t find_usable_index(void)
{
     uint32_t i = 0;
     bool status = false;

     for(i = 0; i < CAMERA_QUEUE_NUM; i ++)
     {
         mico_rtos_lock_mutex(&(cam_data[i].mutex));
         status = cam_data[i].empty;
         mico_rtos_unlock_mutex(&(cam_data[i].mutex));

         if(status == true)
             return i;
     }

     return -1;
}

//成功返回:kNoErr,失败返回 kGeneralErr
OSStatus push_data_to_queue(void)
{
    OSStatus err = kNoErr;
    ssize_t index = -1;

    err = mico_rtos_is_queue_full(&camera_os_queue); //判断队列是否满
    if(err == false)
    {
        index = find_usable_index();     //寻找可用下标
        if(index != -1)
        {
//            memset(cam_data[index].head, 0, CAMERA_QUEUE_DATA_LEN);
            memcpy(cam_data[index].head, cam_global_buf, CAMERA_QUEUE_DATA_LEN);

            DCMI_Start();

            mico_rtos_lock_mutex(&(cam_data[index].mutex));
            cam_data[index].empty = false;
            mico_rtos_unlock_mutex(&(cam_data[index].mutex));

            camera_data_queue_log("push->[%d]", index);

            mico_rtos_push_to_queue(&camera_os_queue, &(cam_data[index]), MICO_WAIT_FOREVER);  //队列不满则插入数据

            return kNoErr;
        }
    }


    DCMI_Start();

    return kGeneralErr;
}

//从队列中取数据, data_p:起始地址, len_p:长度, array_index:下标
OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p, uint8_t *array_index)
{
    OSStatus err = kNoErr;
    camera_data in_cam_data;
    uint8_t real_index = 0;
    bool status = false;

    err = mico_rtos_is_queue_empty(&camera_os_queue);      //判断队列是否满
    if(err == false)
    {
        err = mico_rtos_pop_from_queue(&camera_os_queue, &in_cam_data, MICO_WAIT_FOREVER);  //队列不空则取数据
        if(err != kNoErr)
           return kGeneralErr;

        real_index = in_cam_data.index;

        mico_rtos_lock_mutex(&(cam_data[real_index].mutex));
        status = cam_data[real_index].empty;      //这个非常重要
        mico_rtos_unlock_mutex(&(cam_data[real_index].mutex));

        if(status == false)
        {
            *len_p = find_jpeg_tail(cam_data[real_index].head);
            if(*len_p != -1)
            {
                *data_p = cam_data[real_index].head;
                *array_index = real_index;
                return kNoErr;
            }else
            {
                mico_rtos_lock_mutex(&(cam_data[real_index].mutex));
                cam_data[real_index].empty = true;      //这个非常重要
                mico_rtos_unlock_mutex(&(cam_data[real_index].mutex));
                camera_data_queue_log("find error jpeg data, index = %d", real_index);
            }
        }
    }

    return kGeneralErr;
}

void clear_array_flag(uint8_t index)
{
     mico_rtos_lock_mutex(&(cam_data[index].mutex));
     cam_data[index].empty = true;
     mico_rtos_unlock_mutex(&(cam_data[index].mutex));

     return;
}

int32_t find_jpeg_tail(uint8_t *data)
{
    uint32_t i = 0;
    uint8_t *p = data;

    if((*p == 0xFF) && (*(p + 1) == 0xD8)) //帧头判断
    {
        for(i = 0; i < CAMERA_QUEUE_DATA_LEN - 2; i ++)
        {
            if((*p == 0xFF) && (*(p + 1) == 0xD8))
            {
                if(i == 0)
                {
                    p++;
                    continue;
                }else
                {
                    camera_data_queue_log("picture find other head at %d", i);
                    return -1;
                }
            }else if((*p == 0xFF) && (*(p + 1) == 0xD9))
            {
                return  i + 2;
            }

            p++;
        }
    }else
    {
        camera_data_queue_log("picture head error!");
        return -1;
    }

    camera_data_queue_log("picture tail error!");

    return -1;
}
