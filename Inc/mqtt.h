#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_DUP_FLAG     1<<3 
#define MQTT_QOS0_FLAG    0<<1 
#define MQTT_QOS1_FLAG    1<<1 
#define MQTT_QOS2_FLAG    2<<1 


#define MQTT_RETAIN_FLAG  1 


#define MQTT_CLEAN_SESSION  1<<1 
#define MQTT_WILL_FLAG      1<<2 
#define MQTT_WILL_RETAIN    1<<5 
#define MQTT_USERNAME_FLAG  1<<7 
#define MQTT_PASSWORD_FLAG  1<<6 
#define MQTT_MSG_CONNECT       1<<4 
#define MQTT_MSG_CONNACK       2<<4 
#define MQTT_MSG_PUBLISH       3<<4 
#define MQTT_MSG_PUBACK        4<<4 
#define MQTT_MSG_PUBREC        5<<4 
#define MQTT_MSG_PUBREL        6<<4 
#define MQTT_MSG_PUBCOMP       7<<4 
#define MQTT_MSG_SUBSCRIBE     8<<4 
#define MQTT_MSG_SUBACK        9<<4 
#define MQTT_MSG_UNSUBSCRIBE  10<<4 
#define MQTT_MSG_UNSUBACK     11<<4 
#define MQTT_MSG_PINGREQ      12<<4 
#define MQTT_MSG_PINGRESP     13<<4 
#define MQTT_MSG_DISCONNECT   14<<4 


RET_VALUE   MQTT_MSG_createConnect(char* buffer, uint32_t bufferLength, uint32_t* msgLength, char* clientId, char* userId, char *passwd);
RET_VALUE   MQTT_MSG_createPublish(char* buffer, uint32_t bufferLength, uint32_t* frameLength, char* topic, char *message);

#endif