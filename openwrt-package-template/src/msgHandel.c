/*================================================================
*   Copyright (C) 2021 Geniatech Ltd. All rights reserved.
*   
*   文件名称：msgHandel.c
*   创 建 者：zhangtao@geniatech.com
*   创建日期：2021年08月10日
*   描    述：
*
================================================================*/
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "msgHandel.h" 
#include "mqttclient.h"
#include "cJSON.h"
#include "log.h"
#include "common.h"
//extern struct MQTT mqtt;
extern mqtt_client_t *client;
//该值用于存储请求信息处理状态，目前只支持同时处理一条指令
//所以其他指令需要等待其他指令处理完以后才能执行
static int REQ_MSG_HANDELING  = 0;
//该值用于存储BLE AT指令处理状态，目前只支持同时处理一条指令
//所以其他指令需要等待其他指令处理完以后才能执行
static int BLE_CTL_CMD_RUNING = 0;

struct SCAN_RESULT{
    char statu[8];
    char flag[8];
    char mac[64];
    char name[64];
};
struct REQ_MSG parseReqMsg(char *msg){
    while(REQ_MSG_HANDELING){
        mqtt_sleep_ms(10);
    }
    struct REQ_MSG reqMsg;
    cJSON *root = cJSON_Parse(msg);
    reqMsg.MsgHead = cJSON_GetObjectItem(root, MSG_HEAD_KEY);
    if(!reqMsg.MsgHead){
        LOG(ERROR, "Get MsgHead fail!!!");
        return reqMsg;
    }
    reqMsg.MsgBody = cJSON_GetObjectItem(root, MSG_BODY_KEY);
    if(!reqMsg.MsgBody){
        LOG(ERROR, "Get MsgBody fail!!!");
        return reqMsg;
    }
    //reqMsg.ServiceCode = cJSON_GetObjectItem(reqMsg.MsgHead, SERVICE_CODE_KEY)->valuestring;
    char *ServiceCode = cJSON_GetObjectItem(reqMsg.MsgHead, SERVICE_CODE_KEY)->valuestring;
    if(!ServiceCode){
        LOG(ERROR, "Get ServiceCode fail!!!");
        return reqMsg;
    }
    strncpy(reqMsg.ServiceCode, ServiceCode, sizeof(reqMsg.ServiceCode));
    //LOG(DEBUG, "ServiceCode:[%s]", ServiceCode);
    LOG(DEBUG, "ServiceCode:[%s]", reqMsg.ServiceCode);
    char *transID = cJSON_GetObjectItem(reqMsg.MsgHead, TRANSACTION_ID_KEY)->valuestring;
    if(!transID){
        LOG(ERROR, "Get transID fail!!!");
        return reqMsg;
    }
    strncpy(reqMsg.transactionID, transID, sizeof(reqMsg.transactionID));
    LOG(DEBUG, "transID:[%s]", reqMsg.transactionID);
    //根据Params和Result字段来判断是设备发来的应答消息还是主动发送的请求消息
    reqMsg.Params = cJSON_GetObjectItem(reqMsg.MsgBody, PARAMS_KEY);
    if(!reqMsg.Params){
        LOG(INFO, "Get Params fail!!!");
        reqMsg.Result = cJSON_GetObjectItem(reqMsg.MsgBody, RESULT_KEY);
        if(!reqMsg.Result){
            LOG(ERROR, "Get Result also fail,wrong data format!!!");
            return reqMsg;
        //如果是设备返回的应答消息那么将消息存入消息列表，供其他线程处理
        }else{
            LOG(INFO, "This is a ack message from device!!!");
        }
    }else{
        //如果是设备主动发送的请求消息那么立即处理
        LOG(INFO, "This is a request message from device!!!");
    }
    return reqMsg;
}
void genTransID(char *transID){
    char timeStamp[16] = {'\0'};
    char randID[16] = {'\0'};
	time_t t;
	t = time(NULL);
	int ii = time(&t);

    int x;
    srand((unsigned int)time(NULL));
    x=rand()%9000000000+1000000000;
    if(x < 0){
        x = x * -1;
    }
    sprintf(transID, "%d-%d", ii, x);
    LOG(DEBUG, "New transID:[%s]", transID);
}
void executeCMD(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
           strcat(result, buf_ps);
           if(strlen(result)>1024)
               break;
        }
        LOG(DEBUG, "[%s] result is [%s]", cmd, result);
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        LOG(ERROR,"popen %s error\n", ps);
    }
}
///root/ams_led_soil_info "gatt write command 2C:AB:33:0A:10:D7 37 001 017 034 051 068 085 102" //开锁
///root/ams_led_soil_info "gatt write command 2C:AB:33:0A:10:D7 37 002 017 034 051 068 085 102" //关锁
///root/ams_led_soil_info "gatt read 2C:AB:33:0A:10:D7 20"读Model number String
//gatt read 2C:AB:33:0A:10:D7 20
//1,08,2C:AB:33:0A:10:D7,20,65476565546F7563682050444C2D787878782D7878
///root/ams_led_soil_info "gatt read 2C:AB:33:0A:10:D7 22"读Serial number String
//gatt read 2C:AB:33:0A:10:D7 22
//1,08,2C:AB:33:0A:10:D7,22,65476565546F7563682053657269616C2F4E
///root/ams_led_soil_info "gatt read 2C:AB:33:0A:10:D7 40"读开关锁执行状态01成功,02失败
//gatt read 2C:AB:33:0A:10:D7 40
//1,08,2C:AB:33:0A:10:D7,40,01


