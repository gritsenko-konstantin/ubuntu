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
#include <iostream>
using namespace std;
#include <unistd.h>
#include "api.hpp"

int main()
{
	void *pId;
	char szRtnCode[268];
	char pRetStr[1024];
	memset(szRtnCode, 0, sizeof(szRtnCode));
	memset(pRetStr, 0, sizeof(pRetStr));

	pId = MB_CONNECT("SMPSHMIZUHO",szRtnCode,(char*)pRetStr);
    printf("CONNECT MQ MANAGER RETURN CODE...[%s]\n",szRtnCode);
	if(strcmp(szRtnCode, "000") != 0)
	{
		printf("返回码...[%s]\n",szRtnCode);
		return 0 ;
	} 
	cout << "连接成功" << endl;
	
	char szErrDesc[256];
	memset(szErrDesc, 0, sizeof(szErrDesc));

	char szRecvMsg[10240];
	memset(szRecvMsg, 0, sizeof(szRecvMsg));
	
	char szFilePath[512];
	memset(szFilePath, 0, sizeof(szFilePath));
	sprintf(szFilePath,"/home/smprun/api/smps");
	
	char pQLName[128];
	sprintf(pQLName,"SMPPBC2SHMIZUHO");

	int nFileFlg = 0;
	
	while(1)
	{
		
		memset(szRecvMsg, 0, sizeof(szRecvMsg));
		memset(szRtnCode, 0, sizeof(szRtnCode));
		MB_RECEIVE(pId,pQLName,szRecvMsg,szFilePath,nFileFlg,szRtnCode,(char*)pRetStr);
        printf("RECEIVE RETURN CODE...[%s]\n",szRtnCode);
		if(strcmp(szRtnCode, "000") == 0)
		{
			cout << "szRecvMsg: " << szRecvMsg << endl;
		}else if(strcmp(szRtnCode, "201") == 0)
		{
			cout << "No Msg!" << endl;
			sleep(5);
			continue;
		}
		else
		{
			cout << "Error!:" << szRtnCode << endl;	
			sleep(5);
			continue;
		}
		
		/*
		
		处理报文
		*/
		char szProcFlag[2];
		memset(szProcFlag, 0, sizeof(szProcFlag));
		szProcFlag[0] = '0';
		MB_CONFIRM(pId, szProcFlag, szRtnCode,(char*)pRetStr);

		sleep(5);
	}


	MB_DISCONNECT(pId);
	return 0 ;
}

