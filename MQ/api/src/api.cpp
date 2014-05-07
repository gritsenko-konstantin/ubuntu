/********************************************************************
�ļ�����api.cpp
�����ˣ�aps-zgh
��  �ڣ�2008-08-19
�޸��ˣ�
��  �ڣ�
��  �����Ϻ���֧�������ۺ�ҵ��ϵͳǰ�û�API�ӿ�ʵ���ļ�

��  ����
Copyright (c) 2008  SZFESC
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;
#include <unistd.h>
#include "cstring.hpp"
#include "api.hpp"
#include "mq.hpp"
#include "mqagent.hpp"

#define MAX_LENGTH  1024*22 //20k
 
MQAgent myAgent;

void * MB_CONNECT(char * pQMRName ,char *pRetCode, char *pRetInfo)
{
		int nRet = myAgent.Init(pQMRName);
		
  	if( nRet != 0)	// ��ʾ��ʼ������
  	{
  		strcpy(pRetCode, "102");
  		CString strNewTmp = myAgent.GetErrorText();
  		int len = strNewTmp.GetLength();
  		sprintf(pRetInfo,"MQ����ʧ��[%s]",strNewTmp.GetBuffer(len));
  		strNewTmp.ReleaseBuffer();
  	}
  	else
  	{
  		strcpy(pRetCode, "000");//���ӳɹ�
  		sprintf(pRetInfo,"���ӳɹ�");
  	}
   
		return pRetCode;
}

//�Ͽ�����
void MB_DISCONNECT(void *pID)
{
	myAgent.Disconnect();
	return; 
}


//����api
void MB_SEND(void *pID, char *pQLName,char *pInPutMsg, char *pInPath,char *pRetCode, char *pRetInfo)
{    			    
    if (NULL == pQLName)
    {
        sprintf(pRetInfo,"���Ͷ��в��ܿ�");
        strcpy(pRetCode, "102");
        return;
    }
		
		
    if (strlen(pInPutMsg) == 0)
    {
        sprintf(pRetInfo,"pInPutMsg ��������Ϊ��");
        strcpy(pRetCode, "102");
        return;
    }
		
    if ( strlen(pInPutMsg) > MAX_LENGTH )
    {
        sprintf(pRetInfo,"���ĳ��ȴ���20K�������ļ���ʽ����");
        strcpy(pRetCode, "102");
        return;
    }
		
    int nRet = myAgent.PutMsg(pQLName, pInPutMsg,"11111111111111111111",strlen(pInPutMsg),pInPath);
		
    if( nRet != 0)	// ϵͳ������ҵ���м����Ϻ���ط�
    {
        CString strNewTmp = myAgent.GetErrorText();
        int len = strNewTmp.GetLength();
        sprintf(pRetInfo,"���ķ���ʧ��[%s]",strNewTmp.GetBuffer(len));
        strNewTmp.ReleaseBuffer();
        sprintf(pRetInfo,"���ķ���ʧ��");
        strcpy(pRetCode, "102");
        return;
    }
    else
    {
        sprintf(pRetInfo,"���ķ��ͳɹ�");
        strcpy(pRetCode, "000");
        return;
    }

}
		
		
//����api
void MB_RECEIVE(void *pID, char *pQLName, char *pOutPutMsg, char *pOutPath,int &nFileFlg, char *pRetCode, char *pRetInfo)
{
				
		if (NULL == pQLName)
		{
			printf("���ն��в��ܿ�");
			strcpy(pRetCode, "102");
			return;
		}
		 
		
		CString strMsg,strFilePath;
		strFilePath.Format("%s",pOutPath);

		int nRet = myAgent.GetMsg(pQLName, strMsg,strFilePath,nFileFlg);
		

		if( nRet == 1)
  	{
 			printf("û�н��յ�����");
			strcpy(pRetCode, "201");
			sprintf(pRetInfo,"û�н��յ�����");
  	}
  	else if (nRet == 0)
  	{
  		printf("���Ľ��ճɹ�");
			strncpy(pOutPutMsg ,strMsg,strMsg.GetLength() );
			//strncpy(pOutPath ,strFilePath,strFilePath.GetLength() );
			strcpy(pRetCode, "000");
  		sprintf(pRetInfo,"���Ľ��ճɹ�");
  	}
  	else
  	{
			printf("���Ľ���ʧ��");
			CString strNewTmp = myAgent.GetErrorText();
  		    int len = strNewTmp.GetLength();
			sprintf(pRetInfo,"���Ľ���ʧ��[%s]",strNewTmp.GetBuffer(len));
			strNewTmp.ReleaseBuffer();
			printf("ԭ��[%s]\n",(LPCSTR)myAgent.GetErrorText());
			strcpy(pRetCode, "102");
  	}
  	
  	return;

}

void MB_CONFIRM(void *pId, char *szProcFlag, char *szRtnCode, char *pRetInfo)
{
	bool bFlag;
	if(szProcFlag[0] == '0')//�ɹ�
	{
		cout <<"commit" << endl;
		bFlag = myAgent.Commit();
	}
	else//ʧ��
	{
		cout <<"rollback" << endl;
		bFlag = myAgent.RollBack();
	}
	
	// �ύ������
	if (!bFlag)
	{
		printf("ȷ��ʧ��[%s]",(LPCSTR)myAgent.GetErrorText());
		strcpy(szRtnCode, "102");
		CString strNewTmp = myAgent.GetErrorText();
        int len = strNewTmp.GetLength();
		sprintf(pRetInfo,"ȷ��ʧ��[%s]",strNewTmp.GetBuffer(len));
		strNewTmp.ReleaseBuffer();
		return;
	}
	strcpy(szRtnCode, "000");
	sprintf(pRetInfo,"ȷ�ϳɹ�");
	return;	
}
