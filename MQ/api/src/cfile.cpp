#include "cfile.hpp"
#include <time.h>
#define MAX_LINE_SIZE 4095

CString GetTime(LPCSTR pFormat)
{
	CString sRet;
	char pBuff[20];
	tm* pCurTime;
	time_t t;
	time(&t);
	pCurTime = localtime(&t); 
	CString sFormat = pFormat;
	sFormat.MakeUpper();
	BOOL bDate = FALSE;
	int iPos, iPosStart = 0;
	iPos = sFormat.Find("YYYY");
	if (iPos >= 0) {
		sRet += sFormat.Left(iPos);
		sprintf(pBuff, "%4d", pCurTime->tm_year + 1900);
		sRet += pBuff;
		bDate = TRUE;
		iPosStart = iPos + 4;
	} else {
		iPos = sFormat.Find("YY");
		if (iPos >= 0) {
			sRet += sFormat.Left(iPos);
			sprintf(pBuff, "%02d", pCurTime->tm_year - 100);
			sRet += pBuff;
			bDate = TRUE;
			iPosStart = iPos + 2;
		}
	}
	if (bDate)  {
		iPos = sFormat.Find("MM", iPosStart);
		if (iPos > 0) {
			sRet += sFormat.Mid(iPosStart,  iPos - iPosStart);
			sprintf(pBuff, "%02d", pCurTime->tm_mon + 1);
			sRet += pBuff;
			iPosStart = iPos + 2;
		}
		iPos = sFormat.Find("DD", iPosStart);
		if (iPos > 0) {
			sRet += sFormat.Mid(iPosStart,  iPos - iPosStart);
			sprintf(pBuff, "%02d", pCurTime->tm_mday);
			sRet += pBuff;
			iPosStart = iPos + 2;
		}
	}
	
	iPos = sFormat.Find("HH", iPosStart);
	if (iPos >= 0) {
		sRet += sFormat.Mid(iPosStart,  iPos - iPosStart);
		sprintf(pBuff, "%02d", pCurTime->tm_hour);
		sRet += pBuff;
		iPosStart = iPos + 2;
	}
	
	iPos = sFormat.Find("MM", iPosStart);
	if (iPos >= 0) {
		sRet += sFormat.Mid(iPosStart,  iPos - iPosStart);
		sprintf(pBuff, "%02d", pCurTime->tm_min);
		sRet += pBuff;
		iPosStart = iPos + 2;
	}

	iPos = sFormat.Find("SS", iPosStart);
	if (iPos >= 0) {
		sRet += sFormat.Mid(iPosStart,  iPos - iPosStart);
		sprintf(pBuff, "%02d", pCurTime->tm_sec);
		sRet += pBuff;
		iPosStart = iPos + 2;
		sRet += sFormat.Mid(iPosStart);
	}
	return sRet;
}


CString GetFileName(LPCSTR pFullPath)
{
	int ip;
	CString sTemp;
	sTemp = pFullPath;
	ip = sTemp.Find('\\'); 
	if (ip < 0)
		ip = sTemp.Find('/'); 
	while (ip >=0) {
		sTemp = sTemp.Mid(ip + 1);
		ip = sTemp.Find('\\'); 
		if (ip < 0)
			ip = sTemp.Find('/'); 
	}
	return sTemp;
}

CString GetFileDir(const CString& sFullPath)
{
	const char* ptr = sFullPath;
	int i = sFullPath.GetLength(); 
	ptr += i - 1;
	while(i) {
		if(*ptr == '\\' || *ptr == '/')
			return sFullPath.Left(i - 1);
		ptr--;
		i--;
	}
	return "";
}

BOOL CreateDir(const CString& sPath)
{
	int iPos = 1;
	int iPos2 = 1;
	while (iPos2 > 0 ) {
		CString s;
		iPos2 = sPath.Find('\\', iPos);
		if (iPos2 < 0)
			iPos2 = sPath.Find('/', iPos);
		if (iPos2 > 0)
			s = sPath.Left(iPos2);
		else
			s = sPath;
		if (access(s, 0) == -1) 
#ifdef WIN32		
			if (mkdir(s))
#else			
			if (mkdir(s, 0777))
#endif			
				return FALSE;
		iPos = iPos2 + 1;
	}
	return TRUE;
}

BOOL IsFileExist(LPCSTR pPath)
{
	return access(pPath, 0) == 0 ? TRUE : FALSE;
}

CFile::CFile()
{
	m_hFile = -1;
}

CFile::CFile(LPCSTR pPath, int nOpenFlags, int nMode)
{
	m_hFile = -1;
	Open(pPath, nOpenFlags, nMode);
}

CFile::~CFile()
{
	if (m_hFile != -1)
		Close();
}

