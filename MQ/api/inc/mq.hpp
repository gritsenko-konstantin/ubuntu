#ifndef __CNAPSMQ___
#define __CNAPSMQ___

#include "cmqc.h"
#include "cstring.hpp"
#include "cfile.hpp"


// 关于消息的一些宏定义
// 分段标志
enum {SF_NONESEGMENT, 	// 未分段
	  SF_SEGMENT,  		// 非最后一段
	  SF_LASTSEGMENT 	// 最后一段
}; 

// 消息类型
enum {MT_NORMAL, 	// 正常消息
	  MT_REPORT		// 被退回的消息
}; 

// 优先级	  
enum {MPR_COMMON = 1, 	// 普通
	  MPR_LOWER = 0,  	// 低级
	  MPR_HIGHER = 2	// 高级
};

// 超时	  
#define MET_NONE  MQEI_UNLIMITED // 无超时限制

// 消息缓存区长度
#define DEFAULT_MQMSG_LEN 4096
#define MAX_MQMSG_LEN 30720
// 退回码
enum {MFBC_EXPIRATION = MQFB_EXPIRATION, 	// 超时
	  MFBC_MQ_ERR = MQFB_APPL_FIRST, 		// MQ系统其它报告
	  MFBC_INVALID_MSG, 			 		// 报文不合法
	  MFBC_ACCESS_DENY,						// 无权限
	  MFBC_WRONG_TIME,						// 系统状态不对
	  MFBC_SYS_ERR,							// 系统故障
	  MFBC_NONE = MQFB_NONE 				// 无
};

// 消息
struct MQMsgInfo {
	MQMsgInfo(int nBuffLen = 0);
	~MQMsgInfo();
	
	CString m_strMsgId;			// 消息ID
	
	int m_iMsgType;				// 消息类型
	int m_iFeedback;			// 退回码（消息类型为MT_REPORT时有效）
	int m_nExpire;				// 时限（以秒数计）
	int m_nPriority;			// 优先级
	int m_iSegmentFlag;			// 分段标志
	int m_iBackoutCount;		// 消息曾被回滚的次数
	char m_pFormat[8];			// 消息格式
	
	char* m_pBuffer;			// 缓存区
	int m_nBufferLen;			// 缓存区长度
	int m_nDataLen;				// 数据长度
	BOOL m_bAutoDelete;			// 解构时是否要删除缓存
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
	//memset(m_pFormat, ' ', 8); //电信出现问题测试所以修改
	strncpy(m_pFormat,MQFMT_STRING,8); //改成MQFMT_STRING试试
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


// MQ管理器
enum {MQOQF_GET, MQOQF_PUT, MQOQF_GET_PUT, MQOQF_BROWSE};
class MQQueue;
class MQManager {
public:
	MQManager(LPCSTR pMgrName = 0) { if (pMgrName) m_strMgrName = pMgrName; }
	
// 打开指定队列
	// 打开一个队列
	virtual MQQueue* OpenQueue(LPCSTR pQueueName, int iOpenFlag) = 0;
	// 打开一组队列
	virtual MQQueue* OpenQueue(NameList& queueNames) = 0;
	virtual BOOL Commit() = 0;
	virtual BOOL Rollback() = 0;
protected:
	CString m_strMgrName;
};

// 队列
class MQQueue {
public:
friend class MQManager;
	virtual BOOL Put(MQMsgInfo& msg) = 0;
	virtual BOOL Get(MQMsgInfo& msg, BOOL bWait = FALSE, LPCSTR pMsgId = 0) = 0;
	
protected:
	MQQueue() {}
};

// IBM MQ管理器
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

