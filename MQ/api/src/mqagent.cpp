/********************************************************************
�ļ�����MQAgent.cpp
�����ˣ�jerry_hu
��  �ڣ�2004-2-26
�޸��ˣ�aps-zgh
��  �ڣ�2008-07-28
��  ����MQAgent��ʵ���ļ�


��֪���⣺
	1��ȡ�����д����ַ���������ʱ��ȡ�����ķ���CStirng�У����ܵ���ʹ�ô���


��  ����
Copyright (c) 2008  SZFESC
********************************************************************/


#include	<errno.h>
#include "mqagent.hpp"
#include "cfile.hpp"
#include "sysrefdata.hpp"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>



void getSeno(char * szSysTime)/*ȡϵͳʱ��*/
{
	struct tm *nowtime;
	struct timeb tp;

	ftime (&tp);
	nowtime=localtime(&(tp.time));

	sprintf(szSysTime,"%04d",nowtime->tm_year+1900);
	sprintf(&szSysTime[4],"%02d",nowtime->tm_mon+1);
	sprintf(&szSysTime[6],"%02d",nowtime->tm_mday);
	sprintf(&szSysTime[8],"%02d",nowtime->tm_hour);
	sprintf(&szSysTime[10],"%02d",nowtime->tm_min);
	sprintf(&szSysTime[12],"%02d",nowtime->tm_sec);
	sprintf(&szSysTime[14],"%03d",tp.millitm);
}


MQAgent::MQAgent():m_MQManager(), m_buffer(MQ_MAX_SEGMENT_SIZE + 4096)
{
	m_pCurQueue = 0;
    m_nStatus = 0;
}

MQAgent::MQAgent(const char* szMgrName):m_MQManager(), m_buffer(MQ_MAX_SEGMENT_SIZE + 4096)
{
	m_pCurQueue = 0;
    m_nStatus = 0;
    if (szMgrName != NULL)
    {
        Init(szMgrName);
    }
}

MQAgent::~MQAgent()
{
	if (m_pCurQueue) 
	{
		delete m_pCurQueue;
		m_pCurQueue = 0;
	}	
}

int MQAgent::Init(const char* szMgrName)
{

	if (m_pCurQueue) 
	{
		delete m_pCurQueue;
		m_pCurQueue = 0;
	}

    if (!m_MQManager.Connect(szMgrName))
    {
        m_strErrText.Format("MQ:����MQ�������������[%d]",m_MQManager.GetReason());
        m_nStatus = 1;
        return -1;
    }

    return 0;
}

int MQAgent::Disconnect()
{
	if (m_pCurQueue) 
	{
		delete m_pCurQueue;
		m_pCurQueue = 0;
	}
  m_MQManager.Disconnect();
  return 0;
}

