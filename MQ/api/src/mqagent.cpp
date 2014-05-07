/********************************************************************
文件名：MQAgent.cpp
创建人：jerry_hu
日  期：2004-2-26
修改人：aps-zgh
日  期：2008-07-28
描  述：MQAgent类实现文件


已知问题：
	1、取报文中存在字符串结束符时，取出报文放在CStirng中，可能导致使用错误


版  本：
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



void getSeno(char * szSysTime)/*取系统时间*/
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
        m_strErrText.Format("MQ:连接MQ管理器出错代码[%d]",m_MQManager.GetReason());
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
		m_strErrText.Format("打开Queue(%s)失败(出错代码:%d)", szQueueName, m_MQManager.GetReason());
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
			m_strErrText.Format("MQ Get失败(出错代码:%d)", m_pCurQueue->GetReason());
			return -1;
		}
    }

		//报文
		memcpy(strMsg.GetBufferSetLength(mqMsg.m_nDataLen),m_buffer,mqMsg.m_nDataLen);

    if (mqMsg.m_iSegmentFlag == SF_NONESEGMENT) //未分段消息
    {
        bLongMsg = FALSE;
        return 0;
    }

    //分段处理	
	  strFilePath = MakeExclusiveFilePath(strFilePath);
		printf("开始接收文件保存到%s",(LPCSTR)strFilePath);
    CFile fMsg;
    if (!fMsg.Open(strFilePath,O_CREAT|O_EXCL|O_TRUNC|O_RDWR|O_APPEND))
    {
        m_MQManager.Rollback();
        m_strErrText.Format("MQ错误：生成接收分段报文的报文文件[%s]错!",(LPCSTR)strFilePath);
        return -1;
    }

    bLongMsg = TRUE;

	if (!m_pCurQueue->Get(mqMsg, nWait))
	{
		if (m_pCurQueue->GetReason() == MQRC_NO_MSG_AVAILABLE)
		{
			m_MQManager.Rollback();
			m_strErrText = "MQ致命错误：在接收分段报文时超时";
			return -1;
		}
		else
		{
			m_MQManager.Rollback();
			m_strErrText.Format("MQ Get失败(出错代码:%d)", m_pCurQueue->GetReason());
			return -1;
		}
	}

    while(mqMsg.m_iSegmentFlag != SF_LASTSEGMENT)
    {

        if (mqMsg.m_iSegmentFlag != SF_SEGMENT)
        {
            m_MQManager.Rollback();
            m_strErrText = "MQ超级奇怪错误：在接收分段报文的时候收到了非分段报文!";
            return -1;

        }

        if (!fMsg.Write(mqMsg.m_pBuffer,mqMsg.m_nDataLen))
        {
            m_MQManager.Rollback();
            m_strErrText = "MQ错误：生成接收分段报文的报文文件错!";
            return -1;
        }       

		if (!m_pCurQueue->Get(mqMsg, nWait))
        {
    	    if (m_pCurQueue->GetReason() == MQRC_NO_MSG_AVAILABLE)
            {
                m_MQManager.Rollback();
                m_strErrText = "MQ致命错误：在接收分段报文时超时";
                return -1;
            }
		    else
            {
               m_MQManager.Rollback();
			    m_strErrText.Format("MQ Get失败(出错代码:%d)", m_pCurQueue->GetReason());
			    return -1;
		    }
        }
   }

    //最后一个分段报文片
    if (!fMsg.Write(mqMsg.m_pBuffer,mqMsg.m_nDataLen))
    {
        m_MQManager.Rollback();
        m_strErrText = "MQ错误：生成接收分段报文的报文文件错!";
        return -1;
    }
	
		if ( 0 != ReplaceMsgFilePath(strMsg,strFilePath) )
		{
			printf("修改消息中附加文件路径失败\n");
			return -1;
		}
	return 0;    
}

