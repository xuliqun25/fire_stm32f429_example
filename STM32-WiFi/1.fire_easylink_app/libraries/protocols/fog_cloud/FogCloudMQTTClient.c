/**
******************************************************************************
* @file    FogCloudMQTTClient.c
* @author  Eshen Wang
* @version V0.1.0
* @date    21-Nov-2014
* @brief   This file contains functions implementation of MQTT client based
           on MICO platform.
  operation
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
******************************************************************************
*/

#include "MICO.h"
//#include "MICONotificationCenter.h"
#include "FogCloudMQTTClient.h"

//#ifdef debug_out
//#define  _debug_out debug_out
//#else
#define _debug_out(format, ...) do {;}while(0)

#define mico_mqtt_client_log(M, ...) custom_log("MQTTClient", M, ##__VA_ARGS__)
#define mico_mqtt_client_log_trace() custom_log_trace("MQTTClient")
//#endif

/*******************************************************************************
 * VARIABLES
 ******************************************************************************/

static mqtt_client_context_t mqttClientContext = {0};
//static mico_mutex_t mqttClientContext_mutex = NULL;

mico_thread_t mqttClientThreadHandle = NULL;

//low level mqtt client
static Network n;
static Client c;

/*******************************************************************************
* FUNCTIONS
*******************************************************************************/

static void mqttClientThread(void *arg);
static OSStatus internal_FogCloudMQTTClientPublishto(const char* topic,
                                                      const unsigned char* msg, int msglen);
#ifndef ECS_NO_SOCKET_LOOPBACK
static OSStatus internal_FogCloudMQTTClientPublish(const unsigned char* msg, int msglen);
static OSStatus internal_format_topic_msg_buf(unsigned char** outDataBuffer, int* outDataLen,
                                              const char* pubTopic, int pubTopicLen, bool level_flag,
                                              const unsigned char* sendMsg, int sendMsgLen);
static OSStatus internal_parse_topic_msg(const unsigned char* recvDataBuffer, int recvDataLen,
                                    char** recvTopic, int* recvTopicLen,
                                    unsigned char* level_flag,
                                    unsigned char** recvMsg, int* recvMsgLen);
#endif

/*******************************************************************************
* IMPLEMENTATIONS
*******************************************************************************/

void FogCloudMQTTClientInit(mqtt_client_config_t init)
{
  mico_mqtt_client_log("mqtt client init");
  //if(mqttClientContext_mutex == NULL)
  //  mico_rtos_init_mutex( &mqttClientContext_mutex );

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_config_info = init;
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );
}

OSStatus FogCloudMQTTClientStart(void)
{
  return mico_rtos_create_thread(&mqttClientThreadHandle,
                                 MICO_APPLICATION_PRIORITY,
                                 "mqtt client", mqttClientThread,
                                 STACK_SIZE_MQTT_CLIENT_THREAD, NULL);
}

/* MQTT client callback function for message arrived
 * this funcion will call user registered callback function at last.
 */
void messageArrived(MessageData* md)
{
  //MQTT client msg handle
  MQTTMessage* message = md->message;
  //MQTTString* topic = md->topicName;
  mico_mqtt_client_log("messageArrived: [%.*s]\t [%.*s]",
                       md->topicName->lenstring.len, md->topicName->lenstring.data,
                       (int)message->payloadlen, (char*)message->payload);

  //call user registered handler
  mqttClientContext.client_config_info.hmsg(mqttClientContext.client_config_info.context,
                                            md->topicName->lenstring.data,
                                            (unsigned int)md->topicName->lenstring.len,
                                            (unsigned char*)message->payload,
                                            (unsigned int)message->payloadlen);
}

