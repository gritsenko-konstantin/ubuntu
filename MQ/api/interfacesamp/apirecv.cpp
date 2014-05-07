/********************************************************************
�ļ�����apirecv.cpp
�����ˣ�aps-zgh
��  �ڣ�2008-08-19
�޸��ˣ�
��  �ڣ�
��  ������������

��  ����
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
		printf("������...[%s]\n",szRtnCode);
		return 0 ;
	} 
	cout << "���ӳɹ�" << endl;
	
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
		
		������
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

