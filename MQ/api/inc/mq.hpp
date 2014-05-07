#ifndef __CNAPSMQ___
#define __CNAPSMQ___

#include "cmqc.h"
#include "cstring.hpp"
#include "cfile.hpp"


// ������Ϣ��һЩ�궨��
// �ֶα�־
enum {SF_NONESEGMENT, 	// δ�ֶ�
	  SF_SEGMENT,  		// �����һ��
	  SF_LASTSEGMENT 	// ���һ��
}; 

// ��Ϣ����
enum {MT_NORMAL, 	// ������Ϣ
	  MT_REPORT		// ���˻ص���Ϣ
}; 

// ���ȼ�	  
enum {MPR_COMMON = 1, 	// ��ͨ
	  MPR_LOWER = 0,  	// �ͼ�
	  MPR_HIGHER = 2	// �߼�
};

// ��ʱ	  
#define MET_NONE  MQEI_UNLIMITED // �޳�ʱ����

// ��Ϣ����������
#define DEFAULT_MQMSG_LEN 4096
#define MAX_MQMSG_LEN 30720
// �˻���
enum {MFBC_EXPIRATION = MQFB_EXPIRATION, 	// ��ʱ
	  MFBC_MQ_ERR = MQFB_APPL_FIRST, 		// MQϵͳ��������
	  MFBC_INVALID_MSG, 			 		// ���Ĳ��Ϸ�
	  MFBC_ACCESS_DENY,						// ��Ȩ��
	  MFBC_WRONG_TIME,						// ϵͳ״̬����
	  MFBC_SYS_ERR,							// ϵͳ����
	  MFBC_NONE = MQFB_NONE 				// ��
};

// ��Ϣ
struct MQMsgInfo {
	MQMsgInfo(int nBuffLen = 0);
	~MQMsgInfo();
	
	CString m_strMsgId;			// ��ϢID
	
	int m_iMsgType;				// ��Ϣ����
	int m_iFeedback;			// �˻��루��Ϣ����ΪMT_REPORTʱ��Ч��
	int m_nExpire;				// ʱ�ޣ��������ƣ�
	int m_nPriority;			// ���ȼ�
	int m_iSegmentFlag;			// �ֶα�־
	int m_iBackoutCount;		// ��Ϣ�����ع��Ĵ���
	char m_pFormat[8];			// ��Ϣ��ʽ
	
	char* m_pBuffer;			// ������
	int m_nBufferLen;			// ����������
	int m_nDataLen;				// ���ݳ���
	BOOL m_bAutoDelete;			// �⹹ʱ�Ƿ�Ҫɾ������
};

inline
MQMsgInfo::MQMsgInfo(int nBuffLen)
{
	if (nBuffLen > MAX_MQMSG_LEN || nBuffLen < 0)
		nBuffLen = DEFAULT_MQMSG_LEN;
	m_nPriority = MPR_COMMON;
	m_nExpire = MET_NONE;
	m_iMsgType = MT_NORMAL;
	m_iSegmentFlag = SF_NONESEGMENT;
	m_iFeedback = MFBC_NONE;
	//memset(m_pFormat, ' ', 8); //���ų���������������޸�
	strncpy(m_pFormat,MQFMT_STRING,8); //�ĳ�MQFMT_STRING����
	if (nBuffLen) {
		m_pBuffer = new char[nBuffLen + 1];
		m_nBufferLen = nBuffLen;
		m_nDataLen = 0;
		m_bAutoDelete = TRUE;
	} else {
		m_pBuffer = 0;
		m_nDataLen = m_nBufferLen = 0;
		m_bAutoDelete = FALSE;
	}
}

inline
MQMsgInfo::~MQMsgInfo()
{
	if (m_pBuffer && m_bAutoDelete) 
		delete[] m_pBuffer; 
}


// MQ������
enum {MQOQF_GET, MQOQF_PUT, MQOQF_GET_PUT, MQOQF_BROWSE};
class MQQueue;
class MQManager {
public:
	MQManager(LPCSTR pMgrName = 0) { if (pMgrName) m_strMgrName = pMgrName; }
	
// ��ָ������
	// ��һ������
	virtual MQQueue* OpenQueue(LPCSTR pQueueName, int iOpenFlag) = 0;
	// ��һ�����
	virtual MQQueue* OpenQueue(NameList& queueNames) = 0;
	virtual BOOL Commit() = 0;
	virtual BOOL Rollback() = 0;
protected:
	CString m_strMgrName;
};

// ����
class MQQueue {
public:
friend class MQManager;
	virtual BOOL Put(MQMsgInfo& msg) = 0;
	virtual BOOL Get(MQMsgInfo& msg, BOOL bWait = FALSE, LPCSTR pMsgId = 0) = 0;
	
protected:
	MQQueue() {}
};

// IBM MQ������
class IMQQueue;
class IMQManager : public MQManager{
public:
	~IMQManager();
	IMQManager(LPCSTR pMgrName);
	IMQManager();
	BOOL Connect(LPCSTR pMgrName);
	void Disconnect();

	MQQueue* OpenQueue(LPCSTR pQueueName, int iOpenFlag);
	MQQueue* OpenQueue(NameList& queueNames);
	BOOL Commit();
	BOOL Rollback();
	
	BOOL IsConnected() { return m_bIsConnected; }
	MQLONG GetReason() { return m_iReason; }
private:
	BOOL m_bIsConnected;
	MQHCONN m_hConn;
	MQOD m_od;
	MQLONG m_iComCode;
	MQLONG m_iReason;
};

// IBM MQ
class IMQQueue : public MQQueue {
public:
friend class IMQManager;
	~IMQQueue();

	BOOL Put(MQMsgInfo& msg);
	BOOL PutFisc(LPCSTR pMsg, int iLen);

	BOOL Get(MQMsgInfo& msg, int nWait = 0, LPCSTR pMsgId = 0);
	
	MQLONG GetReason() { return m_iReason; }
	void SetReplyToQ(LPCSTR pName);
	CString GetQueueName() { return m_strQueueName; }
	int GetOpenFlag() { return m_iOpenFlag; }
protected:
	IMQQueue(MQHCONN hConn, MQHOBJ& hObj, LPCSTR pQueueName, int iOpenFlag);
	
	BOOL m_bNeedReply;
	CString	m_strQueueName;
	CString m_strReplyToQ;
	int m_iOpenFlag;
	MQMD m_md;
	MQPMO m_pmo;
	MQGMO m_gmo;
	MQHCONN m_hConn;
	MQHOBJ m_hQueue;
	MQLONG m_iComCode;
	MQLONG m_iReason;
};

inline
void IMQQueue::SetReplyToQ(LPCSTR pName) 
{ 
	if (pName) 
		m_strReplyToQ = pName; 
	else 
		m_strReplyToQ.Empty(); 
} 

#endif