static void mqttClientThread(void *arg)
{
  mico_mqtt_client_log_trace();
  //mico_Context_t *Context = arg;

  fd_set readfds;
  struct timeval_t t;

  int retry_cnt = 0;

#ifndef ECS_NO_SOCKET_LOOPBACK
  OSStatus err = kUnknownErr;
  int recv_data_loopBack_fd = -1;
  struct sockaddr_t addr;
  unsigned char recvDataBuffer[MAX_PLAYLOAD_SIZE] = {0};
  int recvDataLen = 0;
  char* recvPubTopic = NULL;
  int recvPubTopicLen = 0;
  unsigned char* recvMsg = NULL;
  int recvMsgLen = 0;
  unsigned char level_flag = 0;
  char finalPubTopic[MAX_SIZE_MQTT_PUBLISH_TOPIC] = {0};
#endif

  int rc = -1;
  unsigned char buf[DEFAULT_MICO_MQTT_BUF_SIZE];
  unsigned char readbuf[DEFAULT_MICO_MQTT_READBUF_SIZE];

  MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
  MQTTPacket_connectData connectDataCopy = MQTTPacket_connectData_initializer;

  mico_mqtt_client_log("mqttClientThread start...");

  /* socket connect */
  NewNetwork(&n);

MQTTClientRestart:
  retry_cnt = 0;
  memset(&c, 0, sizeof(c));
  memset(buf, 0, sizeof(buf));
  memset(readbuf, 0, sizeof(readbuf));
  memset(&connectData, 0, sizeof(connectData));
  memcpy(&connectData, &connectDataCopy, sizeof(connectData));

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STARTED;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

 #ifndef ECS_NO_SOCKET_LOOPBACK
  /* recv data loopback socket */
  if(-1 != recv_data_loopBack_fd){
    close(recv_data_loopBack_fd);
    recv_data_loopBack_fd = -1;
  }
  recv_data_loopBack_fd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
  require_action(IsValidSocket( recv_data_loopBack_fd ), MQTTClientRestart, err = kNoResourcesErr );
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = RECVED_DATA_LOOPBACK_PORT;
  err = bind( recv_data_loopBack_fd, &addr, sizeof(addr) );
  require_noerr( err,  MQTTClientRestart);
#endif

  t.tv_sec = 0;
  t.tv_usec = DEFAULT_MICO_MQTT_YIELD_TMIE*1000;

  /* 1. socket connect */
  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == FogCloudMQTTClientState()){
      goto client_stop;
    }

    mico_mqtt_client_log("MQTT socket network connect...");
    rc = ConnectNetwork(&n, mqttClientContext.client_config_info.host,
                        mqttClientContext.client_config_info.port);
    if (rc >= 0){
      retry_cnt = 0;
      break;
    }

    mico_mqtt_client_log("MQTT socket network connect failed, rc = %d", rc);
    retry_cnt++;
    if(retry_cnt >= 5){
      retry_cnt = 0;
      goto MQTT_disconnected;
    }
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT socket network connect OK!");

  /* 2. send mqtt client connect request */
  MQTTClient(&c, &n, DEFAULT_MICO_MQTT_CMD_TIMEOUT,
             buf, DEFAULT_MICO_MQTT_BUF_SIZE,
             readbuf, DEFAULT_MICO_MQTT_READBUF_SIZE);

  connectData.willFlag = 0;
  connectData.MQTTVersion = 4;
  connectData.clientID.cstring = mqttClientContext.client_config_info.client_id;
  connectData.username.cstring = mqttClientContext.client_config_info.username;
  connectData.password.cstring = mqttClientContext.client_config_info.password;
  connectData.keepAliveInterval = mqttClientContext.client_config_info.keepAliveInterval;
  connectData.cleansession = 1;

  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == FogCloudMQTTClientState()){
      goto client_stop;
    }

    mico_mqtt_client_log("MQTT client connect...");
    rc = MQTTConnect(&c, &connectData);
    if(EC_SUCCESS == rc){
      retry_cnt = 0;
      break;
    }

    mico_mqtt_client_log("MQTT client connect failed, rc = %d\r\nRetry ...", rc);
    retry_cnt++;
    if(retry_cnt >= 5){
      retry_cnt = 0;
      goto MQTT_disconnected;
    }
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT client connect OK!");

  /* 3. subscribe request */
  //mico_mqtt_client_log("subscribing to %s", mqtt_client_config.subtopic);
  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == FogCloudMQTTClientState()){
      goto client_stop;
    }

    //mico_mqtt_client_log("MQTT client subscribe [%s]", mqttClientContext.client_config_info.subtopic);
    rc = MQTTSubscribe(&c, mqttClientContext.client_config_info.subtopic,
                       mqttClientContext.client_config_info.subscribe_qos ,
                       messageArrived);
    if (EC_SUCCESS == rc){
      retry_cnt = 0;
      break;
    }

    mico_mqtt_client_log("MQTT client subscribe [%s] failed, rc = %d\r\nRetry ...",
                         mqttClientContext.client_config_info.subtopic, rc);
    retry_cnt++;
    if(retry_cnt >= 5){
      retry_cnt = 0;
      goto MQTT_disconnected;
    }
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT client subscribed [%s] OK!", mqttClientContext.client_config_info.subtopic);

  /* 4. set running state */
  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_CONNECTED;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

  while (1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == FogCloudMQTTClientState()){
      mico_mqtt_client_log("MQTT stop by user.");
      goto client_stop;
    }

    /* recv data both from loopback and mqtt server */
    FD_ZERO(&readfds);