BOOL CFile::Open(LPCSTR pPath, int nOpenFlags, int nMode)
{
	if (m_hFile != -1)
		Close();
	CString sDir = ::GetFileDir(pPath);
	if (!sDir.IsEmpty()) {
		if (!CreateDir(sDir))
			return FALSE;
	}
	m_hFile = open(pPath, nOpenFlags, nMode);
	if (m_hFile != -1) {
		m_strFileName = pPath;
		return TRUE;
	}
	return FALSE;
}

BOOL CFile::Read(void* pBuffer, int nCount)
{
	if (m_hFile == -1)
		return FALSE;
	return  read(m_hFile, pBuffer, nCount) < 0 ? FALSE : TRUE;
}

BOOL CFile::ReadN(void* pBuffer, int& nCount)
{
	if (m_hFile == -1)
		return FALSE;
    nCount = read(m_hFile, pBuffer, nCount);  
    return nCount < 0 ? FALSE : TRUE;
}

BOOL CFile::Write(const void* pBuffer, int nCount)
{
	if (m_hFile == -1)
		return FALSE;
	return  write(m_hFile, pBuffer, nCount) < 0 ? FALSE : TRUE;
}

int CFile::SeekToEnd()
{
	return lseek(m_hFile, 0, SEEK_END);
}

void CFile::SeekToBegin()
{
	lseek(m_hFile, 0, SEEK_SET);
}

int CFile::Seek(int off, int nFrom)
{
	return lseek(m_hFile, off, nFrom);
}

int CFile::GetLength() const
{
	int iPos = lseek(m_hFile, 0, SEEK_CUR);
	int iLen = lseek(m_hFile, 0, SEEK_END);
	lseek(m_hFile, iPos, SEEK_SET);
	return iLen;
}

void CFile::Close()
{
	if (m_hFile != -1)
		close(m_hFile);
	m_hFile	= -1;
	m_strFileName.Empty();
}

//BOOL CFile::Flush()
//{
//    if (m_hFile == -1)
//        return FALSE;
//    return 0 == commit(m_hFile)?TRUE:FALSE;
//}

CString CFile::GetFileName() const
{
	return ::GetFileName(m_strFileName);
}

CString CFile::GetFileDir() const
{
	return ::GetFileDir(m_strFileName);
}

CString CFile::GetFilePath() const
{
	return m_strFileName;
}

LogFile::LogFile(LPCSTR pName, int nMaxLineLen) : CFile()
{
	m_nMaxLineLen = nMaxLineLen;
	int nMode;
#ifdef WIN32		
	nMode = _S_IREAD | _S_IWRITE;
#else
	nMode = 0666;
#endif
	Open(pName, O_RDWR | O_CREAT | O_APPEND, nMode);
}

LogFile::LogFile() : CFile()
{
	m_nMaxLineLen = 10000;
}

BOOL LogFile::Open(LPCSTR pPath, int , int )
{
	int nMode;
#ifdef WIN32		
	nMode = _S_IREAD | _S_IWRITE;
#else
	nMode = 0666;
#endif
	return CFile::Open(pPath, O_RDWR | O_CREAT | O_APPEND, nMode);
}

BOOL LogFile::WriteLog(LPCSTR pText)
{
	CString sText = GetTime("[YYYY-MM-DD HH:MM:SS] ");
	sText += pText;
	sText += "\n";
	if (!Write(sText, sText.GetLength()))
		return FALSE;
	return TRUE;
}	

ConfigureFile::ConfigureFile()
{
	m_pFile = 0;
}

ConfigureFile::~ConfigureFile()
{
	if (m_pFile)
		fclose(m_pFile);
}

ConfigureFile::ConfigureFile(LPCSTR pFileName)
{
	Open(pFileName);
}

BOOL ConfigureFile::Open(LPCSTR pFileName)
{
	m_pFile = fopen(pFileName, "r");
	if (!m_pFile)
		return FALSE;
	m_strFileName = pFileName;
	return TRUE;
}

CString ConfigureFile::GetConfigure(LPCSTR pItemName, LPCSTR pDefValue)
{
	CString sRet;
	if (pDefValue)
		sRet = pDefValue;
	if (!m_pFile)
		return sRet;
	SafeBuffer buff(MAX_LINE_SIZE + 1);
	fseek(m_pFile, 0, SEEK_SET);
	for(;;) {
		if (!fgets(buff, MAX_LINE_SIZE, m_pFile))
			return sRet;
		if (!strstr(buff, pItemName))
			continue;
		CString sLine(buff);
		int iPos = sLine.Find("=");
		if (iPos < 0)
			continue;
		CString sName = sLine.Left(iPos);
		sName.Trim();
		if (sName != pItemName)
			continue;
		sRet = sLine.Mid(iPos + 1);
		iPos = sRet.Find("\r\n");
		if (iPos < 0)
			iPos = sRet.Find("\n");
		if (iPos >= 0)
			sRet.Left(iPos);
		sRet.Trim();
		return sRet;
	}
}


