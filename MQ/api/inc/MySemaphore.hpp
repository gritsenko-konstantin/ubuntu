/*************************************************************************
�ļ�����MySemaphore.hpp
��	�ܣ��ź����࣬ʵ�ּ���/����/���³�ʼ����������
�����ˣ�jerry_hu
��  �ڣ�2004-12-5
�޸��ˣ�
��  �ڣ�
��  ������ʼֵΪ1�ĵ����ź���������ʵ����ͷ�ļ�
************************************************************/
#ifndef _H_INCLUDE_MYSEMAPHORE_H
#define _H_INCLUDE_MYSEMAPHORE_H
#ifndef WIN32
	#include <sys/shm.h>	/* include the proper headers */ 
	#include <sys/types.h>	/* basic system data types */
	#include <sys/ipc.h>	
	#include <sys/sem.h>	
	#include <unistd.h>
#endif


class MySemaphore
{
public:
	MySemaphore(int nKey);
	MySemaphore();
	~MySemaphore(){};
	
	int Init(int nKey);
	int Lock();				//����
	int UnLock();			//����
	int Reset();			//��λ ��ֵΪ1
	
protected:
	int m_nKey;
	int m_nStatus;
	int m_semID;
};


#endif //_H_INCLUDE_MYSEMAPHORE_H