#ifndef ECS_NO_SOCKET_LOOPBACK
    FD_SET(recv_data_loopBack_fd, &readfds);
#endif
    FD_SET(c.ipstack->my_socket, &readfds);

    select(1, &readfds, NULL, NULL, &t);

#ifndef ECS_NO_SOCKET_LOOPBACK
    /* recv loopback data to publish */
    //err = kNoErr;
    if (FD_ISSET( recv_data_loopBack_fd, &readfds )) {
      memset(recvDataBuffer, 0, MAX_PLAYLOAD_SIZE);
      recvDataLen = recv( recv_data_loopBack_fd, (void*)(&recvDataBuffer[0]), MAX_LOOPBACK_BUF_SIZE, 0 );
      if(recvDataLen > 0){
        //err = internal_FogCloudMQTTClientPublish((unsigned char*)recvDataBuffer, recvDataLen);
        // parse pubtopic && msg
        err = internal_parse_topic_msg((const unsigned char*)(&recvDataBuffer[0]), recvDataLen,
                                       &recvPubTopic, &recvPubTopicLen, &level_flag, &recvMsg, &recvMsgLen);
        //mico_mqtt_client_log("DATA_TO_PUBLISH[%d][%.*s]", recvMsgLen, recvMsgLen, recvMsg);
        if(kNoErr == err){
          if(0 == recvPubTopicLen){
            // default topic "device_id/out"
            mico_mqtt_client_log("Publish to /out.");
            err = internal_FogCloudMQTTClientPublish((unsigned char*)recvMsg, recvMsgLen);
          }
          else if (recvPubTopicLen > 0){
            if(1 == level_flag){
              //mico_mqtt_client_log("Publish to sub topic, recvMsgLen=%d.", recvMsgLen);
              // publish to sub-level "device_id/out/<level>"
              memset(finalPubTopic, 0, MAX_SIZE_MQTT_PUBLISH_TOPIC);
              sprintf(finalPubTopic, "%s/%s",
                      mqttClientContext.client_config_info.pubtopic, recvPubTopic);
              err = internal_FogCloudMQTTClientPublishto((const char*)finalPubTopic,
                                                          (unsigned char*)recvMsg,
                                                          recvMsgLen);
            }
            else{
              // publish to user-defined topic
              //mico_mqtt_client_log("Publish to user topic.");
              err = internal_FogCloudMQTTClientPublishto((const char*)recvPubTopic,
                                                          (unsigned char*)recvMsg,
                                                          recvMsgLen);
            }
          }
          else{}
        }

        if(kNoErr != err){
          mico_mqtt_client_log("ERROR: Publish data failed, err=%d.", err);
          if(kConnectionErr == err){
            mico_mqtt_client_log("ERROR: MQTT client connection error, err=%d.", err);
            goto MQTT_disconnected;
          }
        }

        // free buffer
        if(NULL != recvPubTopic){
          free(recvPubTopic);
        }
        if(NULL != recvMsg){
          free(recvMsg);
        }
      }
    }
