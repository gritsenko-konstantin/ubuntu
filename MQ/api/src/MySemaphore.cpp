/*************************************************************************
文件名：MySemaphore.cpp
工	能：信号量类，实现加锁/解锁/重新初始化三个函数
创建人：jerry_hu
日  期：2004-12-5
修改人：
日  期：
描  述：初始值为1的单个信号量操作的实现文件
************************************************************************/
#include "MySemaphore.hpp"
#include "stdlib.h"
#include "stdio.h"

MySemaphore::MySemaphore()
{
	m_nStatus = -1;
	m_nKey = -1;
	m_semID = -1;
}

MySemaphore::MySemaphore(int nKey)
{
	Init(nKey);
}

int MySemaphore::Init(int nKey)
{
	m_nStatus = 0;
	m_nKey = nKey;
	m_semID = -1;
	
#ifndef WIN32
	m_semID = semget(m_nKey,0,0);
	if (m_semID < 0)
	{
		m_semID = semget(m_nKey,1,IPC_CREAT|0666);
		if (m_semID < 0)
		{
			printf("Key=[%d]的信号量不存在,创建也失败\n",m_nKey);
			m_nStatus = -1;
		}
		else
		{
			printf("创建Key=[%d]的信号量成功\n",m_nKey);
//			union semun semVal;
//			semVal.val = 1;
			int nRet = semctl(m_semID,0,SETVAL,1); //运行一个进程访问			
			if (nRet < 0)
			{
				printf("给信号量=[%d]赋初值失败\n",m_nKey);
				m_nStatus = -1;
			}
		}
	}
	else
	{
		printf("取得Key=[%d]的信号量成功\n",m_nKey);
	}
#endif

	return m_nStatus;
}


int MySemaphore::Reset()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]没有正常构造\n",m_nKey);
		return -1;
	}	
#ifndef WIN32
//	union semun semVal;
//	semVal.val = 1;
	int nRet = semctl(m_semID,0,SETVAL,1); //运行一个进程访问			
	if (nRet < 0)
	{
		printf("给信号量=[%d]赋初值失败\n",m_nKey);
		//m_nStatus = -1;
		return -1;
	}
#endif
	printf("成功Reset");
	return 0;
}

int MySemaphore::Lock()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]没有正常构造\n",m_nKey);
		return -1;
	}
#ifndef WIN32
	struct sembuf sem_lock={0,-1,IPC_NOWAIT|SEM_UNDO}; //加锁的结构
	int nRet = semop(m_semID,&sem_lock,1);
	if (nRet < 0) //这里应该根据error的值返回
	{
		printf("加锁冲突\n");
		return 1; //可以继续尝试加锁
	}
	printf("成功加锁\n");
#endif

	return 0;
}

int MySemaphore::UnLock()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]没有正常构造\n",m_nKey);
		return -1;
	}
#ifndef WIN32
	struct sembuf sem_lock={0,1,IPC_NOWAIT|SEM_UNDO}; //解锁结构
	int nRet = semop(m_semID,&sem_lock,1);
	if (nRet < 0) //这里应该根据error的值返回
	{
		printf("解锁[%d]失败\n",m_nKey);
		return 1;
	}
#endif
	
	return 0;	
}
