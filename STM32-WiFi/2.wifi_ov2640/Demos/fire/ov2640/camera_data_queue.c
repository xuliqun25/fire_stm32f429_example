#include "mico.h"
#include "camera_data_queue.h"
#include "bsp_sdram.h"

#define camera_data_queue_log(M, ...) //custom_log("camera_data_queue", M, ##__VA_ARGS__)

camera_data  cam_data[CAMERA_QUEUE_NUM];
CircularBuffer cam_circular_buff;

//���л��������ڴ��
__align(4) uint8_t queue_buff[CAMERA_QUEUE_NUM][CAMERA_QUEUE_DATA_LEN] __EXRAM;



int32_t find_jpeg_tail(uint8_t *data,uint8_t *jpeg_start,int32_t search_point) ;
void camera_queue_free(void);
OSStatus camera_queue_init(void);
ssize_t find_usable_index(void);
OSStatus push_data_to_queue(void);
OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p);
void clear_array_flag(uint8_t index);




/*���λ������*/

/**
  * @brief  ��ʼ���������
  * @param  cb:������нṹ��
  * @param  size: ������е�Ԫ�ظ���
  * @note 	��ʼ��ʱ����Ҫ��cb->elemsָ�븳ֵ
  */
void cbInit(CircularBuffer *cb, int size) 
{
    cb->size  = size;	/* maximum number of elements           */
    cb->start = 0; 		/* index of oldest element              */
    cb->end   = 0; 	 	/* index at which to write new element  */
//    cb->elems = (uint8_t *)calloc(cb->size, sizeof(uint8_t));  //elems Ҫ�����ʼ��
}
 
/**
  * @brief  ���������е�ǰ��״̬��Ϣ
  * @param  cb:������нṹ��
  */
void cbPrint(CircularBuffer *cb) 
{
    camera_data_queue_log("size=0x%x, start=%d, end=%d\n", cb->size, cb->start, cb->end);
	  camera_data_queue_log("size=0x%x, start_using=%d, end_using=%d\n", cb->size, cb->start_using, cb->end_using);
}
 
/**
  * @brief  �жϻ��������(1)��(0)����
  * @param  cb:������нṹ��
  */
int cbIsFull(CircularBuffer *cb) 
{
    return cb->end == (cb->start ^ cb->size); /* This inverts the most significant bit of start before comparison */ 
}
 
/**
  * @brief  �жϻ��������(1)��(0)ȫ��
  * @param  cb:������нṹ��
  */		
int cbIsEmpty(CircularBuffer *cb) 
{
    return cb->end == cb->start; 
}

/**
  * @brief  �Ի�����е�ָ���1
  * @param  cb:������нṹ��
  * @param  p��Ҫ��1��ָ��
  * @return  ���ؼ�1�Ľ��
  */	
int cbIncr(CircularBuffer *cb, int p) 
{
    return (p + 1)&(2*cb->size-1); /* start and end pointers incrementation is done modulo 2*size */
}
 
/**
  * @brief  ��ȡ��д��Ļ�����ָ��
  * @param  cb:������нṹ��
  * @return  �ɽ���д��Ļ�����ָ��
  * @note  �õ�ָ���ɽ���д���������дָ�벻��������1��д������ʱ��Ӧ����cbWriteFinish
  */
camera_data* cbWrite(CircularBuffer *cb) 
{
    if (cbIsFull(cb)) /* full, overwrite moves start pointer */
    {
			return NULL;
		}		
		else
			cb->end_using = cbIncr(cb, cb->end); //δ����������1
		
	return  cb->elems[cb->end_using&(cb->size-1)];
}

/**
  * @brief  ��ȡ�ɵ�ǰ����ʹ�õ�д������ָ��
  * @param  cb:������нṹ��
  * @return  ��ǰ����ʹ�õ�д������ָ��
  */
camera_data* cbWriteUsing(CircularBuffer *cb) 
{
	return  cb->elems[cb->end_using&(cb->size-1)];
}

/**
  * @brief ����д����ϣ�����дָ�뵽����ṹ��
  * @param  cb:������нṹ��
  */
void cbWriteFinish(CircularBuffer *cb)
{
    cb->end = cb->end_using;
}
 
/**
  * @brief  ��ȡ�ɶ�ȡ�Ļ�����ָ��
  * @param  cb:������нṹ��
  * @return  �ɽ��ж�ȡ�Ļ�����ָ��
  * @note  �õ�ָ���ɽ����ȡ����������ָ�벻��������1����ȡ������ʱ��Ӧ����cbReadFinish
  */
