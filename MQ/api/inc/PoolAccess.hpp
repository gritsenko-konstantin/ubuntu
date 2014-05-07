/*************************************************************************
�ļ�����PoolAccess.hpp
��	�ܣ�ʵ�ֶ�һ����Դ�صķ��ʿ�(������û����Դ���ӽ��̲������е����)
�����ˣ�jerry_hu
��  �ڣ�2004-12-5
�޸��ˣ�
��  �ڣ�
��  ��������ԴΪn һ�����̿�����Щ��Դ�����������Դ�����ӽ��̷��ʡ�
����Ӧ�ã�TcpServ�����������ơ�

��֪ȱ�ݣ�
1����������̿��Ƶ���Դ�ز���ʹ�ø���
************************************************************************/

#ifndef _H_POOLACCESS_CONTROL_H
#define _H_POOLACCESS_CONTROL_H

#ifndef WIN32
	#include <sys/shm.h>	/* include the proper headers */ 
	#include <sys/types.h>	/* basic system data types */
	#include <sys/ipc.h>	
	#include <sys/sem.h>	
	#include <unistd.h>
#endif

class CPoolAccess
{
public:
	CPoolAccess();
	~CPoolAccess(){};
	CPoolAccess(int nKey,int nDepth = 0);

	int Init(int m_nKey,int nDepth = 0);

	int GetCurrentDepth();   //�õ���ǰ��Դ�� ����0ʱ�����ӽ���

	int Employ();		//ʹ����Դ  �ӽ��̽���ʱ����
	int Fire();			//�黹��Դ  �ӽ����˳�ʱ����

	int Reset(int nDepth); //������Դ�����		

protected:
	int m_nKey;
	int m_nStatus;
	int m_semID;
};

#endif //_H_POOLACCESS_CONTROL_H
