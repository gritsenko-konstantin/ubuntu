/********************************************************************
文件名：api.hpp
创建人：aps-zgh
日  期：2008-08-19
修改人：
日  期：
描  述：上海市支付结算综合业务系统前置机API接口头文件

版  本：
Copyright (c) 2008  SZFESC
********************************************************************/
#ifndef __H_API_HPP
#define __H_API_HPP

/*连接 */
void * MB_CONNECT(char * pQMRName, char *pRetCode, char *pRetInfo);

/*断开连接 */
void MB_DISCONNECT(void *pID);

/*发送 */
void MB_SEND(void *pID, char *pQLName,char *pInPutMsg, char *pInPath ,char *pRetCode, char *pRetInfo);

/*接收 */
void MB_RECEIVE(void *pID, char *pQLName, char *pOutPutMsg,char *pOutPath,int &nFileFlg, char *pRetCode, char *pRetInfo);


//接收确认
void MB_CONFIRM(void *pId, char *szProcFlag, char *szRtnCode, char *pRetInfo);

#endif //__H_API_HPP