void setDev(char *value, char *handel, char *mac){
    //char suffix[128] = "|grep -oE '[0-9],[0-9]{2},[0-9],([0-9A-Z]{2}:){5}[0-9A-Z]{2}.+'| grep -oE '([0-9A-Z]{2}:){5}[0-9A-Z]{2}'";
    char cmd_full[128] = {'\0'};// ret[128] = {'\0'};
    char ret_surpose[128] = {'\0'};
    char ret[128] = {'\0'};
    //char ack[8] = {'\0'};
    //sprintf(ret_surpose, "1,01,%s", mac);
    sprintf(cmd_full, "%s %s %s %s", AT_CMD_GATT_WRITE_CMD, mac, handel, value);
    getBLEATCMDResult(cmd_full, ret, NULL);
    //getControlMotoACK(ack);
}
//"grep -oE '[0-9],[0-9]{2},([0-9A-Z]{2}:){5}[0-9A-Z]{2},[0-9]+,[0-9]+' |cut -d ',' -f 5"
//gatt read 2C:AB:33:0A:10:D7 40
void getControlMotoACK(char *ack, char *mac){
    char cmd_full[128] = {'\0'};// ret[128] = {'\0'};
    //char ret_surpose[128] = {'\0'};
    //char ret[128] = {'\0'};
    char suffix[128] = "|grep -oE '[0-9],[0-9]{2},([0-9A-Z]{2}:){5}[0-9A-Z]{2},[0-9]+,[0-9]+' |cut -d ',' -f 5";
    //sprintf(suffix, "", mac);
    sprintf(cmd_full, "%s %s %s", AT_CMD_GATT_READ, mac, HANDEL_GEETOUCH_MOTO_STATUS_ACK);
    getBLEATCMDResult(cmd_full, ack, suffix);
    LOG(DEBUG, "ack is [%s]", ack);
}
//1/0 on/off
//0/1 success/fail
int controlMoto(char *mac, int value){
    char ack[8] = {'\0'};
    if(value == 0){
        LOG(INFO, "Start to switch off moto");
        setDev(HANDEL_GEETOUCH_MOTO_OFF, HANDEL_GEETOUCH_MOTO_STATUS, mac);
        mqtt_sleep_ms(100);
        getControlMotoACK(ack, mac);
        //0x01 fail
        LOG(DEBUG, "ack is [%s]", ack);
        if(strncmp(ack, "02", 2) == 0){
            return 0;
        }else{
            return -1;
        }
    }else if(value == 1){
        LOG(INFO, "Start to switch on moto");
        setDev(HANDEL_GEETOUCH_MOTO_ON, HANDEL_GEETOUCH_MOTO_STATUS, mac);
        mqtt_sleep_ms(100);
        getControlMotoACK(ack, mac);
        //0x01 fail
        LOG(DEBUG, "ack is [%s]", ack);
        if(strncmp(ack, "01", 2) == 0){
            return 0;
        }else{
            return -1;
        }
    }
}
/*
2C:AB:33:0A:10:D7
FB:74:2B:D7:CA:0A
*/
void getConnectedDevices(char *ret){
    char suffix[128] = "|grep -oE '[0-9],[0-9]{2},[0-9],([0-9A-Z]{2}:){5}[0-9A-Z]{2}.+'| grep -oE '([0-9A-Z]{2}:){5}[0-9A-Z]{2}'";
    char cmd[128] = {'\0'};// ret[128] = {'\0'};
    char ret_surpose[128] = {'\0'};
    //sprintf(ret_surpose, "1,01,%s", mac);
    //sprintf(cmd, "%s", AT_CMD_CONNECTED_DEVICES);
    getBLEATCMDResult(AT_CMD_CONNECTED_DEVICES, ret, suffix);
}
int connectDev(char *mac){
    char suffix[128] = "|grep -oE '[0-9],[0-9]{2},([0-9A-Z]{2}:){5}[0-9A-Z]{2}.+'";
    char cmd[128] = {'\0'}, ret[128] = {'\0'};
    char ret_surpose[128] = {'\0'};
    char connectedDeviceMacList[1024] = {'\0'};
    sprintf(ret_surpose, "1,01,%s", mac);
    sprintf(cmd, "%s %s", AT_CMD_CONNECT,  mac);
    getBLEATCMDResult(cmd, ret, suffix);
    mqtt_sleep_ms(500);
    getConnectedDevices(connectedDeviceMacList);
    int i = 0, j = 0;
    char line[64] = {'\0'};
    while(connectedDeviceMacList[i]){
        if(connectedDeviceMacList[i] != '\n' && connectedDeviceMacList[i] != '\r'){
            line[j] = connectedDeviceMacList[i];
            //LOG(DEBUG, "line[%d] [%c]", j, line[j]);
            //LOG(DEBUG, "line[%d] [%#X]", j, line[j]);
            ++i;++j;
        }else{
            line[j] = '\0';
            j = 0;
            ++i;
            LOG(DEBUG, "line:[%s]", line);
            if(strncmp(mac, line, strlen(mac)) == 0){
                LOG(INFO, "Connect device:%s success!", mac);
                return 0;
            }
        }
    }
    LOG(WARN, "Connect device:%s fail!", mac);
    return -1;
}
int disConnectDev(char *mac){
    //char suffix[128] = "|grep -oE '[0-9],[0-9]{2},([0-9A-Z]{2}:){5}[0-9A-Z]{2}.+'";
    char cmd[128] = {'\0'}, ret[128] = {'\0'};
    //char ret_surpose[128] = {'\0'};
    //char connectedDeviceMacList[1024] = {'\0'};
    //sprintf(ret_surpose, "1,01,%s", mac);
    sprintf(cmd, "%s %s", AT_CMD_DISCONNECT,  mac);
    getBLEATCMDResult(cmd, ret, NULL);
    //mqtt_sleep_ms(500);
    //getConnectedDevices(connectedDeviceMacList);
    /*
       int i = 0, j = 0;
       char line[64] = {'\0'};
       while(connectedDeviceMacList[i]){
       if(connectedDeviceMacList[i] != '\n' && connectedDeviceMacList[i] != '\r'){
       line[j] = connectedDeviceMacList[i];
    //LOG(DEBUG, "line[%d] [%c]", j, line[j]);
    //LOG(DEBUG, "line[%d] [%#X]", j, line[j]);
    ++i;++j;
    }else{
    line[j] = '\0';
    j = 0;
    ++i;
    LOG(DEBUG, "line:[%s]", line);
    if(strncmp(mac, line, strlen(mac)) == 0){
    LOG(INFO, "Connect device:%s success!", mac);
    return 0;
    }
    }
    }
    LOG(WARN, "Connect device:%s fail!", mac);
    */
    return 0;
}
void getBLEATCMDResult(char *cmd, char *ret, char *suffix){
    while(BLE_CTL_CMD_RUNING){
        mqtt_sleep_ms(10);
    }
    char cmd_full[256] = {'\0'};
    if(suffix){
        sprintf(cmd_full, "%s \"%s\" %s", BLE_CTL_BIN_PATH, cmd, suffix);
    }else{
        sprintf(cmd_full, "%s \"%s\"", BLE_CTL_BIN_PATH, cmd);
    }
    LOG(DEBUG, "exec cmd [%s]", cmd_full);
    BLE_CTL_CMD_RUNING = 1;
    executeCMD(cmd_full, ret);
    BLE_CTL_CMD_RUNING = 0;
}
void getScanResult(char *ret){
    char suffix[128] = "|grep -oE '[0-9],[0-9]{2},([0-9A-Z]{2}:){5}[0-9A-Z]{2}.+'";
    getBLEATCMDResult(AT_CMD_SCAN, ret, suffix);
    system(SHELL_GET_BLE_INFO);
}
struct DEV_INFO {
    char BroadcastName[64];
    char DeviceMAC[32];
    char NickName[64];
    char ModelNo[64];
    char SN[64];
    char MotorSta[8];
};
///root/ams_led_soil_info "gatt read 2C:AB:33:0A:10:D7 20"读Model number String
//gatt read 2C:AB:33:0A:10:D7 20
//1,08,2C:AB:33:0A:10:D7,20,65476565546F7563682050444C2D787878782D7878
///root/ams_led_soil_info "gatt read 2C:AB:33:0A:10:D7 22"读Serial number String
//gatt read 2C:AB:33:0A:10:D7 22
//1,08,2C:AB:33:0A:10:D7,22,65476565546F7563682053657269616C2F4E
void getGeeTouchSN_ModelNo(char *mac, char *sn, char *modelNo){
    char cmd[128] = {'\0'};
    char ret[128] = {'\0'};
    sprintf(cmd, "%s %s %s", AT_CMD_GATT_READ, mac, HANDEL_GEETOUCH_SERIAL_NUMBER);
    getBLEATCMDResult(cmd, ret, "|cut -d ',' -f 5");
    if(strlen(ret) > 2 && (strlen(ret) % 2) == 0){
        hexStrtoAsciiStr(ret, sn);
    }else{
        strncpy(sn, "unknow", 6);
    }

    memset(ret, '\0', sizeof(ret));
    memset(cmd, '\0', sizeof(cmd));

    sprintf(cmd, "%s %s %s",AT_CMD_GATT_READ, mac, HANDEL_GEETOUCH_MODEL_NUMBER);
    getBLEATCMDResult(cmd, ret, "|cut -d ',' -f 5");
    if(strlen(ret) > 2 && (strlen(ret) % 2) == 0){
        hexStrtoAsciiStr(ret, modelNo);
    }else{
        strncpy(modelNo, "unknow", 6);
    }
    LOG(INFO, "SN:[%s]", sn);
    LOG(INFO, "ModelNo:[%s]", modelNo);
}
void getDevNameByMAC(char *mac, char *name){
    char cmd[128] = {'\0'};
    sprintf(cmd, "cat /etc/scan.result |grep %s|cut -d ',' -f 4", mac);
    //getBLEATCMDResult(cmd, name, NULL);
    executeCMD(cmd, name);
}
cJSON *genGetConnectedListResp(char *connectedMACList){
    cJSON *root, *ConnectedDevList, *list;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "ConnectedDevList", ConnectedDevList = cJSON_CreateArray());
    //LOG(DEBUG,"scandev_info is [%s]", scandev_info);
    char mac[64] = {'\0'};
    int i = 0, j = 0, k = 0;
    while(connectedMACList[i]){
        if(connectedMACList[i] != '\n' && connectedMACList[i] != '\r'){
            mac[j] = connectedMACList[i];
            //LOG(DEBUG, "mac[%d] [%c]", j, mac[j]);
            //LOG(DEBUG, "mac[%d] [%#X]", j, mac[j]);
            ++i;++j;
        }else{
            mac[j] = '\0';
            LOG(DEBUG, "mac:[%s]", mac);
            if(strlen(mac) > 16){
                char sn[128] = {'\0'};
                char modelNo[128] = {'\0'};
                char name[128] = {'\0'};
                struct DEV_INFO dev_info;
                //1,01,FB:74:2B:D7:CA:0A,GTC_LED
                //LOG(DEBUG, "Get match mac");
                //memset(dev_info, 0, sizeof(dev_info));
                strncpy(dev_info.DeviceMAC, mac, strlen(mac));

                //LOG(DEBUG, "mac lenth is [%d]", strlen(mac));
                //LOG(DEBUG, "name lenth is [%d]", name_len);
                getDevNameByMAC(mac, name);
                //strncpy(dev_info.BroadcastName, name, sizeof(dev_info.BroadcastName));
                if(strlen(name)> 3){
                    strncpy(dev_info.BroadcastName, name, strlen(name));
                }else{
                    if(strncmp(mac, "2C:AB:33", 8) == 0){
                        strncpy(dev_info.BroadcastName, "eGeeTouch A13006", 16);
                    }else{
                        strncpy(dev_info.BroadcastName, "unknow", 6);
                    }
                }

                getGeeTouchSN_ModelNo(mac, sn, modelNo);
                strncpy(dev_info.SN, sn, sizeof(dev_info.SN));
                strncpy(dev_info.ModelNo, modelNo, sizeof(dev_info.ModelNo));
                //LOG(DEBUG, "statu:[%s]", dev_info.statu);
                //LOG(DEBUG, "flag:[%s]", dev_info.flag);
                //LOG(DEBUG, "mac:[%s]", dev_info.mac);
                //LOG(DEBUG, "name:[%s]", dev_info.name);
                LOG(DEBUG, "DeviceMAC:[%s]", dev_info.DeviceMAC);
                LOG(DEBUG, "BroadcastName:[%s]", dev_info.BroadcastName);
                LOG(DEBUG, "SN:[%s]", dev_info.SN);
                LOG(DEBUG, "ModelNo:[%s]", dev_info.ModelNo);

                cJSON_AddItemToArray(ConnectedDevList, list = cJSON_CreateObject());
                cJSON_AddStringToObject(list, "DeviceMAC", dev_info.DeviceMAC);
                cJSON_AddStringToObject(list, "BroadcastName", dev_info.BroadcastName);
                cJSON_AddStringToObject(list, "ModelNo", dev_info.ModelNo);
                cJSON_AddStringToObject(list, "SN", dev_info.SN);
            }
            memset(mac, 0, sizeof(mac));
            j = 0;
            ++i;
        }
    }
    char *s = cJSON_PrintUnformatted(ConnectedDevList);
    if(s){
        LOG(DEBUG,"root is  %s \n",s);
        free(s);
    }
    return root;
}
/*
//1,01,FB:74:2B:D7:CA:0A,GTC_LED
LOG(DEBUG, "Get match line");
memset(result.statu, '\0', sizeof(result.statu));
memset(result.flag, '\0', sizeof(result.flag));
memset(result.mac, '\0', sizeof(result.mac));
memset(result.name, '\0', sizeof(result.name));
strncpy(result.statu, line + 0, 1);
strncpy(result.flag, line + 2, 2);
strncpy(result.mac, line + 5, 17);

int name_len = strlen(line) - 23;
LOG(DEBUG, "line lenth is [%d]", strlen(line));
LOG(DEBUG, "name lenth is [%d]", name_len);
if(name_len > 1){
strncpy(result.name, line + 23, name_len);
}else{
strncpy(result.name, "unknow", 6);
}

LOG(DEBUG, "statu:[%s]", result.statu);
LOG(DEBUG, "flag:[%s]", result.flag);
LOG(DEBUG, "mac:[%s]", result.mac);
LOG(DEBUG, "name:[%s]", result.name);

cJSON_AddItemToArray(ScanResult, list = cJSON_CreateObject());
cJSON_AddStringToObject(list, "DeviceMAC", result.mac);
cJSON_AddStringToObject(list, "BroadcastName", result.name);
*/
cJSON *genScanResultResp(char *scanResult){
    struct SCAN_RESULT result;
    cJSON *root, *ScanResult, *list;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "ScanResult", ScanResult = cJSON_CreateArray());
    LOG(DEBUG,"scanResult is [%s]", scanResult);
    char line[128] = {'\0'};
    int i = 0, j = 0, k = 0;
    while(scanResult[i]){
        if(scanResult[i] != '\n' && scanResult[i] != '\r'){
            line[j] = scanResult[i];
            //LOG(DEBUG, "line[%d] [%c]", j, line[j]);
            //LOG(DEBUG, "line[%d] [%#X]", j, line[j]);
            ++i;++j;
        }else{
            line[j] = '\0';
            //LOG(DEBUG, "line:[%s]", line);
            if(strlen(line) > 22){
                if(line[7] == ':' && line[10] == ':' && 
                   line[13] == ':' && line[16] == ':' && 
                   line[19] == ':'){
                    //1,01,FB:74:2B:D7:CA:0A,GTC_LED
                    //LOG(DEBUG, "Get match line");
                    memset(result.statu, '\0', sizeof(result.statu));
                    memset(result.flag, '\0', sizeof(result.flag));
                    memset(result.mac, '\0', sizeof(result.mac));
                    memset(result.name, '\0', sizeof(result.name));
                    strncpy(result.statu, line + 0, 1);
                    strncpy(result.flag, line + 2, 2);
                    strncpy(result.mac, line + 5, 17);

                    int name_len = strlen(line) - 23;
                    //LOG(DEBUG, "line lenth is [%d]", strlen(line));
                    //LOG(DEBUG, "name lenth is [%d]", name_len);
                    if(name_len > 3){
                        strncpy(result.name, line + 23, name_len);
                    }else{
                        if(strncmp(result.mac, "2C:AB:33", 8) == 0){
                            strncpy(result.name, "eGeeTouch A13006", 16);
                        }else{
                            strncpy(result.name, "unknow", 6);
                        }
                    }

                    //LOG(DEBUG, "statu:[%s]", result.statu);
                    //LOG(DEBUG, "flag:[%s]", result.flag);
                    //LOG(DEBUG, "mac:[%s]", result.mac);
                    //LOG(DEBUG, "name:[%s]", result.name);

                    cJSON_AddItemToArray(ScanResult, list = cJSON_CreateObject());
                    cJSON_AddStringToObject(list, "DeviceMAC", result.mac);
                    cJSON_AddStringToObject(list, "BroadcastName", result.name);
                }else{
                    LOG(WARN, "[%s] not a right format", line);
                }
            }
            memset(line, 0, sizeof(line));
            j = 0;
            ++i;
        }
    }
    char *s = cJSON_PrintUnformatted(ScanResult);
    if(s){
        LOG(DEBUG,"root is  %s \n",s);
        free(s);
    }
    return root;
}
cJSON *genRespMsg(char *transID, char *serviceCode, int resultCode, cJSON *ResultData){

    cJSON *root, *MsgHead, *MsgBody, *ResultCode, *Result;
    char *s = cJSON_PrintUnformatted(ResultData);
    if(s){
        LOG(DEBUG,"ResultData is [%s] \n",s);
        free(s);
    }
    root = cJSON_CreateObject();

    MsgHead = cJSON_CreateObject();
    cJSON_AddStringToObject(MsgHead, SERVICE_CODE_KEY, serviceCode);
    cJSON_AddStringToObject(MsgHead, TRANSACTION_ID_KEY, transID);

    MsgBody = cJSON_CreateObject();
    Result = cJSON_CreateObject();
    //ResultData = cJSON_CreateObject();
    cJSON_AddItemToObject(MsgBody, RESULT_KEY, Result);
    cJSON_AddItemToObject(Result, RESULT_DATA_KEY, ResultData);
    cJSON_AddNumberToObject(Result, RESULT_CODE_KEY, resultCode);

    cJSON_AddItemToObject(root, MSG_BODY_KEY, MsgBody);
    cJSON_AddItemToObject(root, MSG_HEAD_KEY, MsgHead);
    return root;
}
void APIgetConnectedList(struct REQ_MSG *reqMsg){
    LOG(INFO, "Enter APIgetConnectedList");
    //char transID[64] = {'\0'};
    //genTransID(transID);
    char scanResult[1024 * 4] = {'\0'};
    char connectedDevicesList[1024] = {'\0'};
    getScanResult(scanResult);
    getConnectedDevices(connectedDevicesList);
    cJSON *ResultData;
    cJSON *respMsg;
    //char respMsgStr[1024 * 4] = {'\0'};
    //char *respMsgStr;
    ResultData = genGetConnectedListResp(connectedDevicesList);
    respMsg = genRespMsg(reqMsg->transactionID, reqMsg->ServiceCode, 0, ResultData);
    char *respMsgStr = cJSON_PrintUnformatted(respMsg);
    //char *respMsgStr = cJSON_Print(respMsg);
    if(respMsgStr){
        //LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, mqtt.tx_topic);
        LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, "GTW350B/TX");
        //mqttPublishMsg(client, mqtt.tx_topic, respMsgStr);
        mqttPublishMsg(client, "GTW350B/TX", respMsgStr);
        free(respMsgStr);
    }
}
void APIgetScanResult(struct REQ_MSG *reqMsg){
    LOG(INFO, "Enter get scan result api");
    //char transID[64] = {'\0'};
    //genTransID(transID);
    char scanResult[1024 * 4] = {'\0'};
    getScanResult(scanResult);
    cJSON *ResultData;
    cJSON *respMsg;
    //char respMsgStr[1024 * 4] = {'\0'};
    //char *respMsgStr;
    ResultData = genScanResultResp(scanResult);
    respMsg = genRespMsg(reqMsg->transactionID, reqMsg->ServiceCode, 0, ResultData);
    char *respMsgStr = cJSON_PrintUnformatted(respMsg);
    //char *respMsgStr = cJSON_Print(respMsg);
    if(respMsgStr){
        //LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, mqtt.tx_topic);
        LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, "GTW350B/TX");
        //mqttPublishMsg(client, mqtt.tx_topic, respMsgStr);
        mqttPublishMsg(client, "GTW350B/TX", respMsgStr);
        free(respMsgStr);
    }
}
void APIsetDev(struct REQ_MSG *reqMsg){
    LOG(DEBUG, "Enter APIsetDev");
    cJSON *ResultData;
    cJSON *respMsg;
    char *mac, *action;
    int ack = -1;
    mac = cJSON_GetObjectItem(reqMsg->Params, "mac")->valuestring;
    action = cJSON_GetObjectItem(reqMsg->Params, "action")->valuestring;
    LOG(DEBUG,"mac:[%s]", mac);
    LOG(DEBUG,"action:[%s]", action);
    if(strncmp(action, "on", 2) == 0){
        LOG(DEBUG, "get on command");
        ack = controlMoto(mac, 1);
    }else if(strncmp(action, "off", 3) == 0){
        LOG(DEBUG, "get off command");
        ack = controlMoto(mac, 0);
    }
    //char respMsgStr[1024 * 4] = {'\0'};
    //char *respMsgStr;
    //ResultData = genScanResultResp(scanResult);
    respMsg = genRespMsg(reqMsg->transactionID, reqMsg->ServiceCode, ack, ResultData);
    char *respMsgStr = cJSON_PrintUnformatted(respMsg);
    //char *respMsgStr = cJSON_Print(respMsg);
    if(respMsgStr){
        //LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, mqtt.tx_topic);
        LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, "GTW350B/TX");
        //mqttPublishMsg(client, mqtt.tx_topic, respMsgStr);
        mqttPublishMsg(client, "GTW350B/TX", respMsgStr);
        free(respMsgStr);
    }
}
void APIconnectDev(struct REQ_MSG *reqMsg){
    LOG(DEBUG, "Enter APIconnectDev");
    int ret = 0;
    char *MAC;
    MAC = cJSON_GetObjectItem(reqMsg->Params, "MAC")->valuestring;
    LOG(DEBUG,"MAC:[%s]", MAC);
    ret = connectDev(MAC);

    //char transID[64] = {'\0'};
    //genTransID(transID);
    //char scanResult[1024 * 4] = {'\0'};
    cJSON *ResultData;
    cJSON *respMsg;
    //char respMsgStr[1024 * 4] = {'\0'};
    //char *respMsgStr;
    //ResultData = genScanResultResp(scanResult);
    respMsg = genRespMsg(reqMsg->transactionID, reqMsg->ServiceCode, ret, ResultData);
    char *respMsgStr = cJSON_PrintUnformatted(respMsg);
    //char *respMsgStr = cJSON_Print(respMsg);
    if(respMsgStr){
        //LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, mqtt.tx_topic);
        LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, "GTW350B/TX");
        //mqttPublishMsg(client, mqtt.tx_topic, respMsgStr);
        mqttPublishMsg(client, "GTW350B/TX", respMsgStr);
        free(respMsgStr);
    }
}
void APIdisConnectDev(struct REQ_MSG *reqMsg){
    LOG(DEBUG, "Enter APIdisConnectDev");
    int ret = 0;
    char *MAC;
    MAC = cJSON_GetObjectItem(reqMsg->Params, "MAC")->valuestring;
    LOG(DEBUG,"MAC:[%s]", MAC);
    ret = disConnectDev(MAC);

    //char transID[64] = {'\0'};
    //genTransID(transID);
    //char scanResult[1024 * 4] = {'\0'};
    cJSON *ResultData;
    cJSON *respMsg;
    //char respMsgStr[1024 * 4] = {'\0'};
    //char *respMsgStr;
    //ResultData = genScanResultResp(scanResult);
    respMsg = genRespMsg(reqMsg->transactionID, reqMsg->ServiceCode, ret, ResultData);
    char *respMsgStr = cJSON_PrintUnformatted(respMsg);
    //char *respMsgStr = cJSON_Print(respMsg);
    if(respMsgStr){
        //LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, mqtt.tx_topic);
        LOG(DEBUG,"Send respMsg  [%s] to topic [%s]", respMsgStr, "GTW350B/TX");
        //mqttPublishMsg(client, mqtt.tx_topic, respMsgStr);
        mqttPublishMsg(client, "GTW350B/TX", respMsgStr);
        free(respMsgStr);
    }
}
#if 0
void reqMsgHandel(void *client, char *ServiceCode, char *transID, char *msg){
    if(!strcmp(ServiceCode, SER_CODE_SETDEV)){
        LOG(INFO, "Enter set device api");
    }else if(!strcmp(ServiceCode, SER_CODE_GETSCANRESULT)){
        APIgetScanResult(client, ServiceCode, transID);
    }
}
#else
void reqMsgHandel(void *client, struct REQ_MSG *reqMsg){
    if(strcmp(reqMsg->ServiceCode, SER_CODE_SETDEV) == 0){
        LOG(INFO, "Enter set device api");
        APIsetDev(reqMsg);
    }else if(strcmp(reqMsg->ServiceCode, SER_CODE_CONNECTDEV) == 0){
        APIconnectDev(reqMsg);
    }else if(strcmp(reqMsg->ServiceCode, SER_CODE_GETSCANRESULT) == 0){
        APIgetScanResult(reqMsg);
    }else if(strcmp(reqMsg->ServiceCode, "getConnectedList") == 0){
        APIgetConnectedList(reqMsg);
    }else if(strcmp(reqMsg->ServiceCode, SER_CODE_DISCONNECTDEV) == 0){
        APIdisConnectDev(reqMsg);
    }
}
#endif
int mqttPublishMsg(mqtt_client_t *client, char *topic, char *msg_json)
{
    //char *msg_json;
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.qos = 0;
    msg.payload = (void *) msg_json;

    return mqtt_publish(client, topic, &msg);
}

int  recvMsgHandel(char *msg, void *client){ 
    struct REQ_MSG reqMsg;
    reqMsg = parseReqMsg(msg);
    if(reqMsg.Params){
        //reqMsgHandel(client, reqMsg.ServiceCode, reqMsg.transactionID, msg);
        reqMsgHandel(client, &reqMsg);
    }else if(reqMsg.Result){
        LOG(INFO, "do it later");
    }
} 
