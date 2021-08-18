/*================================================================
*   Copyright (C) 2021 Geniatech Ltd. All rights reserved.
*   
*   文件名称：common.h
*   创 建 者：zhangtao@geniatech.com
*   创建日期：2021年08月15日
*   描    述：
*
================================================================*/

#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "log.h"
typedef   unsigned char     BOOL_T;
typedef   unsigned char     U8_T;
typedef   signed   char     S8_T;
typedef   unsigned short    U16_T;
typedef   signed   short    S16_T;
typedef   unsigned int      U32_T;
typedef   signed   int      S32_T;
//typedef   unsigned __int64  U64_T;
//typedef   signed   __int64  S64_T;
typedef   float             F32_T;
typedef   double            F64_T;
typedef   char *            STR_T;

unsigned char HexToChar(unsigned char bChar);
void StrToHex(U8_T* pbDest, U8_T* pbSrc, U8_T nLen);
void hexStrtoAsciiStr(U8_T *hexStr, U8_T *out);
#endif
