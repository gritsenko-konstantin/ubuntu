/*************************************************************************
文件名：PoolAccess.hpp
功	能：实现对一个资源池的访问控(适用于没有资源则子进程不能运行的情况)
创建人：jerry_hu
日  期：2004-12-5
修改人：
日  期：
描  述：总资源为n 一个进程控制这些资源，如果还有资源运行子进程访问。
典型应用：TcpServ的连接数控制。

已知缺陷：
1、多个父进程控制的资源池不能使用该类
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

	int GetCurrentDepth();   //得到当前资源数 大于0时启动子进程

	int Employ();		//使用资源  子进程进入时调用
	int Fire();			//归还资源  子进程退出时调用

	int Reset(int nDepth); //重设资源池深度		

protected:
	int m_nKey;
	int m_nStatus;
	int m_semID;
};

#endif //_H_POOLACCESS_CONTROL_H
