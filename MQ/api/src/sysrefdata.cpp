//////////////////////////////////////////////////////////////////// 
//�ļ�����sysrefdata.cpp
//��  �ܣ������ڴ����ͷ�ļ�
//˵  ����
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
	// �����ڴ� ʹ����Դ��
	if (bWriteFlg)
	{
		m_iShmId = shmget(m_tKey, MAX_SHAREMEMOEY_BYTE, IPC_CREAT|0666); 
		if(m_respool.Init(m_tKey+1) < 0) //31
		{
			printf("��ʼ����Դ��ʧ��\n");
		}				
	}
	else 
		m_iShmId = shmget(m_tKey, MAX_SHAREMEMOEY_BYTE, IPC_CREAT); 
	
	if (m_iShmId < 0)
	{ 
		printf("���빲���ڴ����[%s]\n", strerror(errno));
	}
	else
	{
		m_pSysRefData = (SysRefData*)shmat(m_iShmId, NULL, 0);
		
		if (bInitSem) 
		{
			if (m_semLock.Init(m_tKey) < 0)
			{
				printf("��ʼ���ź���ʧ��\n");
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
	while(nRet > 0)  //��������ѭ��Ŷ
	{
		nCount++;
		if (nCount > 100) printf("���棺[%d]����û��������Դ!!!!!\n",nCount);
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
		printf("����:û����srd(TRUE,TRUE)�ķ�ʽ�õ������ڴ棬�ֳ��Լ��������ڴ�!!!!!\n");		
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