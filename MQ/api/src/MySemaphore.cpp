/*************************************************************************
�ļ�����MySemaphore.cpp
��	�ܣ��ź����࣬ʵ�ּ���/����/���³�ʼ����������
�����ˣ�jerry_hu
��  �ڣ�2004-12-5
�޸��ˣ�
��  �ڣ�
��  ������ʼֵΪ1�ĵ����ź���������ʵ���ļ�
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
			printf("Key=[%d]���ź���������,����Ҳʧ��\n",m_nKey);
			m_nStatus = -1;
		}
		else
		{
			printf("����Key=[%d]���ź����ɹ�\n",m_nKey);
//			union semun semVal;
//			semVal.val = 1;
			int nRet = semctl(m_semID,0,SETVAL,1); //����һ�����̷���			
			if (nRet < 0)
			{
				printf("���ź���=[%d]����ֵʧ��\n",m_nKey);
				m_nStatus = -1;
			}
		}
	}
	else
	{
		printf("ȡ��Key=[%d]���ź����ɹ�\n",m_nKey);
	}
#endif

	return m_nStatus;
}


int MySemaphore::Reset()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]û����������\n",m_nKey);
		return -1;
	}	
#ifndef WIN32
//	union semun semVal;
//	semVal.val = 1;
	int nRet = semctl(m_semID,0,SETVAL,1); //����һ�����̷���			
	if (nRet < 0)
	{
		printf("���ź���=[%d]����ֵʧ��\n",m_nKey);
		//m_nStatus = -1;
		return -1;
	}
#endif
	printf("�ɹ�Reset");
	return 0;
}

int MySemaphore::Lock()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]û����������\n",m_nKey);
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

int MySemaphore::UnLock()
{
	if (m_nStatus != 0 || m_semID < 0)
	{
		printf("MySemaphore Key = [%d]û����������\n",m_nKey);
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
