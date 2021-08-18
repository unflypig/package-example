/*================================================================
*   Copyright (C) 2021 Geniatech Ltd. All rights reserved.
*   
*   文件名称：msgHandel.h
*   创 建 者：zhangtao@geniatech.com
*   创建日期：2021年08月11日
*   描    述：
*
================================================================*/
#ifndef __MSGHANDEL_H__
#include <stdint.h>
#include "cJSON.h"
#define __MSGHANDEL_H__
#define MSG_HEAD_KEY "MsgHead"
#define MSG_BODY_KEY "MsgBody"
#define PARAMS_KEY "Params"
#define RESULT_KEY "Result"
#define RESULT_CODE_KEY "ResultCode"
#define RESULT_DATA_KEY "ResultData"
#define SERVICE_CODE_KEY "ServiceCode"
#define TRANSACTION_ID_KEY "transactionID"
#define SCAN_RESULT_KEY "ScanResult"


#define SER_CODE_SETDEV "setDev"
#define SER_CODE_GETSCANRESULT "getScanResult"
#define SER_CODE_CONNECTDEV "connectDev"
#define SER_CODE_DISCONNECTDEV "disConnectDev"

//AT cmd list
#define AT_CMD_GATT_READ "gatt read"
#define AT_CMD_GATT_WRITE_CMD "gatt write command"

#define AT_CMD_SCAN "scan"
#define AT_CMD_CONNECT "connect"
#define AT_CMD_DISCONNECT "disconnect"
#define AT_CMD_CONNECTED_DEVICES "connected_devices"
#define AT_CMD_READ_GEETOUCH_SERIAL_NUMBER ""

#define BLE_CTL_BIN_PATH "/root/ams_led_soil_info"
#define SHELL_GET_BLE_INFO "gtc_get_ble_dev_info.sh"

//Handel list
#define HANDEL_GEETOUCH_SERIAL_NUMBER   "22"
#define HANDEL_GEETOUCH_MODEL_NUMBER    "20"
#define HANDEL_GEETOUCH_MOTO_STATUS     "37" 
#define HANDEL_GEETOUCH_MOTO_STATUS_ACK "40"
#define HANDEL_GEETOUCH_MOTO_ON "001 017 034 051 068 085 102"
#define HANDEL_GEETOUCH_MOTO_OFF "002 017 034 051 068 085 102"

//请求报文结构
/*
{
    "MsgBody": {
        "Params": {
            "MAC": "AA:BB:CC:DD:EE:FF"
        }
    },
    "MsgHead": {
        "ServiceCode": "getScanResult",
        "transactionID": "1436232616993-7485637297"
    }
}
*/
struct REQ_MSG {
    cJSON *MsgBody;
    cJSON *MsgHead;
    cJSON *Params;
    cJSON *Result;
    char ServiceCode[128];
    char transactionID[64];
};
/*
struct MSG {
    struct MSG_HEAD;
    struct MSG_BODY;
};
struct MSG_HEAD{
    char ServiceCode[256];
    char transactionID[64];
}
*/

//应答报文结构
/*
{
    "MsgBody": {
        "Result": {
            "ResultCode": 0,
            "ResultData": {
                "ScanResult": [
                    {
                        "BroadcastName": "Bluemity",
                        "DeviceMAC": "AA:BB:CC:DD:EE:FF",
                        "NickName": "master bedroom lamp"
                    },
                    {
                        "BroadcastName": "Bluemity",
                        "DeviceMAC": "AA:BB:CC:DD:EE:EE",
                        "NickName": "guest bedroom lamp"
                    }
                ]
            }
        }
    },
    "MsgHead": {
        "ServiceCode": "getScanResult",
        "transactionID": "1436232616993-7485637297"
    }
}
*/
struct RESP_MSG{
    cJSON *MsgBody;
    cJSON *MsgHead;
    cJSON *Result;
    cJSON *ResultData;
    cJSON *Params;
    char ServiceCode[128];
    char transactionID[64];
    int ResultCode;
};
int  recvMsgHandel(char *msg, void *client);
#endif
