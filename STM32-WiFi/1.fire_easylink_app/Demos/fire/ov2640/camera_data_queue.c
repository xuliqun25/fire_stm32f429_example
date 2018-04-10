#include "mico.h"
#include "camera_data_queue.h"
#include "ov2640_Init.h"
#include "malloc.h"

#define camera_data_queue_log(M, ...) custom_log("camera_data_queue", M, ##__VA_ARGS__)

typedef struct _camera_data
{
    uint8_t index;
    mico_mutex_t  mutex;
    bool    empty; //�Ƿ�Ϊ��. trueΪ�գ����Բ������ݣ�falseΪ���գ����ܲ�������
    uint8_t *head;//ͷָ��
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

//����ͷ���е�ָ��ָ��Ļ�����ȫ������
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

//������г�ʼ���������ڴ�
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
        cam_data[i].empty = true; //��ʼ��ʱ,��ֵΪtrue,�����ɲ������ݣ���������Ч���ݿɶ�

        cam_data[i].index = i;
        cam_data[i].head = sdram_malloc(CAMERA_QUEUE_DATA_LEN);
        camera_data_queue_log("cam_data[%d].head = 0x%08X", i, cam_data[i].head);

        if(i >= 1)
        {
            camera_data_queue_log("%d",cam_data[i - 1].head - cam_data[i].head);    //ƫ��
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

//Ѱ�ҹ��򣺴�ǰ�������δ�õĳ�Ա�����س�Ա�±�,Ѱ�Ҳ����򷵻�-1
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

//�ɹ�����:kNoErr,ʧ�ܷ��� kGeneralErr
OSStatus push_data_to_queue(void)
{
    OSStatus err = kNoErr;
    ssize_t index = -1;

    err = mico_rtos_is_queue_full(&camera_os_queue); //�ж϶����Ƿ���
    if(err == false)
    {
        index = find_usable_index();     //Ѱ�ҿ����±�
        if(index != -1)
        {
//            memset(cam_data[index].head, 0, CAMERA_QUEUE_DATA_LEN);
            memcpy(cam_data[index].head, cam_global_buf, CAMERA_QUEUE_DATA_LEN);

            DCMI_Start();

            mico_rtos_lock_mutex(&(cam_data[index].mutex));
            cam_data[index].empty = false;
            mico_rtos_unlock_mutex(&(cam_data[index].mutex));

            camera_data_queue_log("push->[%d]", index);

            mico_rtos_push_to_queue(&camera_os_queue, &(cam_data[index]), MICO_WAIT_FOREVER);  //���в������������

            return kNoErr;
        }
    }


    DCMI_Start();

    return kGeneralErr;
}

//�Ӷ�����ȡ����, data_p:��ʼ��ַ, len_p:����, array_index:�±�
OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p, uint8_t *array_index)
{
    OSStatus err = kNoErr;
    camera_data in_cam_data;
    uint8_t real_index = 0;
    bool status = false;

    err = mico_rtos_is_queue_empty(&camera_os_queue);      //�ж϶����Ƿ���
    if(err == false)
    {
        err = mico_rtos_pop_from_queue(&camera_os_queue, &in_cam_data, MICO_WAIT_FOREVER);  //���в�����ȡ����
        if(err != kNoErr)
           return kGeneralErr;

        real_index = in_cam_data.index;

        mico_rtos_lock_mutex(&(cam_data[real_index].mutex));
        status = cam_data[real_index].empty;      //����ǳ���Ҫ
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
                cam_data[real_index].empty = true;      //����ǳ���Ҫ
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

    if((*p == 0xFF) && (*(p + 1) == 0xD8)) //֡ͷ�ж�
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
