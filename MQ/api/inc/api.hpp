/********************************************************************
�ļ�����api.hpp
�����ˣ�aps-zgh
��  �ڣ�2008-08-19
�޸��ˣ�
��  �ڣ�
��  �����Ϻ���֧�������ۺ�ҵ��ϵͳǰ�û�API�ӿ�ͷ�ļ�

��  ����
Copyright (c) 2008  SZFESC
********************************************************************/
#ifndef __H_API_HPP
#define __H_API_HPP

/*���� */
void * MB_CONNECT(char * pQMRName, char *pRetCode, char *pRetInfo);

/*�Ͽ����� */
void MB_DISCONNECT(void *pID);

/*���� */
void MB_SEND(void *pID, char *pQLName,char *pInPutMsg, char *pInPath ,char *pRetCode, char *pRetInfo);

/*���� */
void MB_RECEIVE(void *pID, char *pQLName, char *pOutPutMsg,char *pOutPath,int &nFileFlg, char *pRetCode, char *pRetInfo);


//����ȷ��
void MB_CONFIRM(void *pId, char *szProcFlag, char *szRtnCode, char *pRetInfo);

#endif //__H_API_HPP