camera_data* cbRead(CircularBuffer *cb) 
{
		if(cbIsEmpty(cb))
			return NULL;
		
	cb->start_using = cbIncr(cb, cb->start);
	return cb->elems[cb->start_using&(cb->size-1)];
}


/**
  * @brief ���ݶ�ȡ��ϣ����¶�ָ�뵽����ṹ��
  * @param  cb:������нṹ��
  */
void cbReadFinish(CircularBuffer *cb) 
{
    cb->start = cb->start_using;
}






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

  memset(cam_data, 0, sizeof(cam_data));
		
	/*��ʼ���������*/
	cbInit(&cam_circular_buff,CAMERA_QUEUE_NUM);

    for(i = 0; i < CAMERA_QUEUE_NUM; i ++)
    {
        cam_data[i].head = queue_buff[i];//cam_global_buf+CAMERA_QUEUE_DATA_LEN*i;
        
        /*��ʼ�����л���ָ�룬ָ��ʵ�ʵ��ڴ�*/
        cam_circular_buff.elems[i] = &cam_data[i];
        
        camera_data_queue_log("cam_data[i].head=0x%x,cam_circular_buff.elems[i] =0x%x", (uint32_t)cam_data[i].head,(uint32_t)cam_circular_buff.elems[i]->head);

        memset(cam_data[i].head, 0, CAMERA_QUEUE_DATA_LEN);
    }

    return kNoErr;
}





//�Ӷ�����ȡ����, data_p:��ʼ��ַ, len_p:����, array_index:�±�
OSStatus pull_data_from_queue(uint8_t **data_p, int32_t *len_p)
{
		int32_t len_p_get = 0 ;		
		uint8_t jpeg_start_offset = 0;	
		camera_data *cam_data_pull;	
		uint32_t time_old,time_new;
		
		if(!cbIsEmpty(&cam_circular_buff))//������зǿ�
		{
				/*�ӻ�������ȡ���ݣ����д���*/
				cam_data_pull = cbRead(&cam_circular_buff);
			
				if (cam_data_pull == NULL)
						return kGeneralErr;
#if 1				
			/*�����ļ�*/	
				
			*len_p = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,cam_data_pull->img_dma_len*2/3);

//			*len_p = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,0);

#elif 0		//���ڵ��ԣ��Աȿ��������ļ������ٵ�	����
				camera_data_queue_log("-------------------------------------");
				
				time_old = mico_get_time();			
				*len_p = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,cam_data_pull->img_dma_len*99/100);
				time_new = mico_get_time();			
				len_p_get =  cam_data_pull->img_real_len;
			
				camera_data_queue_log("find len = %d,get_len=%d",*len_p,len_p_get);
				camera_data_queue_log("find time = %d ",time_new-time_old);
			
				camera_data_queue_log("---****************-------------");
				time_old = mico_get_time();
				*len_p = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,0);

				time_new = mico_get_time();
			
				camera_data_queue_log("find len = %d,get_len=%d",*len_p,len_p_get);
				camera_data_queue_log("find time = %d ",time_new-time_old);
				
#else	//���ڵ��ԣ��Աȿ��������ļ������ٵ�	����
			
				{
							//��ĸ
					static uint8_t numerator = 99 ,denominator=100 ;

					
						time_old = mico_get_time();			
						len_p_get = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,cam_data_pull->img_dma_len*numerator/denominator);
						time_new = mico_get_time();						


			
						time_old = mico_get_time();
						*len_p = find_jpeg_tail(cam_data_pull->head,&jpeg_start_offset,0);
						time_new = mico_get_time();
						
						if(len_p_get != *len_p )
						{						

							camera_data_queue_log("--------------different-------------");
							camera_data_queue_log("fast find len = %d,find=%d",len_p_get,*len_p);
							camera_data_queue_log("img_real_len = %d",cam_data_pull->img_dma_len);
							
							//������ĸ�����ٳ���
							if(numerator != 1)
							{
								numerator--;
								denominator--;					
							}
							camera_data_queue_log("newly search point =  %d/%d",numerator,denominator);


						}
						
//						*len_p = len_p_get;
				}
			

#endif			
				
				if(*len_p != -1)
				{
						*data_p = cam_data_pull->head + jpeg_start_offset;
						return kNoErr;
				}else
				{
				}		
		}

    return kGeneralErr;
}

 


/**
  * @brief  ����jpeg�ļ�ͷ���ļ�β
  * @param  data:������ָ��
	* @param  jpeg_start:����jpeg�ļ�ͷ����������ƫ��
	* @param  search_point:����jpeg�ļ�βʱ������������������ʼλ��
  */