#endif

    /* MQTT recv */
    if (FD_ISSET( c.ipstack->my_socket, &readfds )){
      //mico_mqtt_client_log("MQTT client running...");
      rc = MQTTYield(&c, (int)DEFAULT_MICO_MQTT_YIELD_TMIE);  //keepalive failed will return EC_FAILURE
      if (EC_SUCCESS != rc) {
        goto MQTT_disconnected;
      }
    }
    else{
      /* MQTT ping */
      rc = keepalive(&c);
      if (EC_SUCCESS != rc) {
        goto MQTT_disconnected;
      }
    }
    continue;

  MQTT_disconnected:
    MQTTDisconnect(&c);
    n.disconnect(&n);

    //mico_rtos_lock_mutex( &mqttClientContext_mutex );
    mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_DISCONNECTED;
    //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

    mico_mqtt_client_log("MQTT client disconnected,reconnect ...");
    //mico_thread_sleep(1);
    goto MQTTClientRestart;
  }

  /* stop */
client_stop:
#ifndef ECS_NO_SOCKET_LOOPBACK
  if(-1 != recv_data_loopBack_fd){
    close(recv_data_loopBack_fd);
    recv_data_loopBack_fd = -1;
  }
#endif
  MQTTDisconnect(&c);
  n.disconnect(&n);

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

  mico_mqtt_client_log("MQTT client stopped.");

  mico_rtos_delete_thread(NULL);
  mqttClientThreadHandle = NULL;
  return;
}

#ifndef ECS_NO_SOCKET_LOOPBACK

/*******************************************************************************
 *
 ******************************************************************************/

// format_topic && msg string
// [0xAA][totalLen][pubtopicLen][sub-level][pubtopic][msg][CRC16][0x55]
//    1       4         4            1         0~n    1~n    2      1
// NOTE: outDatatBuffer MUST be freed by user.
//
// pubTopicLen:
//               > 0 :  user-defined topic or sub-level
//               = 0 :  default topic "device_id/out"
// level_flag flag:
//               0   :  user-defined topic
//               1   :  add sub-level to default topic "device_id/out/<level>"
static OSStatus internal_format_topic_msg_buf(unsigned char** outDataBuffer, int* outDataLen,
                                              const char* pubTopic, int pubTopicLen, bool level_flag,
                                              const unsigned char* sendMsg, int sendMsgLen)
{
  //OSStatus err = kUnknownErr;
  unsigned char* ptr = NULL;

  // check params
  if((NULL == sendMsg) || (0 >= sendMsgLen)){
       return kParamErr;
  }

  // not set pubTopic, use default: "device_id/out"
  if(NULL == pubTopic){
    pubTopicLen = 0;
  }

  // malloc mem
  *outDataLen = 10 + pubTopicLen + sendMsgLen + 3;
  *outDataBuffer = (unsigned char*)malloc(*outDataLen+1);
  if(NULL == *outDataBuffer){
    return kNoMemoryErr;
  }
  memset(*outDataBuffer, 0, (*outDataLen)+1);

  // set buf
  ptr = *outDataBuffer;
  *((unsigned char*)ptr) = 0xAA;
  ptr += 1;
  *((int*)ptr) = pubTopicLen + sendMsgLen;
  ptr += 4;
  *((int*)ptr) = pubTopicLen;
  ptr += 4;
  *((unsigned char*)ptr) = (level_flag ? 1:0);  // sub-level topic flag
  ptr += 1;
  if(pubTopicLen > 0){
    memcpy(ptr, pubTopic, pubTopicLen);
    ptr += pubTopicLen;
  }
  memcpy(ptr, sendMsg, sendMsgLen);
  ptr += sendMsgLen;
  *((short*)ptr) = 0xFFFF;
  ptr += 2;
  *((unsigned char*)ptr) = 0x55;

  return kNoErr;
}