int MQAgent::GetMsg(LPCSTR szQueueName, CString& strMsg, CString& strFilePath,BOOL& bLongMsg,int nWait, const char* szMsgID)
{
	if (!OpenQueue(szQueueName, MQOQF_GET)) 
	{
		m_strErrText.Format("��Queue(%s)ʧ��(�������:%d)", szQueueName, m_MQManager.GetReason());
		return -1;
	}
	
	MQMsgInfo mqMsg;
	if (szMsgID != NULL)
		mqMsg.m_strMsgId = szMsgID;
	mqMsg.m_pBuffer = m_buffer;
	mqMsg.m_nBufferLen = m_buffer.GetLength();

    if (!m_pCurQueue->Get(mqMsg, nWait, szMsgID))
    {
    	if (m_pCurQueue->GetReason() == MQRC_NO_MSG_AVAILABLE)
			return 1;
		else
        {
			m_strErrText.Format("MQ Getʧ��(�������:%d)", m_pCurQueue->GetReason());
			return -1;
		}
    }

		//����
		memcpy(strMsg.GetBufferSetLength(mqMsg.m_nDataLen),m_buffer,mqMsg.m_nDataLen);

    if (mqMsg.m_iSegmentFlag == SF_NONESEGMENT) //δ�ֶ���Ϣ
    {
        bLongMsg = FALSE;
        return 0;
    }

    //�ֶδ���	
	  strFilePath = MakeExclusiveFilePath(strFilePath);
		printf("��ʼ�����ļ����浽%s",(LPCSTR)strFilePath);
    CFile fMsg;
    if (!fMsg.Open(strFilePath,O_CREAT|O_EXCL|O_TRUNC|O_RDWR|O_APPEND))
    {
        m_MQManager.Rollback();
        m_strErrText.Format("MQ�������ɽ��շֶα��ĵı����ļ�[%s]��!",(LPCSTR)strFilePath);
        return -1;
    }

    bLongMsg = TRUE;

	if (!m_pCurQueue->Get(mqMsg, nWait))
	{
		if (m_pCurQueue->GetReason() == MQRC_NO_MSG_AVAILABLE)
		{
			m_MQManager.Rollback();
			m_strErrText = "MQ���������ڽ��շֶα���ʱ��ʱ";
			return -1;
		}
		else
		{
			m_MQManager.Rollback();
			m_strErrText.Format("MQ Getʧ��(�������:%d)", m_pCurQueue->GetReason());
			return -1;
		}
	}

    while(mqMsg.m_iSegmentFlag != SF_LASTSEGMENT)
    {

        if (mqMsg.m_iSegmentFlag != SF_SEGMENT)
        {
            m_MQManager.Rollback();
            m_strErrText = "MQ������ִ����ڽ��շֶα��ĵ�ʱ���յ��˷Ƿֶα���!";
            return -1;

        }

        if (!fMsg.Write(mqMsg.m_pBuffer,mqMsg.m_nDataLen))
        {
            m_MQManager.Rollback();
            m_strErrText = "MQ�������ɽ��շֶα��ĵı����ļ���!";
            return -1;
        }       

		if (!m_pCurQueue->Get(mqMsg, nWait))
        {
    	    if (m_pCurQueue->GetReason() == MQRC_NO_MSG_AVAILABLE)
            {
                m_MQManager.Rollback();
                m_strErrText = "MQ���������ڽ��շֶα���ʱ��ʱ";
                return -1;
            }
		    else
            {
               m_MQManager.Rollback();
			    m_strErrText.Format("MQ Getʧ��(�������:%d)", m_pCurQueue->GetReason());
			    return -1;
		    }
        }
   }

    //���һ���ֶα���Ƭ
    if (!fMsg.Write(mqMsg.m_pBuffer,mqMsg.m_nDataLen))
    {
        m_MQManager.Rollback();
        m_strErrText = "MQ�������ɽ��շֶα��ĵı����ļ���!";
        return -1;
    }
	
		if ( 0 != ReplaceMsgFilePath(strMsg,strFilePath) )
		{
			printf("�޸���Ϣ�и����ļ�·��ʧ��\n");
			return -1;
		}
	return 0;    
}

