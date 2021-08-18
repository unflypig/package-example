/* * @Author: jiejie * @Github: https://github.com/jiejieTop * @LastEditTime: 2021-06-22 08:54:12 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.  */ #include <stdio.h>
#include <stdint.h>

#include "mqttclient.h"
#include "log.h"
#include "cJSON.h"
#include "uci.h"
#include "msgHandel.h" 
#define MQTT_SERVER_USR_NAME "admin"
#define MQTT_SERVER_PSW "geniatech1234"
struct MQTT mqtt;
mqtt_client_t *client = NULL;
struct MQTT
{
    char server_url[256]; //mqtt服务器地址
    char server_port[256];  //mqtt服务器端口
    char rx_topic[256];
    char tx_topic[256];
};
/*根据传入的配置文件option的key，查找对应的value*/
//@ctx:该值是为接口分配空间的指针，类似于malloc
//@key:要查询option的key
//@value:查询到的key所对应的value值，存放指针
//@n:拷贝n个字节到value
int getValue(struct uci_context *ctx, char *key, char*value, int n)
{
    char strKey[100];
    struct uci_ptr ptr;

    snprintf(strKey, sizeof(strKey), "hello.globe.%s",key);
    if (uci_lookup_ptr(ctx, &ptr, strKey, true) == UCI_OK)
    {
        //printf("%s\n", ptr.o->v.string);
        strncpy(value, ptr.o->v.string, n-1);
    }
    return 0;
}

int read_conf( struct MQTT *mqtt)
{
    struct uci_context *ctx = uci_alloc_context();
    if (!ctx) 
    {
        fprintf(stderr, "No memory\n");
        return 1;
    }
    getValue(ctx, "server_url", mqtt->server_url, sizeof(mqtt->server_url));
    getValue(ctx, "server_port", mqtt->server_port, sizeof(mqtt->server_port));
    getValue(ctx, "rx_topic", mqtt->rx_topic, sizeof(mqtt->rx_topic));
    getValue(ctx, "tx_topic", mqtt->tx_topic, sizeof(mqtt->tx_topic));

    uci_free_context(ctx);    
    return 0;
}



//static char mqtt_msg[256] = {'\0'};
void msg_gen(char **msg_json){
    int age = 18;
    char *name = "student";
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", name);
    cJSON_AddNumberToObject(root, "age", age);
    *msg_json = cJSON_Print(root);
    LOG(DEBUG, "msg_json is [%s]", *msg_json);
    strncpy(*msg_json, cJSON_Print(root), sizeof(*msg_json));
    //LOG(DEBUG, "msg_json is [%s]", *msg_json);
}
static int mqtt_publish_handle1(mqtt_client_t *client, char *topic, char *msg_json)
{
    //char *msg_json;
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    //msg_gen(&mqtt_msg);
    /*
    int age = 18;
    char *name = "student";
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", name);
    cJSON_AddNumberToObject(root, "age", age);
    msg_json = cJSON_Print(root);
    LOG(DEBUG, "msg_json is [%s]", msg_json);
    */

    msg.qos = 2;
    msg.payload = (void *) msg_json;

    return mqtt_publish(client, topic, &msg);
}
void recv_handlel_bk(void* client, message_data_t* msg){
    (void) client;
    //LOG(INFO,"----------------------------------------------------------------------------------");
    LOG(INFO,"get message");
    LOG(DEBUG, "msg->message->payload is [%s]", (char *)msg->message->payload);
    cJSON *root = cJSON_Parse(msg->message->payload);
    if(!root){
        LOG(ERROR, "[%s] not a json format", (char *)msg->message->payload);
        return;
    }
    recvMsgHandel(msg->message->payload, client);
#if 0
    /*
     { "message":"hellow client", "code":10086}
    */
    if(!root){
        LOG(ERROR, "not a json format:[%s]", (char *)msg->message->payload);
        return;
    }
    char *message  = cJSON_GetObjectItem(root, "message")->valuestring;
    int code = -1;
    code = cJSON_GetObjectItem(root, "code")->valueint;
    if(message){
        LOG(DEBUG,"message is [%s]", message);  
    }else{
        LOG(ERROR, "get message fail");
    }
    if(code != -1){
        LOG(DEBUG,"code is [%d]", code);  
    }else{
        LOG(ERROR, "get code fail");
    }
#endif
    LOG(INFO,"----------------------------------------------------------------------------------");
    cJSON_Delete(root);
}


int main(void)
{
    log_init();
    LOG(INFO, "Program start");
    read_conf(&mqtt);
    LOG(INFO, "server_url:[%s]", mqtt.server_url);
    LOG(INFO, "server_port:[%s]", mqtt.server_port);
    LOG(INFO, "rx_topic:[%s]", mqtt.rx_topic);
    LOG(INFO, "tx_topic:[%s]", mqtt.tx_topic);

    //mqtt_log_init();

    client = mqtt_lease();

    mqtt_set_host(client, mqtt.server_url);
    mqtt_set_port(client, mqtt.server_port);
    mqtt_set_user_name(client, MQTT_SERVER_USR_NAME);
    mqtt_set_password(client, MQTT_SERVER_PSW);
    mqtt_set_clean_session(client, 1);
    mqtt_set_read_buf_size(client, 4096);
    mqtt_set_write_buf_size(client, 4096);

    mqtt_connect(client);
    mqtt_subscribe(client, mqtt.rx_topic, QOS2, recv_handlel_bk);

    while (1) {
        //mqtt_publish_handle1(client, mqtt.tx_topic);
        mqtt_sleep_ms(4 * 1000);
    }
}
