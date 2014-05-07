//////////////////////////////////////////////////////////////////// 
//文件名：sysrefdata.cpp
//功  能：共享内存操作
//说  明：
////////////////////////////////////////////////////////////////////

#ifndef __SYSREFDATA__
#define __SYSREFDATA__

#include "cstring.hpp"
#include "cfile.hpp"
#include <sys/shm.h>	/* include the proper headers */ 
#include <sys/types.h>	/* basic system data types */
#include <sys/ipc.h>	
#include <sys/sem.h>
#include "MySemaphore.hpp"	
#include "PoolAccess.hpp"

#define MAX_SHAREMEMOEY_BYTE	5*1024	//共享内存长度

struct SysRefData
{
	int	 m_nMaxTranCount;
};

class SysRefDataSvr 
{
public:	

	SysRefDataSvr(bool bWriteFlg=FALSE , BOOL bInitSem = FALSE);
	~SysRefDataSvr();

	SysRefData* LockSysRefData();
	BOOL UnlockSysRefData();
	
	int IncOnlyIndex();


protected:
	SysRefData* m_pSysRefData;
	BOOL m_bLocked;
	int m_iErrCode;
	key_t m_tKey;
	int m_iSemId;
	int m_iShmId;
	//struct sembuf* SetSemStruct(int iNum, int iOper = 0, int iFlag = 0);

	MySemaphore m_semLock; //锁
	CPoolAccess m_respool; //资源池

};

#endif

