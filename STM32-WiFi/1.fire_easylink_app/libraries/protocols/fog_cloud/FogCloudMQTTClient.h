/**
******************************************************************************
* @file    FogCloudMQTTClient.h 
* @author  EShen Wang
* @version V1.0.0
* @date    21-Nov-2014
* @brief   This header contains function prototypes for MQTT client. based
           on MICO platform
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


#ifndef __FOGCLOUD_MQTT_CLIENT_H_
#define __FOGCLOUD_MQTT_CLIENT_H_

#include "Common.h"
#include "MQTTClient.h"

/*******************************************************************************
* DEFINES
*******************************************************************************/

/* if a socket fd can used in different thread, just use ECS_NO_SOCKET_LOOPBACK,
 * else use socket loopback to solve this problem.
 */
//#define ECS_NO_SOCKET_LOOPBACK                1

//in ms
#define DEFAULT_MICO_MQTT_YIELD_TMIE          1000
//in ms
#define DEFAULT_MICO_MQTT_CMD_TIMEOUT         3000
//in byte
#define MAX_SIZE_MQTT_SUBSCRIBE_TOPIC         256
#define MAX_SIZE_MQTT_PUBLISH_TOPIC           256

#define MAX_PLAYLOAD_SIZE                     2048
#define DEFAULT_MICO_MQTT_BUF_SIZE            (MAX_PLAYLOAD_SIZE + MAX_SIZE_MQTT_PUBLISH_TOPIC)
#define DEFAULT_MICO_MQTT_READBUF_SIZE        (1024 + MAX_SIZE_MQTT_SUBSCRIBE_TOPIC)
#define MAX_LOOPBACK_BUF_SIZE                 (DEFAULT_MICO_MQTT_BUF_SIZE + 15)   // 15 is loopback head && tail(13)
#define STACK_SIZE_MQTT_CLIENT_THREAD         0x2000

#define RECVED_DATA_LOOPBACK_PORT             9001
#define SEND_DATA_LOOPBACK_PORT               9002

/*******************************************************************************
* STRUCTURES
*******************************************************************************/

typedef void (*mqttMsgArrivedHandler)(void* context, 
                                      const char* topic,
                                      const unsigned int topicLen,
                                      unsigned char* Msg, unsigned int len);

typedef enum {
  MQTT_CLIENT_STATUS_STOPPED = 1,  //client stopped
  MQTT_CLIENT_STATUS_STARTED = 2,  //client start up
  MQTT_CLIENT_STATUS_CONNECTED = 3,  //client work ok
  MQTT_CLIENT_STATUS_DISCONNECTED = 4,  //client disconnect
} mqttClientState;

typedef struct _mqtt_client_config_t {
  /*server info*/
  char *host;
  uint16_t port;
  
  /*client opts*/
  char * client_id;
  enum QoS subscribe_qos;
  char *username;
  char *password;
  unsigned short keepAliveInterval;
  
  /*topics*/
  char *pubtopic;
  char *subtopic;
  
  /*message arrived callback*/
  mqttMsgArrivedHandler hmsg;
  //user context
  void* context;
} mqtt_client_config_t;

typedef struct _mqtt_client_status_t {
  mqttClientState state;
} mqtt_client_status_t;

typedef struct _mqtt_client_context_t {
  mqtt_client_config_t client_config_info;
  /* running status */
  mqtt_client_status_t client_status;
} mqtt_client_context_t;

/*******************************************************************************
* USER INTERFACES
*******************************************************************************/

void FogCloudMQTTClientInit(mqtt_client_config_t init);
OSStatus FogCloudMQTTClientStart(void);
OSStatus FogCloudMQTTClientStop(void);

OSStatus FogCloudMQTTClientPublish(const unsigned char* msg, int msglen);
//TODO: send to any topic, must add "device_id/in" by user
OSStatus FogCloudMQTTClientPublishto(const char* topic, 
                                      const unsigned char* msg, int msglen);
//TODO: send to sub-level "device_id/in/<level>"
OSStatus FogCloudMQTTClientPublishtoChannel(const char* channel, 
                                 const unsigned char *msg, unsigned int msglen);
mqttClientState FogCloudMQTTClientState(void);

//not implement
//OSStatus FogCloudMQTTClientSubscribe(const char* subtopic, enum QoS qos, messageHandler hmsg);
//OSStatus FogCloudMQTTClientUnsubscribe(const char* unsubtopic);

#endif
