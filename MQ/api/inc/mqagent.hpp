/********************************************************************
文件名：mqagentt.hpp
创建人：jerry_hu
日  期：2004-2-26
修改人：aps-zgh
日  期：2008-07-28
描  述：MQAgent类头文件

版  本：
Copyright (c) 2008  SZFESC
********************************************************************/
#ifndef __H_SMPS_MQAGNET_H
#define __H_SMPS_MQAGNET_H

#include "mq.hpp"
#include "cstring.hpp"

#define MQ_MAX_SEGMENT_SIZE 1024*25 //MQ消息最大长度为25K

#define MAX_MQMSGFRAME_LENGTH  1024*25 //25*1024

class MQAgent
{
public:
    MQAgent();
    MQAgent(const char* szMgrName);
    ~MQAgent();

    //初始化
    int Init(const char* szMgrName);

    //取报文 如果是分片报文则将第二片开始存为文件 bLongMsg = TRUE strFileName为保存的唯一文件名
		//nWait 为等待返回的时间(如果这段时间内没有消息则不返回) szMsgID为要读取消息的MsgID
    int GetMsg(LPCSTR szQueueName, CString& strMsg, CString& strFilePath,BOOL& bLongMsg, int nWait = 0,const char* szMsgID=NULL);

    //发送短报文
		//szMsg 发送缓冲区 如果nMsgLen = 0发送的是字符串 strlen得到长度
		//szFilePath 发送附加文件的文件路径 = NULL没有要发送的文件
		//szMsgID 为消息的MsgID为空则不填
    int PutMsg(LPCSTR szQueueName, const char* szMsg,const char* szMsgID = NULL,int nMsgLen = 0,const char* szFilePath = NULL);

    CString GetErrorText() { return m_strErrText; }

		IMQManager* GetMQManager(){return &m_MQManager;}
		
		int Disconnect();

	
    BOOL RollBack();
    BOOL Commit();

protected:
    BOOL OpenQueue(LPCSTR pQueueName, int iOpenFlag);

    IMQQueue* m_pCurQueue;
		IMQManager m_MQManager;
    SafeBuffer m_buffer;

    CString m_strErrText;
    int m_nStatus;
};

CString MakeExclusiveFilePath(const char* szPath);
int ReplaceMsgFilePath(CString& strMsg,const char* szFilePath);

#endif //#ifndef __H_SMPS_MQAGNET_H

