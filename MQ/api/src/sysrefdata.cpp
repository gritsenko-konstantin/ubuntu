//////////////////////////////////////////////////////////////////// 
//文件名：sysrefdata.cpp
//功  能：共享内存操作头文件
//说  明：
////////////////////////////////////////////////////////////////////

#include "sysrefdata.hpp"
#include <errno.h>
#include <stdio.h>

extern int errno;

SysRefDataSvr::SysRefDataSvr(bool bWriteFlg, BOOL bInitSem)
{
	m_tKey = 7;
	m_pSysRefData = 0;
	m_bLocked = FALSE;
	// 共享内存 使用资源池
	if (bWriteFlg)
	{
		m_iShmId = shmget(m_tKey, MAX_SHAREMEMOEY_BYTE, IPC_CREAT|0666); 
		if(m_respool.Init(m_tKey+1) < 0) //31
		{
			printf("初始化资源池失败\n");
		}				
	}
	else 
		m_iShmId = shmget(m_tKey, MAX_SHAREMEMOEY_BYTE, IPC_CREAT); 
	
	if (m_iShmId < 0)
	{ 
		printf("申请共享内存出错[%s]\n", strerror(errno));
	}
	else
	{
		m_pSysRefData = (SysRefData*)shmat(m_iShmId, NULL, 0);
		
		if (bInitSem) 
		{
			if (m_semLock.Init(m_tKey) < 0)
			{
				printf("初始化信号锁失败\n");
			}
		}
	}
}

SysRefDataSvr::~SysRefDataSvr()
{
	if (m_bLocked)
		UnlockSysRefData();
	if (m_pSysRefData)
		shmdt(m_pSysRefData);

}

SysRefData* SysRefDataSvr::LockSysRefData()
{
	SysRefData* ptr = NULL;
	int nRet = m_semLock.Lock();
	int nCount = 0;
	while(nRet > 0)  //可能有死循环哦
	{
		nCount++;
		if (nCount > 100) printf("警告：[%d]秒内没有锁定资源!!!!!\n",nCount);
		sleep(1);
		nRet = m_semLock.Lock();
	}
	if (0 == nRet)
	{
		m_bLocked = TRUE;
		ptr = m_pSysRefData;
	}
	else
	{
		printf("警告:没有用srd(TRUE,TRUE)的方式得到共享内存，又尝试加锁共享内存!!!!!\n");		
	}		
	return ptr;
}

BOOL SysRefDataSvr::UnlockSysRefData()
{
	if (0 != m_semLock.UnLock())
	{
		return FALSE;
	}
	m_bLocked = FALSE;
	return TRUE;
}

int SysRefDataSvr::IncOnlyIndex()
{
    int nIndex = 0;
   	LockSysRefData();
    m_pSysRefData->m_nMaxTranCount++;
    if (m_pSysRefData->m_nMaxTranCount > 9999)
    {
        m_pSysRefData->m_nMaxTranCount = 0;
    }
    nIndex = m_pSysRefData->m_nMaxTranCount;
    UnlockSysRefData();
    return nIndex;
}