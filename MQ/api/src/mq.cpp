#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/uio.h>
#endif
#include "mq.hpp"


IMQManager::IMQManager(LPCSTR pMgrName)
{
	m_bIsConnected = FALSE;
	MQOD od = { MQOD_DEFAULT		};
	memcpy(&m_od, &od, sizeof(od));
	m_iReason = 0;
	Connect(pMgrName);
}

IMQManager::IMQManager()
{
	m_bIsConnected = FALSE;
	MQOD od = { MQOD_DEFAULT		};
	memcpy(&m_od, &od, sizeof(od));
	m_iReason = 0;
}

void IMQManager::Disconnect()
{
	if (m_bIsConnected)
		MQDISC(&m_hConn, &m_iComCode, &m_iReason);
	m_bIsConnected = FALSE;
}

BOOL IMQManager::Connect(LPCSTR pMgrName)
{
	if (m_bIsConnected)
		Disconnect();

	if (!pMgrName)
		pMgrName = " ";
	MQCONN((char*)pMgrName, &m_hConn, &m_iComCode, &m_iReason);
	if (m_iComCode != MQCC_FAILED)
		m_bIsConnected = TRUE;
	else
		m_bIsConnected = FALSE;
	return m_bIsConnected;
}
	
MQQueue* IMQManager::OpenQueue(NameList& pQueueNames)
{
	IMQQueue* pQueue = 0;
	if (!m_bIsConnected)
		return pQueue;
	MQLONG iOptions = MQOO_OUTPUT + MQOO_FAIL_IF_QUIESCING;
	MQHOBJ hQueue;
	int nCnt = pQueueNames.GetItemCount();
	MQOR* pObjRecPtr = new MQOR[nCnt];
	memset(pObjRecPtr, ' ', sizeof(MQOR) * nCnt);
	MQOR* ptr = pObjRecPtr;
	for (int i = 0; i < nCnt; i++) {
		strncpy(ptr->ObjectName, pQueueNames.Get(), MQ_Q_NAME_LENGTH);
		ptr ++;
	}
	m_od.Version = MQOD_VERSION_2;
	m_od.ObjectName[0] = 0;
	m_od.RecsPresent = nCnt;
	m_od.ObjectRecPtr = pObjRecPtr;
	MQOPEN(m_hConn, &m_od, iOptions, &hQueue, &m_iComCode, &m_iReason);
	delete[] pObjRecPtr;
	if (m_iComCode != MQCC_OK) 
		return pQueue;
	else 
		return pQueue = new IMQQueue(m_hConn, hQueue, pQueueNames.ToString(), MQOQF_PUT);
}

MQQueue* IMQManager::OpenQueue(LPCSTR pQueueName, int iOpenFlag)
{
	IMQQueue* pQueue = 0;
	if (!m_bIsConnected){
#ifdef ACT_TEST
		fprintf(stderr, "m_hConn=%d\n", m_hConn);
#endif
		return pQueue;
	}
	MQLONG iOptions;
	MQHOBJ hQueue;
	switch (iOpenFlag) {
		case MQOQF_GET:
			iOptions = MQOO_INPUT_AS_Q_DEF + MQOO_FAIL_IF_QUIESCING;
			break;
		case MQOQF_PUT:
			iOptions = MQOO_OUTPUT + MQOO_FAIL_IF_QUIESCING;
			break;
		case MQOQF_BROWSE:
			iOptions = MQOO_BROWSE;
			break;
		case MQOQF_GET_PUT:
			iOptions = MQOO_INPUT_AS_Q_DEF + MQOO_OUTPUT + MQOO_FAIL_IF_QUIESCING;
			break;
		default:
			iOptions = MQOO_INPUT_AS_Q_DEF + MQOO_OUTPUT + MQOO_FAIL_IF_QUIESCING;
	}
	strncpy(m_od.ObjectName, pQueueName, (size_t)MQ_Q_NAME_LENGTH);
	MQOPEN(m_hConn, &m_od, iOptions, &hQueue, &m_iComCode, &m_iReason);
#ifdef ACT_TEST
		fprintf(stderr, "m_hConn=%d, hQueue=%d, iComCode=%d, iReason=%d\n", m_hConn, hQueue, m_iComCode, m_iReason);
		fprintf(stderr,"iOptions=%d\n", iOptions);
#endif
	
	if (m_iComCode == MQCC_FAILED) 
		return pQueue;
	else 
		return pQueue = new IMQQueue(m_hConn, hQueue, pQueueName, iOpenFlag);
}

