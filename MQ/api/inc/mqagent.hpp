/********************************************************************
�ļ�����mqagentt.hpp
�����ˣ�jerry_hu
��  �ڣ�2004-2-26
�޸��ˣ�aps-zgh
��  �ڣ�2008-07-28
��  ����MQAgent��ͷ�ļ�

��  ����
Copyright (c) 2008  SZFESC
********************************************************************/
#ifndef __H_SMPS_MQAGNET_H
#define __H_SMPS_MQAGNET_H

#include "mq.hpp"
#include "cstring.hpp"

#define MQ_MAX_SEGMENT_SIZE 1024*25 //MQ��Ϣ��󳤶�Ϊ25K

#define MAX_MQMSGFRAME_LENGTH  1024*25 //25*1024

class MQAgent
{
public:
    MQAgent();
    MQAgent(const char* szMgrName);
    ~MQAgent();

    //��ʼ��
    int Init(const char* szMgrName);

    //ȡ���� ����Ƿ�Ƭ�����򽫵ڶ�Ƭ��ʼ��Ϊ�ļ� bLongMsg = TRUE strFileNameΪ�����Ψһ�ļ���
		//nWait Ϊ�ȴ����ص�ʱ��(������ʱ����û����Ϣ�򲻷���) szMsgIDΪҪ��ȡ��Ϣ��MsgID
    int GetMsg(LPCSTR szQueueName, CString& strMsg, CString& strFilePath,BOOL& bLongMsg, int nWait = 0,const char* szMsgID=NULL);

    //���Ͷ̱���
		//szMsg ���ͻ����� ���nMsgLen = 0���͵����ַ��� strlen�õ�����
		//szFilePath ���͸����ļ����ļ�·�� = NULLû��Ҫ���͵��ļ�
		//szMsgID Ϊ��Ϣ��MsgIDΪ������
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