// parse topic && msg from recv buffer
// NOTE: recvTopic && recvMsg buffer MUST be freed by user.
// level_flag: 1 == sub-level topic;  0 == normal topic
static OSStatus internal_parse_topic_msg(const unsigned char* recvDataBuffer, int recvDataLen,
                                    char** recvTopic, int* recvTopicLen,
                                    unsigned char* level_flag,
                                    unsigned char** recvMsg, int* recvMsgLen)
{
  //OSStatus err = kUnknownErr;
  unsigned char* ptr = NULL;
  int totalLen = 0;  // topicLen + msgLen

  // check params
  if((NULL == recvDataBuffer) || (0 >= recvDataLen)){
       return kParamErr;
  }

  // check frame header
  ptr = (unsigned char*)recvDataBuffer;
  if( (*ptr) != 0xAA ){
    ptr = NULL;
    return kFormatErr;
  }
  ptr += 1;

  // get total len
  totalLen = *((int*)ptr);
  if(totalLen <= 0){
    return kParamErr;
  }
  ptr += 4;

  // get topic len
  *recvTopicLen = *((int*)ptr);
  if((*recvTopicLen < 0) || (*recvTopicLen >= totalLen)){
    return kParamErr;
  }
  ptr += 4;

  // get msg len
  *recvMsgLen = totalLen - (*recvTopicLen);

  // check CRC
  // not do it now, just 0xFFFF
/*  if( (*((unsigned char*)(ptr + totalLen)) != 0xFF) ||  (*((unsigned char*)(ptr + totalLen + 1)) != 0xFF)){
    ptr = NULL;
    return kFormatErr;
  }

  // check tail
  if( (*((unsigned char*)(ptr + totalLen + 2))) != 0x55 ){
    ptr = NULL;
    return kFormatErr;
  }
  */

  // get sub-level flag
  *level_flag = *((unsigned char*)ptr);
  ptr += 1;

  // get topic
  if(*recvTopicLen > 0){
    *recvTopic = (char*)malloc(*recvTopicLen + 1);
    if(NULL == *recvTopic){
      return kNoMemoryErr;
    }
    memset(*recvTopic, 0, *recvTopicLen + 1);
    memcpy(*recvTopic, ptr, *recvTopicLen);
    ptr += (*recvTopicLen);
  }
  else{
    *recvTopic = NULL;
  }

  // get msg
  *recvMsg = (unsigned char*)malloc(*recvMsgLen + 1);
  if(NULL == *recvMsg){
    return kNoMemoryErr;
  }
  memset(*recvMsg, 0, *recvMsgLen + 1);
  memcpy(*recvMsg, ptr, *recvMsgLen);
  ptr += (*recvMsgLen);

  return kNoErr;
}

/****************************************
 * Loopback publish data to MQTT thread
 ***************************************/
static OSStatus internal_loopbackMsg(const unsigned char *sendBuf, int sendBufLen)
{
  OSStatus err = kUnknownErr;
  int send_loopback_fd = -1;
  struct sockaddr_t addr;
  int ret = 0;

  if (NULL == sendBuf || 0 >= sendBufLen){
    return kParamErr;
  }

  /* use loopback socket send data to MQTT clent thread */
  send_loopback_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  if(send_loopback_fd < 0)
    return kNoResourcesErr;
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = SEND_DATA_LOOPBACK_PORT;
  ret = bind(send_loopback_fd, &addr, sizeof(addr));
  if(0 != ret){
    close(send_loopback_fd);
    send_loopback_fd = -1;
    return kNoResourcesErr;
  }

  //mico_mqtt_client_log("LOOPBACK_SEND:[%d][%s]", sendBufLen, sendBuf);
  addr.s_port = RECVED_DATA_LOOPBACK_PORT;
  ret = sendto(send_loopback_fd, sendBuf, sendBufLen, 0, &addr, sizeof(addr));
  if(ret != sendBufLen){
    err = kWriteErr;
  }
  else{
    err = kNoErr;
  }

  if(-1 != send_loopback_fd){
    close(send_loopback_fd);
    send_loopback_fd = -1;
  }

  return err;
}