//如果有附加文件，则分片发送，第二片开始为文件
int MQAgent::PutMsg(LPCSTR szQueueName, const char* szMsg,const char* szMsgID,int nMsgLen,const char* szFilePath)
{
    if (nMsgLen  <= 0) nMsgLen = strlen(szMsg); 

    if (szQueueName == NULL)
    {
        m_strErrText.Format("MQ错误：输入参数错误szQueueName == NULL");
        return -1;
    }

    if (strlen(szMsg) > MAX_MQMSGFRAME_LENGTH)
    {
        m_strErrText = "MQ错误:写入消息超长";
        return -1;
    }

    MQMsgInfo mqMsg;

    // 设置类型
    mqMsg.m_iMsgType = MT_NORMAL;
    
    // 设置优先级
    mqMsg.m_nPriority = MPR_COMMON;
    
    // 设置ID
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
            m_strErrText.Format("MQ错误:打开要发送的文件:[%s]失败,原因：[%s]",szFilePath,strerror(errno));
            return -1;
        }

        mqMsg.m_iSegmentFlag = SF_SEGMENT;
    }
	

	//打开队列
	if (!OpenQueue(szQueueName, MQOQF_PUT)) 
	{
		m_strErrText.Format("打开Queue(%s)失败(出错代码:%d)", szQueueName, m_MQManager.GetReason());
		return -1;
	}
	

	//发送报文
	if( !m_pCurQueue->Put(mqMsg) )
	{
		m_strErrText.Format("MQ错误,队列:%s 写入消息失败(出错代码:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_MQManager.GetReason());
        return -1;
	}
	
	printf("MQAgent::PutMsg  向队列[%s]发送报文[%s]\n", szQueueName, szMsg);

	//发送文件
//	if (NULL != szFilePath)
	if ( strlen(szFilePath) != 0 )
	{
		char sdbuf[MQ_MAX_SEGMENT_SIZE+1];
		memset(sdbuf,0,sizeof(sdbuf));

		printf("发送文件[%s]\n", szFilePath);
		int nReadCount = MQ_MAX_SEGMENT_SIZE;
		if (!fSnd.ReadN(sdbuf,nReadCount))
		{
			m_strErrText.Format("MQ错误:从要发送的文件:%s读内容失败",szFilePath);
			RollBack();
			return -1;
		}

		//不是最后一段
		while (nReadCount == MQ_MAX_SEGMENT_SIZE)
		{
			memcpy(m_buffer,sdbuf,nReadCount);
			mqMsg.m_pBuffer = m_buffer;
			mqMsg.m_nDataLen= nReadCount; //strlen(sdbuf);
			mqMsg.m_nBufferLen = nReadCount; //strlen(sdbuf);

			if( !m_pCurQueue->Put(mqMsg) )
			{
				m_strErrText.Format("WTMQ错误,队列:%s 写入消息失败(出错代码:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_pCurQueue->GetReason());
				RollBack();
				return -1;
			}

			nReadCount = MQ_MAX_SEGMENT_SIZE;
			memset(sdbuf,0,sizeof(sdbuf));
			if (!fSnd.ReadN(sdbuf,nReadCount))
			{
				RollBack();
				m_strErrText.Format("MQ错误:从要发送的文件:%s读内容失败",szFilePath);
				return -1;
			}
		} //如果文件刚好等于MQ_MAX_SEGMENT_SIZE的整数倍会不会有问题?

		//最后一段
		memcpy(m_buffer,sdbuf,nReadCount);
		mqMsg.m_nDataLen= nReadCount;
		mqMsg.m_nBufferLen = nReadCount;
		mqMsg.m_iSegmentFlag = SF_LASTSEGMENT;
		mqMsg.m_pBuffer = m_buffer;
		if( !m_pCurQueue->Put(mqMsg) )
		{
			m_strErrText.Format("MQ错误,队列:%s 写入消息失败(出错代码:%d)",(LPCSTR)m_pCurQueue->GetQueueName(),m_pCurQueue->GetReason());
			m_MQManager.Rollback();
			return -1;
		}
    }

	// 提交该事务
	if( !m_MQManager.Commit() )
	{
		m_strErrText.Format("MQManager Commit(Put)(%s)失败(出错代码:%d)",
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
