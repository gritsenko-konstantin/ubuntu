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
	memset(pRetStr  , 0, sizeof(pRetStr));	


 	pId = MB_CONNECT("SMPSHMIZUHO",szRtnCode, (char*)pRetStr);

	if(strcmp(szRtnCode, "000") != 0)
	{
		printf("������...[%s][%s]\n",szRtnCode,pRetStr);
		return 0;
	}

	cout << "���ӳɹ�" << endl;
	
	char szErrDesc[256];
	memset(szErrDesc, 0, sizeof(szErrDesc));

	char szSendMsg[1024];
	memset(szSendMsg, 0, sizeof(szSendMsg));
	
	char szFilePath[1024];
	memset(szFilePath, 0, sizeof(szFilePath));
	
	char pQLName[128];
	memset(pQLName, 0, sizeof(pQLName));
	sprintf(pQLName,"SMPPBC2SHMIZUHO");
	
	FILE *fp;
	if( (fp = fopen("smp.txt", "r")) != NULL )
	{
		while(!feof(fp))
		{
			memset(szSendMsg, 0, sizeof(szSendMsg));
       	memset(szRtnCode, 0, sizeof(szRtnCode));
      	memset(szErrDesc, 0, sizeof(szErrDesc));
			fgets(szSendMsg, sizeof(szSendMsg), fp);

			if(szSendMsg[0] == '#')
				continue;

			if(strlen(szSendMsg) < 10)
				continue;

			cout << "����MB_SEND" << endl;
			
			MB_SEND(pId,pQLName,szSendMsg,szFilePath,szRtnCode,(char*)pRetStr);
			printf("szRtnCode...[%s]\n",szRtnCode);

			if(strncmp(szRtnCode, "000",3) == 0)
			{
				cout << "����ҵ��ɹ�" << endl;
				cout << "szRtnCode: " << szRtnCode << endl;
				cout << "szErrDesc: " << szErrDesc << endl;
			}
			else
			{
				cout << "����ҵ��ʧ��" << endl;
				cout << "szRtnCode: " << szRtnCode << endl;
			}
			
		}
	}
	else
		cout << "open smp.txt error!" << endl;

	fclose(fp);
	
	memset(szSendMsg, 0, sizeof(szSendMsg));
	memset(szRtnCode, 0, sizeof(szRtnCode));
	memset(szErrDesc, 0, sizeof(szErrDesc));
	sprintf(szSendMsg,"{1:       S612    001591075715ETS01         CMT61220070806162038                    2007080620070806162038          9                    }{F:/home/smps/070806154813206.mbd}{C:                                }");
	
	sprintf(szFilePath,"/home/smprun/api/smps/070806154813206.mbd");
	
/*	MB_SEND(pId,pQLName,szSendMsg,szFilePath,szRtnCode,szErrDesc);*/
	
	if(strncmp(szRtnCode, "000",3) == 0)
	{
		cout << "�����ļ��ɹ�" << endl;
		cout << "szRtnCode: " << szRtnCode << endl;
		cout << "szErrDesc: " << szErrDesc << endl;
	}
	else
	{
		cout << "�����ļ�ʧ��" << endl;
		cout << "szRtnCode: " << szRtnCode << endl;
		cout << "szErrDesc: " << szErrDesc << endl;		
	}
	
	MB_DISCONNECT(pId);
	
	return 0;
}

