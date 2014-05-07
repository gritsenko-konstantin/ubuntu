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
		printf("������...[%s]\n",szRtnCode);
		return;
	}

	cout << "���ӳɹ�" << endl;
	
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
		
		������
		*/
		char szProcFlag[2];
		memset(szProcFlag, 0, sizeof(szProcFlag));
		szProcFlag[0] = '0';
		MB_CONFIRM(pId, szProcFlag, szRtnCode,szErrDesc);

	}


	MB_DISCONNECT(pId);
}

