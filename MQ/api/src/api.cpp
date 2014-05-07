/********************************************************************
文件名：api.cpp
创建人：aps-zgh
日  期：2008-08-19
修改人：
日  期：
描  述：上海市支付结算综合业务系统前置机API接口实现文件

版  本：
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
		
  	if( nRet != 0)	// 表示初始化出错
  	{
  		strcpy(pRetCode, "102");
  		CString strNewTmp = myAgent.GetErrorText();
  		int len = strNewTmp.GetLength();
  		sprintf(pRetInfo,"MQ连接失败[%s]",strNewTmp.GetBuffer(len));
  		strNewTmp.ReleaseBuffer();
  	}
  	else
  	{
  		strcpy(pRetCode, "000");//连接成功
  		sprintf(pRetInfo,"连接成功");
  	}
   
		return pRetCode;
}

//断开连接
void MB_DISCONNECT(void *pID)
{
	myAgent.Disconnect();
	return; 
}


//发送api
void MB_SEND(void *pID, char *pQLName,char *pInPutMsg, char *pInPath,char *pRetCode, char *pRetInfo)
{    			    
    if (NULL == pQLName)
    {
        sprintf(pRetInfo,"发送队列不能空");
        strcpy(pRetCode, "102");
        return;
    }
		
		
    if (strlen(pInPutMsg) == 0)
    {
        sprintf(pRetInfo,"pInPutMsg 参数不能为空");
        strcpy(pRetCode, "102");
        return;
    }
		
    if ( strlen(pInPutMsg) > MAX_LENGTH )
    {
        sprintf(pRetInfo,"报文长度大于20K，请用文件形式发送");
        strcpy(pRetCode, "102");
        return;
    }
		
    int nRet = myAgent.PutMsg(pQLName, pInPutMsg,"11111111111111111111",strlen(pInPutMsg),pInPath);
		
    if( nRet != 0)	// 系统错误，商业银行检查故障后可重发
    {
        CString strNewTmp = myAgent.GetErrorText();
        int len = strNewTmp.GetLength();
        sprintf(pRetInfo,"报文发送失败[%s]",strNewTmp.GetBuffer(len));
        strNewTmp.ReleaseBuffer();
        sprintf(pRetInfo,"报文发送失败");
        strcpy(pRetCode, "102");
        return;
    }
    else
    {
        sprintf(pRetInfo,"报文发送成功");
        strcpy(pRetCode, "000");
        return;
    }

}
		
		
//接收api
void MB_RECEIVE(void *pID, char *pQLName, char *pOutPutMsg, char *pOutPath,int &nFileFlg, char *pRetCode, char *pRetInfo)
{
				
		if (NULL == pQLName)
		{
			printf("接收队列不能空");
			strcpy(pRetCode, "102");
			return;
		}
		 
		
		CString strMsg,strFilePath;
		strFilePath.Format("%s",pOutPath);

		int nRet = myAgent.GetMsg(pQLName, strMsg,strFilePath,nFileFlg);
		

		if( nRet == 1)
  	{
 			printf("没有接收到报文");
			strcpy(pRetCode, "201");
			sprintf(pRetInfo,"没有接收到报文");
  	}
  	else if (nRet == 0)
  	{
  		printf("报文接收成功");
			strncpy(pOutPutMsg ,strMsg,strMsg.GetLength() );
			//strncpy(pOutPath ,strFilePath,strFilePath.GetLength() );
			strcpy(pRetCode, "000");
  		sprintf(pRetInfo,"报文接收成功");
  	}
  	else
  	{
			printf("报文接收失败");
			CString strNewTmp = myAgent.GetErrorText();
  		    int len = strNewTmp.GetLength();
			sprintf(pRetInfo,"报文接收失败[%s]",strNewTmp.GetBuffer(len));
			strNewTmp.ReleaseBuffer();
			printf("原因：[%s]\n",(LPCSTR)myAgent.GetErrorText());
			strcpy(pRetCode, "102");
  	}
  	
  	return;

}

void MB_CONFIRM(void *pId, char *szProcFlag, char *szRtnCode, char *pRetInfo)
{
	bool bFlag;
	if(szProcFlag[0] == '0')//成功
	{
		cout <<"commit" << endl;
		bFlag = myAgent.Commit();
	}
	else//失败
	{
		cout <<"rollback" << endl;
		bFlag = myAgent.RollBack();
	}
	
	// 提交该事务
	if (!bFlag)
	{
		printf("确认失败[%s]",(LPCSTR)myAgent.GetErrorText());
		strcpy(szRtnCode, "102");
		CString strNewTmp = myAgent.GetErrorText();
        int len = strNewTmp.GetLength();
		sprintf(pRetInfo,"确认失败[%s]",strNewTmp.GetBuffer(len));
		strNewTmp.ReleaseBuffer();
		return;
	}
	strcpy(szRtnCode, "000");
	sprintf(pRetInfo,"确认成功");
	return;	
}