#define JPEG_HEAD_SEARCH_MAX 	40

int32_t find_jpeg_tail(uint8_t *data,uint8_t *jpeg_start,int32_t search_point) 
{
    uint32_t i = 0,j=0,z=0;
    uint8_t *p = data;
	
	/*Ĭ���ļ�ͷλ��*/
	*jpeg_start = 0;

	for(j = 0;j<JPEG_HEAD_SEARCH_MAX;j++)
	{
		if(! ((*p == 0xFF) && (*(p + 1) == 0xD8)) ) //������֡ͷ
		{
			p++;
		}
    else if((*p == 0xFF) && (*(p + 1) == 0xD8)) //֡ͷ�ж�
    {
			if(j!=0)			
			{
				camera_data_queue_log("JPEG_HEAD_SEARCH_MAX *p=0x%x,j=%d", *p,j );
				/*�ļ�ͷʵ��ƫ��*/
				*jpeg_start=j;
			}

				/*�������*/
				p += search_point;
			
        for(i = search_point ; i < CAMERA_QUEUE_DATA_LEN - 2; i ++)
        {
            if((*p == 0xFF) && (*(p + 1) == 0xD8))
            {
                if(i == 0)
                {
                    p++;
                    continue;
                }else
                {
                    return -1;
                }
            }else if((*p == 0xFF) && (*(p + 1) == 0xD9))
            {
								camera_data_queue_log("pic len = %d", i+2 );

                return  i + 2;
            }

            p++;
        }  	
				
				camera_data_queue_log("picture tail error!");
				return -1;				
    }
	}
#if 0 	//���ڵ��ԣ��鿴�ļ�ͷ�᲻������������ַ
	{
		extern uint32_t  XferTransferNumber;
		extern uint32_t 	XferSize ;

		for(j = 0;j<CAMERA_QUEUE_DATA_LEN;j++)
		{
			if(! ((*p == 0xFF) && (*(p + 1) == 0xD8)) ) //������֡ͷ
			{
				p++;

			}
		else if((*p == 0xFF) && (*(p + 1) == 0xD8)) //֡ͷ�ж�
		{
			camera_data_queue_log("555555555555555555555555555555555555555");
			camera_data_queue_log("found head at xfer z= %d ,p =0x%x",z,(int32_t)p);
			break;

		}
	 }
			
	}
#endif		
	
  camera_data_queue_log("picture head error!");	
	
  return -1;
}















/**
  * @brief  ����jpeg�ļ�ͷ���ļ�β
  * @param  data:������ָ��
	*	@return �����ҵ��ļ���kNoErr���쳣:kGeneralErr
  */
int32_t find_jpeg(camera_data *cambuf) 
{
	uint32_t i = 0,j=0;
	
	uint8_t *p = cambuf->head;	
	
	/*�����ļ�β���*/
	uint32_t end_search_point = cambuf->img_dma_len*99/100;
	
	/*����*/
	cambuf->img_real_start = NULL;	
	cambuf->img_real_len = -1;

	for(j = 0;j<JPEG_HEAD_SEARCH_MAX;j++)
	{
		if(! ((*p == 0xFF) && (*(p + 1) == 0xD8)) ) //������֡ͷ
		{
			p++;
		}
    else if((*p == 0xFF) && (*(p + 1) == 0xD8)) //֡ͷ�ж�
    {
			if(j!=0)			
			{	
				/*�ļ�ͷʵ��ƫ��*/
				camera_data_queue_log("JPEG_HEAD_SEARCH *p=0x%x,j=%d", *p,j );
			}
			
				/*�ļ�ͷʵ��ָ��*/
				cambuf->img_real_start = p;

				/*�ļ�β�������*/
				p += end_search_point;
			
        for(i = end_search_point ; i < CAMERA_QUEUE_DATA_LEN - 2; i ++)
        {
            if((*p == 0xFF) && (*(p + 1) == 0xD8))
            {
                if(i == 0)
                {
                    p++;
                    continue;
                }else
                {
                    return kGeneralErr;
                }
            }else if((*p == 0xFF) && (*(p + 1) == 0xD9))
            {
								camera_data_queue_log("pic len = %d", i+2 );

								cambuf->img_real_len = i+2;
                return  kNoErr;
            }

            p++;
        }  	
				
				camera_data_queue_log("picture tail error!");
				return kGeneralErr;				
    }
	}

	
  camera_data_queue_log("picture head error!");	
	
  return kGeneralErr;
}





