/*************************************************************************
文件名：PoolAccess.cpp
功	能：实现对一个资源池的访问控(适用于没有资源则子进程不能运行的情况)
创建人：jerry_hu
日  期：2004-12-5
修改人：
日  期：
描  述：总资源为n 一个进程控制这些资源，如果还有资源运行子进程访问。
典型应用：TcpServ的连接数控制。

已知缺陷：
1、多个父进程控制的资源池不能使用该类 (使用时可能导致子进程需要等待)

************************************************************************/
#include "PoolAccess.hpp"
#include <signal.h>
#include "stdlib.h"
#include "stdio.h"

CPoolAccess::CPoolAccess()
{
	m_nStatus = -1;
	m_nKey = -1;
	m_semID = -1;
}

CPoolAccess::CPoolAccess(int nKey,int nDepth)
{
	Init(nKey,nDepth);
}

int CPoolAccess::Init(int nKey,int nDepth /* = 0 */)
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
		}
	}

	if (m_semID >0 && nDepth > 0) //成功取得 并且指定了初始值
	{
		Reset(nDepth);
	}
#endif
	
	return m_nStatus;	
}

int CPoolAccess::Reset(int nDepth)
{
	if (nDepth <=0 || m_semID < 0 || m_nStatus < 0)
	{
		return -1;
	}

#ifndef WIN32
//	union semun semVal;
//	semVal.val = nDepth;
	int nRet = semctl(m_semID,0,SETVAL,nDepth); //设置
	if (nRet < 0)
	{
		printf("给信号量=[%d]赋初值失败\n",m_nKey);
		m_nStatus = -1;
	}
	else
	{
		printf("设置信号量=[%d]的值为[%d]\n",m_nKey,nDepth);
	}
#endif

	return m_nStatus;
}

//使用资源 (子进程用 应该时由于父进程已经决定过是否启动子进程所有肯定能得到资源)
int CPoolAccess::Employ()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]没有正常构造\n",m_nKey);
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

//归还资源
int CPoolAccess::Fire()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]没有正常构造\n",m_nKey);
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


//得到当前得资源数
int CPoolAccess::GetCurrentDepth()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]没有正常构造\n",m_nKey);
		return -1;
	}
	
	int nRet = 0;
#ifndef WIN32
//	union semun semVal;
//	semVal.val = nDepth;
	
	nRet = semctl(m_semID,0,GETVAL,0); //取得当前值
	if (nRet < 0)
	{
		printf("取信号量key=[%d]的当前值失败\n",m_nKey);
	}
#endif

	return nRet;
}