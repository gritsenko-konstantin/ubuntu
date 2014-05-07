/*************************************************************************
�ļ�����PoolAccess.cpp
��	�ܣ�ʵ�ֶ�һ����Դ�صķ��ʿ�(������û����Դ���ӽ��̲������е����)
�����ˣ�jerry_hu
��  �ڣ�2004-12-5
�޸��ˣ�
��  �ڣ�
��  ��������ԴΪn һ�����̿�����Щ��Դ�����������Դ�����ӽ��̷��ʡ�
����Ӧ�ã�TcpServ�����������ơ�

��֪ȱ�ݣ�
1����������̿��Ƶ���Դ�ز���ʹ�ø��� (ʹ��ʱ���ܵ����ӽ�����Ҫ�ȴ�)

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
			printf("Key=[%d]���ź���������,����Ҳʧ��\n",m_nKey);
			m_nStatus = -1;
		}
		else
		{
			printf("����Key=[%d]���ź����ɹ�\n",m_nKey);
		}
	}

	if (m_semID >0 && nDepth > 0) //�ɹ�ȡ�� ����ָ���˳�ʼֵ
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
	int nRet = semctl(m_semID,0,SETVAL,nDepth); //����
	if (nRet < 0)
	{
		printf("���ź���=[%d]����ֵʧ��\n",m_nKey);
		m_nStatus = -1;
	}
	else
	{
		printf("�����ź���=[%d]��ֵΪ[%d]\n",m_nKey,nDepth);
	}
#endif

	return m_nStatus;
}

//ʹ����Դ (�ӽ����� Ӧ��ʱ���ڸ������Ѿ��������Ƿ������ӽ������п϶��ܵõ���Դ)
int CPoolAccess::Employ()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]û����������\n",m_nKey);
		return -1;
	}
#ifndef WIN32
	struct sembuf sem_lock={0,-1,IPC_NOWAIT|SEM_UNDO}; //�����Ľṹ
	int nRet = semop(m_semID,&sem_lock,1);
	if (nRet < 0) //����Ӧ�ø���error��ֵ����
	{
		printf("������ͻ\n");
		return 1; //���Լ������Լ���
	}
	printf("�ɹ�����\n");
#endif
	
	return 0;
}

//�黹��Դ
int CPoolAccess::Fire()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]û����������\n",m_nKey);
		return -1;
	}
#ifndef WIN32
	struct sembuf sem_lock={0,1,IPC_NOWAIT|SEM_UNDO}; //�����ṹ
	int nRet = semop(m_semID,&sem_lock,1);
	if (nRet < 0) //����Ӧ�ø���error��ֵ����
	{
		printf("����[%d]ʧ��\n",m_nKey);
		return 1;
	}
#endif
	
	return 0;		
}


//�õ���ǰ����Դ��
int CPoolAccess::GetCurrentDepth()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("CPoolAccess Key = [%d]û����������\n",m_nKey);
		return -1;
	}
	
	int nRet = 0;
#ifndef WIN32
//	union semun semVal;
//	semVal.val = nDepth;
	
	nRet = semctl(m_semID,0,GETVAL,0); //ȡ�õ�ǰֵ
	if (nRet < 0)
	{
		printf("ȡ�ź���key=[%d]�ĵ�ǰֵʧ��\n",m_nKey);
	}
#endif

	return nRet;
}