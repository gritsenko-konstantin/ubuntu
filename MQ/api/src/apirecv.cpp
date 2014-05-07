/********************************************************************
文件名：apirecv.cpp
创建人：aps-zgh
日  期：2008-08-19
修改人：
日  期：
描  述：接收主控

版  本：
Copyright (c) 2008  SZFESC
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <unistd.h>
#include "api.hpp"

void main()
{
	void *pId;
	char szRtnCode[4];
	memset(szRtnCode, 0, sizeof(szRtnCode));
	
	char szErrDesc[256];
	memset(szErrDesc, 0, sizeof(szErrDesc));

	char szRecvMsg[10240];
	memset(szRecvMsg, 0, sizeof(szRecvMsg));
	
	char szFilePath[512];
	
	char pQLName[128];
	sprintf(pQLName,"PBC2BANK");

	int nFileFlg = 0;
	
	pId = MB_CONNECT("SMPPBC",szRtnCode,szErrDesc);

	if(strcmp(szRtnCode, "000") != 0)
	{
		printf("返回码...[%s]\n",szRtnCode);
		return;
	}

	cout << "连接成功" << endl;
	
	while(1)
	{
		
		memset(szRecvMsg, 0, sizeof(szRecvMsg));
		memset(szFilePath, 0, sizeof(szFilePath));
		sprintf(szFilePath,"/home/smps/api");
		MB_RECEIVE(pId,pQLName,szRecvMsg,szFilePath,nFileFlg,szRtnCode,szErrDesc);

		if(strcmp(szRtnCode, "000") == 0)
		{
			cout << "szRecvMsg: " << szRecvMsg << endl;
		}else if(strcmp(szRtnCode, "201") == 0)
		{
			cout << "No Msg!" << endl;
			sleep(30);
		}
		else
		{
			cout << "Error!:" << szRtnCode << endl;	
			sleep(5);
		}
		
		/*
		
		处理报文
		*/
		char szProcFlag[2];
		memset(szProcFlag, 0, sizeof(szProcFlag));
		szProcFlag[0] = '0';
		MB_CONFIRM(pId, szProcFlag, szRtnCode,szErrDesc);

	}


	MB_DISCONNECT(pId);
}

