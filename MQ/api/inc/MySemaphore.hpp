/*************************************************************************
文件名：MySemaphore.hpp
工	能：信号量类，实现加锁/解锁/重新初始化三个函数
创建人：jerry_hu
日  期：2004-12-5
修改人：
日  期：
描  述：初始值为1的单个信号量操作的实现类头文件
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
	int Lock();				//加锁
	int UnLock();			//解锁
	int Reset();			//复位 赋值为1
	
protected:
	int m_nKey;
	int m_nStatus;
	int m_semID;
};


#endif //_H_INCLUDE_MYSEMAPHORE_H
