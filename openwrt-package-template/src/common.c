/*================================================================
*   Copyright (C) 2021 Geniatech Ltd. All rights reserved.
*   
*   文件名称：common.c
*   创 建 者：zhangtao@geniatech.com
*   创建日期：2021年08月15日
*   描    述：
*
================================================================*/
#include "common.h"

unsigned char HexToChar(unsigned char bChar)
{
	if((bChar>=0x30)&&(bChar<=0x39))
	{
		bChar -= 0x30;
	}
	else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
	{
		bChar -= 0x37;
	}
	else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
	{
		bChar -= 0x57;
	}
	else
	{
		bChar = 0xff;
	}
	return bChar;
}
void StrToHex(U8_T* pbDest, U8_T* pbSrc, U8_T nLen)
{
	U8_T h1,h2;
	U8_T s1,s2;
	U8_T i;

	for (i = 0; i < nLen; i++)
	{
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
}
void hexStrtoAsciiStr(U8_T *hexStr, U8_T *out){
    int i;
    //char test[8] = "47";
    //U8_T hexStr[12] = "474f4f4447";
	//U8_T out[6] = {0};
    memset(out, 0 ,sizeof(out));
    StrToHex(out, hexStr, strlen(hexStr) / 2);
    //unsigned char bChar = 0x47;
    for(i = 0; i < (strlen(hexStr) / 2) -1; i++)
	{
		//LOG(DEBUG, "out[%d] : %x \n", i, out[i]);
        HexToChar(out[i]);
        //LOG(DEBUG, "%c\r\n", out[i]);
	}
    out[i] = '\0';
    //LOG(DEBUG, "[%s]", out);
}
/*
void main(void){
    int i;
    U8_T tmp[12] = "474f4f4447";
    U8_T out[12] = {'\0'};
    hexStrtoAsciiStr(tmp, out);
}
*/
