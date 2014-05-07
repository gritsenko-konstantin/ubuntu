#ifndef __CFILE__
#define __CFILE__

#include "cstring.hpp"
#include <stdio.h>

// 创建文件及目录
#ifdef WIN32
#include <direct.h>
#include <io.h>
#include <stdio.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <unistd.h>
#endif

class CFile {
public:
	CFile();
	CFile(LPCSTR pPath, int nOpenFlags, int nMode = 0666);
	~CFile();

	virtual BOOL Open(LPCSTR pPath, int nOpenFlags, int nMode = 0666);

	BOOL Read(void* lpBuf, int nCount);
	BOOL Write(const void* lpBuf, int nCount);

    BOOL ReadN(void* lpBuf,int& nCount);

//    BOOL Flush();

	CString GetFileName() const;  // 返回不含路径的文件名
	CString GetFilePath() const;  // 返回文件全路径名
	CString GetFileDir() const;   // 返回文件所在目录名
	
	int SeekToEnd();
	void SeekToBegin();
	int Seek(int off, int nFrom);
	int GetLength() const;
	
	BOOL Remove(LPCSTR pFileName) { return  unlink(pFileName) < 0 ? FALSE : TRUE; }
	BOOL IsOpened() { return m_hFile == -1 ? FALSE : TRUE; }
	void Close();
protected:
	int m_hFile;	
	CString m_strFileName;
	
};

class LogFile : public CFile {
public:
	LogFile(LPCSTR pName, int nMaxLineLen = 0);
	LogFile();
	BOOL Open(LPCSTR pName, int nOpenFlags = 0, int nMode = 0);
	void SetMaxLineLen(int nLen) { if(nLen > 0) m_nMaxLineLen = nLen; }
	BOOL WriteLog(LPCSTR pText);
protected:
	int m_nMaxLineLen;
};

class ConfigureFile {
public:
	ConfigureFile();
	ConfigureFile(LPCSTR pName);
	~ConfigureFile();
	BOOL Open(LPCSTR pName);
	BOOL IsOpened() { return !m_pFile ? FALSE : TRUE; }
	
	CString GetConfigure(LPCSTR pItemName, LPCSTR pDefValue = 0);
protected:
	FILE* m_pFile;
	CString m_strFileName;
};
	
BOOL IsFileExist(LPCSTR pPath);
CString GetTime(LPCSTR pFormat);
CString GetFileName(LPCSTR pFullPath);
CString GetFileDir(const CString& sFullPath);
BOOL CreateDir(const CString& sPath);

#endif
	