BOOL IMQManager::Commit()
{
	MQCMIT(m_hConn, &m_iComCode, &m_iReason);
    return m_iComCode != MQCC_OK ? FALSE : TRUE;
}	

BOOL IMQManager::Rollback()
{
	MQBACK(m_hConn, &m_iComCode, &m_iReason);
    return m_iComCode != MQCC_OK ? FALSE : TRUE;
}	
	
IMQManager::~IMQManager()
{
	MQDISC(&m_hConn, &m_iComCode, &m_iReason);
}
	
IMQQueue::IMQQueue(MQHCONN hConn, MQHOBJ& hObj, LPCSTR pQueueName, int iOpenFlag)
{
	m_hConn = hConn;
	m_hQueue = hObj;
	m_strQueueName = pQueueName;
	m_iOpenFlag = iOpenFlag;
	
	MQMD md = { MQMD_DEFAULT		};
	memcpy(&m_md, &md, sizeof(md));
	m_md.Version = MQOD_VERSION_2;

    MQPMO pmo = { MQPMO_DEFAULT		};
    memcpy(&m_pmo, &pmo, sizeof(pmo));
	m_pmo.Version = MQOD_VERSION_2;
	
    MQGMO gmo = { MQGMO_DEFAULT		};
    memcpy(&m_gmo, &gmo, sizeof(gmo));
	m_gmo.Version = MQOD_VERSION_2;
    
    m_iComCode = MQCC_OK;
    m_iReason = MQRC_NONE;
}

BOOL IMQQueue::Put(MQMsgInfo& msg)
{
	// 检查队列是否支持PUT
	if (m_iOpenFlag != MQOQF_PUT && m_iOpenFlag != MQOQF_GET_PUT)
		return FALSE;
	// 设置MQMD
	// 设置MsgId
	strncpy((char*)m_md.MsgId, msg.m_strMsgId, sizeof(m_md.MsgId));
    // 设置消息类型
    if (msg.m_iMsgType == MT_NORMAL)
    	m_md.MsgType = MQMT_DATAGRAM;
    else
    	m_md.MsgType = MQMT_REPORT;
	// 设置Feedback Code
	m_md.Feedback = msg.m_iFeedback;
	
    // 设置优先级
    m_md.Priority = msg.m_nPriority;
	
    m_md.Persistence  = MQPER_PERSISTENT;
    // 设置超时
    if (msg.m_nExpire != MQEI_UNLIMITED && !m_strReplyToQ.IsEmpty())
	    m_md.Expiry = msg.m_nExpire * 10;
	else
	    m_md.Expiry = MQEI_UNLIMITED;
	  
    // 设置分段信息
    if (msg.m_iSegmentFlag == SF_SEGMENT) {
    	m_md.MsgFlags = MQMF_SEGMENT;
	    m_pmo.Options = MQPMO_LOGICAL_ORDER | MQPMO_SYNCPOINT;
    } else if (msg.m_iSegmentFlag == SF_LASTSEGMENT) {
    	m_md.MsgFlags = MQMF_LAST_SEGMENT;
	    m_pmo.Options = MQPMO_LOGICAL_ORDER | MQPMO_SYNCPOINT;
	} else {
	    m_pmo.Options = MQPMO_SYNCPOINT;
    	m_md.MsgFlags = MQMF_NONE;
    }
	// 设置格式
   	strncpy(m_md.Format, msg.m_pFormat, 8);

	// 设置ReplyQueue
	strncpy(m_md.ReplyToQ, m_strReplyToQ, MQ_Q_NAME_LENGTH);
   	// 设置Report参数
   	if (!m_strReplyToQ.IsEmpty()) {
   	   	m_md.Report = MQRO_EXPIRATION_WITH_FULL_DATA | MQRO_EXCEPTION_WITH_FULL_DATA;
   	} else
   		m_md.Report = 0;   	
    
    MQPUT(m_hConn, m_hQueue, &m_md, &m_pmo, msg.m_nDataLen, msg.m_pBuffer,
    		&m_iComCode, &m_iReason);
    return m_iComCode != MQCC_OK ? FALSE : TRUE;
}