//����и����ļ������Ƭ���ͣ��ڶ�Ƭ��ʼΪ�ļ�
int MQAgent::PutMsg(LPCSTR szQueueName, const char* szMsg,const char* szMsgID,int nMsgLen,const char* szFilePath)
{
    if (nMsgLen  <= 0) nMsgLen = strlen(szMsg); 

    if (szQueueName == NULL)
    {
        m_strErrText.Format("MQ���������������szQueueName == NULL");
        return -1;
    }

    if (strlen(szMsg) > MAX_MQMSGFRAME_LENGTH)
    {
        m_strErrText = "MQ����:д����Ϣ����";
        return -1;
    }

    MQMsgInfo mqMsg;

    // ��������
    mqMsg.m_iMsgType = MT_NORMAL;
    
    // �������ȼ�
    mqMsg.m_nPriority = MPR_COMMON;
    
    // ����ID
    if (szMsgID)
    	mqMsg.m_strMsgId = szMsgID;
    
    memcpy(m_buffer,szMsg,nMsgLen);
    mqMsg.m_pBuffer = m_buffer;
    mqMsg.m_nDataLen= nMsgLen;
    mqMsg.m_nBufferLen = nMsgLen;
    
    CFile fSnd;
    
//	if (NULL != szFilePath)
//	printf("strlen(szFilePath)..[%d]\n",strlen(szFilePath));
    if ( strlen(szFilePath) != 0 )
    {
        if (!fSnd.Open(szFilePath,O_RDONLY))
        {
            m_strErrText.Format("MQ����:��Ҫ���͵��ļ�:[%s]ʧ��,ԭ��[%s]",szFilePath,strerror(errno));
            return -1;
        }

        mqMsg.m_iSegmentFlag = SF_SEGMENT;
    }
	

	//�򿪶���
	if (!OpenQueue(szQueueName, MQOQF_PUT)) 
	{
		m_strErrText.Format("��Queue(%s)ʧ��(�������:%d)", szQueueName, m_MQManager.GetReason());
		return -1;
	}
	

	//���ͱ���
	if( !m_pCurQueue->Put(mqMsg) )
	{
		m_strErrText.Format("MQ����,����:%s д����Ϣʧ��(�������:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_MQManager.GetReason());
        return -1;
	}
	
	printf("MQAgent::PutMsg  �����[%s]���ͱ���[%s]\n", szQueueName, szMsg);

	//�����ļ�
//	if (NULL != szFilePath)
	if ( strlen(szFilePath) != 0 )
	{
		char sdbuf[MQ_MAX_SEGMENT_SIZE+1];
		memset(sdbuf,0,sizeof(sdbuf));

		printf("�����ļ�[%s]\n", szFilePath);
		int nReadCount = MQ_MAX_SEGMENT_SIZE;
		if (!fSnd.ReadN(sdbuf,nReadCount))
		{
			m_strErrText.Format("MQ����:��Ҫ���͵��ļ�:%s������ʧ��",szFilePath);
			RollBack();
			return -1;
		}

		//�������һ��
		while (nReadCount == MQ_MAX_SEGMENT_SIZE)
		{
			memcpy(m_buffer,sdbuf,nReadCount);
			mqMsg.m_pBuffer = m_buffer;
			mqMsg.m_nDataLen= nReadCount; //strlen(sdbuf);
			mqMsg.m_nBufferLen = nReadCount; //strlen(sdbuf);

			if( !m_pCurQueue->Put(mqMsg) )
			{
				m_strErrText.Format("WTMQ����,����:%s д����Ϣʧ��(�������:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_pCurQueue->GetReason());
				RollBack();
				return -1;
			}

			nReadCount = MQ_MAX_SEGMENT_SIZE;
			memset(sdbuf,0,sizeof(sdbuf));
			if (!fSnd.ReadN(sdbuf,nReadCount))
			{
				RollBack();
				m_strErrText.Format("MQ����:��Ҫ���͵��ļ�:%s������ʧ��",szFilePath);
				return -1;
			}
		} //����ļ��պõ���MQ_MAX_SEGMENT_SIZE���������᲻��������?

		//���һ��
		memcpy(m_buffer,sdbuf,nReadCount);
		mqMsg.m_nDataLen= nReadCount;
		mqMsg.m_nBufferLen = nReadCount;
		mqMsg.m_iSegmentFlag = SF_LASTSEGMENT;
		mqMsg.m_pBuffer = m_buffer;
		if( !m_pCurQueue->Put(mqMsg) )
		{
			m_strErrText.Format("MQ����,����:%s д����Ϣʧ��(�������:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_pCurQueue->GetReason());
			m_MQManager.Rollback();
			return -1;
		}
    }

	// �ύ������
	if( !m_MQManager.Commit() )
	{
		m_strErrText.Format("MQManager Commit(Put)(%s)ʧ��(�������:%d)",
			(LPCSTR)m_pCurQueue->GetQueueName(), m_pCurQueue->GetReason());
		return -1;
	}

	return 0;    
}


BOOL MQAgent::OpenQueue(LPCSTR pQueueName, int iOpenFlag)
{
	if (m_pCurQueue) 
	{
		delete m_pCurQueue;
		m_pCurQueue = 0;
	}
	m_pCurQueue =(IMQQueue*)m_MQManager.OpenQueue(pQueueName, iOpenFlag);
	if (!m_pCurQueue)
	{
	    m_strErrText.Format("MQopen Error, iOpenFlag=(%d),Code=(%d), FILE=%s, LINE=%d\n", iOpenFlag, m_MQManager.GetReason(), __FILE__, __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL MQAgent::RollBack()
{
    return m_MQManager.Rollback();
}

BOOL MQAgent::Commit()
{
    return m_MQManager.Commit();
}

CString MakeExclusiveFilePath(const char* szPath)
{
    CString strFileName;
    char szSerNo[20];
    memset(szSerNo,0,sizeof(szSerNo));
//    SysRefDataSvr svr(TRUE,TRUE);
//    int nSerNo = svr.IncOnlyIndex();
    getSeno(szSerNo);
    strFileName.Format("%s%s",szSerNo,".mbd");
//    strFileName.Format("%s%04d%s",GetTime("YYMMDDHHMMSS"),nSerNo,".mbd");

    if (szPath)
    {
        CString strPath = szPath;
        strFileName = strPath + '/' + strFileName;
    }

    return strFileName;
}


int ReplaceMsgFilePath(CString& strMsg,const char* szFilePath)
{
	int nStart = strMsg.Find("{F:");
	if (nStart < 0) return -1;
	
	int nEnd = strMsg.Find("}",nStart);
	if (nEnd < 0) return -1;

	CString strSrc = strMsg.Mid(nStart,nEnd - nStart);
	CString strNew = "{F:";
	strNew += szFilePath;
	int nCount = strMsg.Replace(strSrc,strNew);
	if (nCount < 1)
		return -1;
	else
		return 0;
}