#endif  // NEED_SOCKET_LOOPBACK

/****************************************
 * Publish msg to "device_id/out"
 ***************************************/
OSStatus FogCloudMQTTClientPublish(const unsigned char* msg, int msglen)
{
  OSStatus err = kUnknownErr;
#ifndef ECS_NO_SOCKET_LOOPBACK
  unsigned char* sendBuf = NULL;
  int sendBufLen = 0;
#endif

  if (NULL == msg || 0 >= msglen || msglen > MAX_PLAYLOAD_SIZE){
    return kParamErr;
  }

  // check MQTT client connect status
  if(0 == c.isconnected)
    return kStateErr;

#ifndef ECS_NO_SOCKET_LOOPBACK
  // format topic && msg string with default pubTopic, MUST be freed later.
  err = internal_format_topic_msg_buf(&sendBuf, &sendBufLen,
                                        NULL, 0, false, msg, msglen);
  if(kNoErr != err){
    return kNoMemoryErr;
  }
  //mico_mqtt_client_log("LOOPBACK_FORMAT:[%d][%s]", sendBufLen, sendBuf+10);

  // send loopback msg
  err = internal_loopbackMsg(sendBuf ,sendBufLen);

  // free sendBuf
  if(NULL != sendBuf){
    free(sendBuf);
  }
#else
  err = internal_FogCloudMQTTClientPublishto(mqttClientContext.client_config_info.pubtopic,
                                              msg, msglen);
#endif

  return err;
}

/****************************************
 * Publish msg to user-defined topic
 ***************************************/
OSStatus FogCloudMQTTClientPublishto(const char* topic,
                                      const unsigned char* msg, int msglen)
{
  OSStatus err = kUnknownErr;
#ifndef ECS_NO_SOCKET_LOOPBACK
  unsigned char* sendBuf = NULL;
  int sendBufLen = 0;
#endif

  if (NULL == topic || NULL == msg || 0 >= msglen || msglen > MAX_PLAYLOAD_SIZE){
    return kParamErr;
  }

  if(0 == c.isconnected)
    return kStateErr;

#ifndef ECS_NO_SOCKET_LOOPBACK
  // format msg string with pubtopic
  err = internal_format_topic_msg_buf(&sendBuf, &sendBufLen,
                                      topic, strlen(topic), false, msg, msglen);
  if(kNoErr != err){
    return kNoMemoryErr;
  }
  //mico_mqtt_client_log("LOOPBAKC_FORMAT:[%d][%s]", sendBufLen, sendBuf+10);

  // send loopback msg
  err = internal_loopbackMsg(sendBuf ,sendBufLen);

  if(NULL != sendBuf){
    free(sendBuf);
  }
#else
  err = internal_FogCloudMQTTClientPublishto(topic, msg, msglen);
#endif

  return err;
}

/****************************************
 * Publish msg to "device_id/out/<CHANNEL>"
 ***************************************/