BOOL IMQQueue::PutFisc(LPCSTR pMsg, int iLen)
{

	char szId[17];
	char szMsgId[24];
	memset(szId, 0, sizeof(szId));
	memset(szMsgId, '0', sizeof(szMsgId));

	strcpy(szId, "00");
	CString sTime = GetTime("YYYYMMDDHHMMSS");
	memcpy(szId + 2, sTime, sTime.GetLength());
//	strcat(szId, "02");
//	strid[16]=0;
//	MessageBox(strid);
	
	strcpy(szMsgId, "JS");
	//szMsgId[24] = 0; //why 
	szMsgId[23] = 0; //is this rigth? jerry 2004.3.16
	
	// 检查队列是否支持PUT
	if (m_iOpenFlag != MQOQF_PUT && m_iOpenFlag != MQOQF_GET_PUT)
		return FALSE;
	// 设置MsgId
	memcpy(m_md.MsgId,  MQMI_NONE, sizeof(m_md.MsgId) );
	memcpy(m_md.MsgId, szMsgId, sizeof(m_md.MsgId) );
	m_md.Persistence = MQPER_PERSISTENT;
	memcpy(m_md.CorrelId, szId, sizeof(m_md.CorrelId) );
	MQPUT(m_hConn, m_hQueue, &m_md, &m_pmo, iLen, (char*)pMsg,
    		&m_iComCode, &m_iReason);
	return m_iComCode != MQCC_OK ? FALSE : TRUE;
}

BOOL IMQQueue::Get(MQMsgInfo& msg, int nWait, LPCSTR pMsgId)
{
	if (m_iOpenFlag == MQOQF_PUT)
		return FALSE;
	if (m_iOpenFlag == MQOQF_BROWSE)
    	m_gmo.Options = MQGMO_BROWSE_NEXT;
    else
    	m_gmo.Options = MQGMO_SYNCPOINT;
	if (nWait >0)
	{
	    m_gmo.Options |= MQGMO_WAIT;
		m_gmo.WaitInterval = nWait*1000;
	}	
	else 
	{
	    m_gmo.Options |= MQGMO_NO_WAIT;
	}
	if (pMsgId) {
		m_gmo.MatchOptions = MQMO_MATCH_MSG_ID;
		strncpy((char*)m_md.MsgId, pMsgId, strlen(pMsgId) > sizeof(m_md.MsgId) ? sizeof(m_md.MsgId) : strlen(pMsgId));
	} else {
		m_gmo.Options |= MQGMO_LOGICAL_ORDER | MQGMO_ALL_MSGS_AVAILABLE | MQGMO_ALL_SEGMENTS_AVAILABLE;
		m_gmo.MatchOptions = MQMO_NONE;
	}

	MQLONG nDataLen;
	m_iReason = 0;
    MQGET(m_hConn, m_hQueue, &m_md, &m_gmo, msg.m_nBufferLen, msg.m_pBuffer,
    		&nDataLen, &m_iComCode, &m_iReason);
    if (m_iComCode == MQCC_OK) {
	    msg.m_nDataLen = nDataLen;
    	msg.m_pBuffer[nDataLen] = 0;
    	if (m_md.MsgType != MQMT_REPORT)
    		msg.m_iMsgType = MT_NORMAL;
    	else {
    		msg.m_iMsgType = MT_REPORT;
    		if (m_md.Feedback >= MQFB_SYSTEM_FIRST && 
    			m_md.Feedback <= MQFB_SYSTEM_LAST) {
    			if (m_md.Feedback == MQFB_EXPIRATION)
    				msg.m_iFeedback = MFBC_EXPIRATION;
    			else 
    				msg.m_iFeedback = MFBC_MQ_ERR;
    		} else 
    			msg.m_iFeedback = m_md.Feedback;
    	}
    	msg.m_iBackoutCount = m_md.BackoutCount;
    	strncpy(msg.m_strMsgId.GetBufferSetLength(sizeof(m_md.MsgId)), (char*)m_md.MsgId, sizeof(m_md.MsgId));
    	msg.m_strMsgId.ReleaseBuffer();
    	msg.m_strMsgId.TrimRight();
    	strncpy(msg.m_pFormat, m_md.Format, 8);
    	if ((m_md.MsgFlags & MQMF_LAST_SEGMENT))
    		msg.m_iSegmentFlag = SF_LASTSEGMENT;
    	else if ((m_md.MsgFlags & MQMF_SEGMENT))
    		msg.m_iSegmentFlag = SF_SEGMENT;
    	else
    		msg.m_iSegmentFlag = SF_NONESEGMENT;
    }
    return m_iComCode != MQCC_OK ? FALSE : TRUE;
}	

IMQQueue::~IMQQueue()
{
	MQCLOSE(m_hConn, &m_hQueue, MQCO_NONE, &m_iComCode, &m_iReason);
}

		
	
