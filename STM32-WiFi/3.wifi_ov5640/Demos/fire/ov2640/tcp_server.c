/**
******************************************************************************
* @file    tcp_server.c
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   First MiCO application to say hello world!
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "MICO.h"

#include "camera_data_queue.h"


extern CircularBuffer cam_circular_buff;

#define tcp_server_log(M, ...) custom_log("TCP", M, ##__VA_ARGS__)

#define SERVER_PORT 20000 /*set up a tcp server,port at 20000*/

#define NO_USED_BUFF_LEN    (1024)
#define TCP_MAX_SEND_SIZE   (1024*50)

OSStatus jpeg_send( int fd, const uint8_t *inBuf, size_t inBufLen )
{
    OSStatus err = kParamErr;
    ssize_t writeResult;
    int selectResult;
    size_t numWritten;
    fd_set writeSet;
    struct timeval_t t;

    require( fd>=0, exit );
    require( inBuf, exit );
    require( inBufLen, exit );

    err = kNotWritableErr;

    t.tv_sec = 5;
    t.tv_usec = 0;
    numWritten = 0;

    while( numWritten < inBufLen )
    {
        FD_ZERO( &writeSet );
        FD_SET( fd, &writeSet );

        selectResult = select( fd + 1, NULL, &writeSet, NULL,  &t );
        require( selectResult >= 1, exit );

        if(FD_ISSET( selectResult, &writeSet ))
        {
            writeResult = write( fd, (void *)( inBuf + numWritten ), ( inBufLen - numWritten ) );
            require( writeResult > 0, exit );

            numWritten += writeResult;
        }
    }

    require_action( numWritten == inBufLen, exit, tcp_server_log("ERROR: Did not write all the bytes in the buffer. BufLen: %zu, Bytes Written: %zu", inBufLen, numWritten ); err = kUnderrunErr );

    err = kNoErr;

exit:
    return err;
}

OSStatus jpeg_tcp_send( int fd, const uint8_t *inBuf, size_t inBufLen )
{
    uint32_t i = 0, count = 0, index = 0;
    OSStatus err = kGeneralErr;

    count =  inBufLen / TCP_MAX_SEND_SIZE;

    for(i = 0; i < count; i ++)
    {
         err = jpeg_send(fd, inBuf + index, TCP_MAX_SEND_SIZE);
         require( err == kNoErr, exit);
         index = index + TCP_MAX_SEND_SIZE;
    }

    if((inBufLen % TCP_MAX_SEND_SIZE) != 0)
    {
        err = jpeg_send(fd, inBuf + index, inBufLen % TCP_MAX_SEND_SIZE);
        require( err == kNoErr, exit);
    }

 exit:
    return err;
}


void jpeg_socket_close(int* fd)
{
    int tempFd = *fd;
    if ( tempFd < 0 )
        return;
    *fd = -1;
    close(tempFd);
}


/* TCP server listener thread */
void tcp_server_thread( void *arg )
{
    OSStatus err = kNoErr;
    struct sockaddr_t server_addr,client_addr;
    socklen_t sockaddr_t_size = sizeof( client_addr );
    char  client_ip_str[16];
    int tcp_listen_fd = -1, client_fd = -1;
    IPStatusTypedef para;
    int32_t camera_data_len = 0, i = 0;
    uint8_t *in_camera_data = NULL;
    uint8_t packet_index = 0;
    uint8_t *no_used_buff = NULL;

    micoWlanGetIPStatus(&para, Station);
    tcp_server_log("TCP server ip:%s, port:%d", para.ip, SERVER_PORT);     //打印本地IP和端口

    tcp_listen_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    require_action(IsValidSocket( tcp_listen_fd ), exit, err = kNoResourcesErr );

    server_addr.s_ip =  INADDR_ANY;  /* Accept conenction request on all network interface */
    server_addr.s_port = SERVER_PORT;/* Server listen on port: 20000 */
    err = bind( tcp_listen_fd, &server_addr, sizeof( server_addr ) );
    require_noerr( err, exit );

    err = listen( tcp_listen_fd, 0);
    require_noerr( err, exit );

    no_used_buff = malloc(NO_USED_BUFF_LEN);
		memset(no_used_buff, 0, NO_USED_BUFF_LEN);

    require_noerr( err, exit );

    while(1)
    {
        client_fd = accept( tcp_listen_fd, &client_addr, &sockaddr_t_size );
        if( IsValidSocket( client_fd ) )
        {
            inet_ntoa( client_ip_str, client_addr.s_ip );
            tcp_server_log( "TCP Client %s:%d connected, fd: %d", client_ip_str, client_addr.s_port, client_fd );

//            //1.打开摄像头
//            DCMI_Cmd(ENABLE);

            while(1)
            {
                //2.数据出队列
                err = pull_data_from_queue(&in_camera_data, &camera_data_len);
                if(err != kNoErr)
                {
                     //更新读指针		
					cbReadFinish(&cam_circular_buff);                  		
			        mico_thread_msleep(2);
                    continue;
                }

//                tcp_server_log("out->[%d]%d KB", packet_index, camera_data_len/1024);

                //3.发送数据
                if((err = jpeg_tcp_send(client_fd, (const uint8_t *)in_camera_data, camera_data_len)) != kNoErr)
                {
                    //更新读指针		
					cbReadFinish(&cam_circular_buff);

                    tcp_server_log("error-->[%d]%d KB", packet_index, camera_data_len/1024);
//                    clear_array_flag(packet_index);
                    break;
                }

                for(i = 0; i < 1; i ++)
                {
                    //4.发送间隔数据
                    if((err = jpeg_send(client_fd, (const uint8_t *)no_used_buff, NO_USED_BUFF_LEN)) != kNoErr)
                    {
                                            //更新读指针		
                        cbReadFinish(&cam_circular_buff);
                        tcp_server_log("error-->[%d]", packet_index);
//                        clear_array_flag(packet_index);
                        break;
                    }
                }

                                    //更新读指针		
				cbReadFinish(&cam_circular_buff);
//                tcp_server_log("send->[%d]%d KB", packet_index, camera_data_len/1024);

                //4.清除标志位
//                clear_array_flag(packet_index);
            }

            tcp_server_log("TCP Client disconnect %s:%d connected, fd: %d",client_ip_str, client_addr.s_port, client_fd);
            jpeg_socket_close( &client_fd );

//            //5.关闭摄像头
//            DCMI_Cmd(DISABLE);
        }
    }

 exit:
    if( err != kNoErr )
    {
        tcp_server_log( "Server listerner thread exit with err: %d", err );
    }

    jpeg_socket_close( &tcp_listen_fd );

    mico_rtos_delete_thread(NULL );
}