OSStatus FogCloudMQTTClientPublishtoChannel(const char* channel,
                                 const unsigned char *msg, unsigned int msglen)
{
  OSStatus err = kUnknownErr;
#ifndef ECS_NO_SOCKET_LOOPBACK
  unsigned char* sendBuf = NULL;
  int sendBufLen = 0;
#else
  char finalPubTopic[MAX_SIZE_MQTT_PUBLISH_TOPIC] = {0};
#endif

  if (NULL == channel || NULL == msg || 0 >= msglen || msglen > MAX_PLAYLOAD_SIZE){
    return kParamErr;
  }

  if(0 == c.isconnected)
    return kStateErr;

#ifndef ECS_NO_SOCKET_LOOPBACK
  // format msg string with sub-level, level_flag set true
  err = internal_format_topic_msg_buf(&sendBuf, &sendBufLen,
                                      channel, strlen(channel), true, msg, msglen);
  if(kNoErr != err){
    return kNoMemoryErr;
  }
  //mico_mqtt_client_log("LOOPBACK_FORMAT:[%d][%s]", sendBufLen, sendBuf+10);

  // send loopback msg
  err = internal_loopbackMsg(sendBuf, sendBufLen);

  if(NULL != sendBuf){
    free(sendBuf);
  }
#else
  // add channel to topic
  memset(finalPubTopic, 0, MAX_SIZE_MQTT_PUBLISH_TOPIC);
  sprintf(finalPubTopic, "%s/%s",
          mqttClientContext.client_config_info.pubtopic, channel);
  err = internal_FogCloudMQTTClientPublishto(finalPubTopic, msg, msglen);
#endif

  return err;
}

#ifndef ECS_NO_SOCKET_LOOPBACK

// use default MQTT client config pubtopic "device_id/out"
static OSStatus internal_FogCloudMQTTClientPublish(const unsigned char* msg, int msglen)
{
  OSStatus err = kUnknownErr;
  err = internal_FogCloudMQTTClientPublishto(mqttClientContext.client_config_info.pubtopic,
                                              msg, msglen);
  return err;
}

#endif  // ! ECS_NO_SOCKET_LOOPBACK

// use user-defined topic
static OSStatus internal_FogCloudMQTTClientPublishto(const char* topic,
                                                      const unsigned char* msg,
                                                      int msglen)
{
  OSStatus err = kUnknownErr;
  int ret = 0;

  MQTTMessage publishData =  MQTTMessage_publishData_initializer;

  if(topic == NULL || msglen <= 0 || msglen > MAX_PLAYLOAD_SIZE)
    return kParamErr;

  if(0 == c.isconnected)
    return kConnectionErr;

  //upload data qos1: at most once.
  publishData.qos = QOS0;
  publishData.payload = (void*)msg;
  publishData.payloadlen = msglen;

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  ret = MQTTPublish(&c, topic, &publishData);
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

  if (EC_SUCCESS == ret){
    err = kNoErr;
  }
  else if(EC_SOCKET_ERR == ret){
    err = kConnectionErr;
  }
  else{
    err = kUnknownErr;
  }

  return err;
}

OSStatus FogCloudMQTTClientStop(void)
{
  OSStatus err = kNoErr;

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

//  MQTTDisconnect(&c);
//  n.disconnect(&n);

  if (NULL != mqttClientThreadHandle) {
    err = mico_rtos_thread_join(&mqttClientThreadHandle);
    if (kNoErr != err)
      return err;
  }

  if (NULL != mqttClientThreadHandle) {
    err = mico_rtos_delete_thread(&mqttClientThreadHandle);
    if (kNoErr != err){
      return err;
    }
    mqttClientThreadHandle = NULL;
  }

  if (kNoErr == err){
    mico_mqtt_client_log("MQTT client stopped ok.");
  }

  return err;
}

mqttClientState FogCloudMQTTClientState(void)
{
  mqttClientState state = MQTT_CLIENT_STATUS_STOPPED;

  //mico_rtos_lock_mutex( &mqttClientContext_mutex );
  state = mqttClientContext.client_status.state;
  //mico_rtos_unlock_mutex( &mqttClientContext_mutex );

  return state;
}


//TODO ???

//OSStatus FogCloudMQTTClientSubscribe(const char* subtopic, enum QoS qos, messageHandler hmsg)
//{
//  OSStatus err = kUnknownErr;
//
//  return err;
//}
//
//OSStatus FogCloudMQTTClientUnsubscribe (const char* unsubtopic)
//{
//  OSStatus err = kUnknownErr;
//
//  return err;
//}


