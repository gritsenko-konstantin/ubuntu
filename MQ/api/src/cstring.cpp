#include "cstring.hpp"
#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef BYTE
#define BYTE unsigned char
#endif

int _afxInitData[] = { -1, 0, 0, 0 };
CStringData* _afxDataNil = (CStringData*)&_afxInitData;

inline 
int max(int i1, int i2)
{
	return i1 > i2 ? i1 : i2;
}

inline 
int min(int i1, int i2)
{
	return i1 < i2 ? i1 : i2;
}
	
void UPPER(char* s)
{
	if (s == NULL)
		return;
	while(*s != 0) {
		if (*s >= 'a' && *s <='z')
			*s = *s + 'A' - 'a';
		s++;
	}
}

void LOWER(char* s)
{
	if (s == NULL)
		return;
	while(*s != 0) {
		if (*s >= 'A' && *s <='Z')
			*s = *s + 'a' - 'A';
		s++;
	}
}	

int NameList::GetItemCount()
{
	int iCnt = 0;
	int iPos = m_strList.Find(m_strSeparator);
	while (iPos >= 0) {
		iCnt++;
		iPos+= strlen(m_strSeparator);
		iPos = m_strList.Find(m_strSeparator, iPos);
	}
	return iCnt;
}

CString itoa(int ino)
{
    CString strT;
    strT.Format("%d",ino);
    return strT;
}


CString::CString()
{
	Init();
}

CString::CString(const CString& stringSrc)
{
	if (stringSrc.GetData()->nRefs >= 0 && stringSrc.GetData() != _afxDataNil) {
		m_pchData = stringSrc.m_pchData;
		GetData()->nRefs++;
	}
	else {
		Init();
		*this = stringSrc.m_pchData;
	}
}
void CString::Init()
{
	m_pchData = _afxDataNil->data(); 
}

void CString::AllocBuffer(int nLen)
// always allocate one extra character for '\0' termination
// assumes [optimistically] that data length will equal allocation length
{
	if (nLen == 0)
		Init();
	else {
		CStringData* pData;
		pData = (CStringData*)	new BYTE[sizeof(CStringData) + (nLen+1)];
		pData->nAllocLength = nLen;
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		m_pchData = pData->data();
	}
}

void CString::FreeData(CStringData* pData)
{
	delete[] (BYTE*)pData;
}

void CString::Release()
{
	if (GetData() != _afxDataNil) {
		if (--GetData()->nRefs <= 0)
			FreeData(GetData());
		Init();
	}
}

void CString::Release(CStringData* pData)
{
	if (pData != _afxDataNil) {
		if (pData->nRefs == 0)
			return;
		if (--pData->nRefs <= 0)
			FreeData(pData);
	}
}


void CString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = _afxDataNil->data();
}

void CString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1) {  // It's the time to allocate exclusive memory
		CStringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), pData->nDataLength+1);
	}
}

void CString::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)	{
		Release();
		AllocBuffer(nLen);
	}
}

CString::~CString()
//  free any attached data
{
	if (GetData() != _afxDataNil) {
		if (--GetData()->nRefs <= 0)
			FreeData(GetData());
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

void CString::AllocCopy(CString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0) {
		dest.Init();
	}
	else {
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen);
	}
}


CString::CString(LPCSTR lpsz)
{
	Init();
	int nSrcLen = lpsz != NULL ? strlen(lpsz) : 0;
	if (nSrcLen != 0) {
		AllocBuffer(nSrcLen);
		memcpy(m_pchData, lpsz, nSrcLen+1);
	}
}

CString::CString(LPCSTR lpsz, int nLength)
{
	Init();
	int nSrcLen = lpsz != NULL ? strlen(lpsz) : 0;
	if (nLength < nSrcLen && nLength >=0)
		nSrcLen = nLength;
	if (nSrcLen != 0) {
		AllocBuffer(nSrcLen);
		memcpy(m_pchData, lpsz, nSrcLen);
		m_pchData[nSrcLen] = '\0';
	}
}

CString::CString(char ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		AllocBuffer(nLength);
		memset(m_pchData, ch, nLength);
	}
}


//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const CString&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void CString::AssignCopy(int nSrcLen, LPCSTR lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen);
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CString& CString::operator=(const CString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData) {
		if ((GetData()->nRefs < 0 && GetData() != _afxDataNil) ||
			stringSrc.GetData()->nRefs < 0) {
			// actual copy necessary since one of the strings is locked
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else {
			// can just copy references around
			Release();
			m_pchData = stringSrc.m_pchData;
			GetData()->nRefs++;
		}
	}
	return *this;
}

const CString& CString::operator=(LPCSTR lpsz)
{
	AssignCopy(strlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator=(char ch)
{
	AssignCopy(1, &ch);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// concatenation

// NOTE: "operator+" is done as friend functions for simplicity
//      There are three variants:
//          CString + CString
// and for ? = TCHAR, LPCSTR
//          CString + ?
//          ? + CString

void CString::ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data,
	int nSrc2Len, LPCSTR lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new CString object

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0) {
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len);
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len);
	}
}

CString operator+(const CString& string1, const CString& string2)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

CString operator+(const CString& string, LPCSTR lpsz)
{
	CString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		CString::SafeStrlen(lpsz), lpsz);
	return s;
}

CString operator+(LPCSTR lpsz, const CString& string)
{
	CString s;
	s.ConcatCopy(CString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

CString operator+(const CString& string1, char ch)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

CString operator+(char ch, const CString& string)
{
	CString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// concatenate in place

void CString::ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData)
{
	//  -- the main routine for += operators

	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)
		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)	{
		// we have to grow the buffer, use the ConcatCopy routine
		CStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		CString::Release(pOldData);
	}
	else {
		// fast concatenation when buffer big enough
		memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen);
		GetData()->nDataLength += nSrcLen;
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

const CString& CString::operator+=(LPCSTR lpsz)
{
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator+=(char ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CString& CString::operator+=(const CString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

LPTSTR CString::GetBuffer(int nMinBufLength)
{
	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength) {
		// we have to grow the buffer
		CStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1));
		GetData()->nDataLength = nOldLen;
		CString::Release(pOldData);
	}
	return m_pchData;
}

void CString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();  // just in case GetBuffer was not called

	if (nNewLength == -1)
		nNewLength = strlen(m_pchData); // zero terminated

	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR CString::GetBufferSetLength(int nNewLength)
{
	if (nNewLength < 0)
		return NULL;
	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void CString::FreeExtra()
{
	if (GetData()->nDataLength != GetData()->nAllocLength) {
		CStringData* pOldData = GetData();
		AllocBuffer(GetData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength);
		CString::Release(pOldData);
	}
}

LPTSTR CString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

void CString::UnlockBuffer()
{
	if (GetData()->nRefs != -1)
		return;
	if (GetData() != _afxDataNil)
		GetData()->nRefs = 1;
}

///////////////////////////////////////////////////////////////////////////////
// Commonly used routines (rarely used routines in STREX.CPP)

int CString::Find(char ch) const
{
	return Find(ch, 0);
}

int CString::Find(char ch, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart >= nLength)
		return -1;

	// find first single character
	LPTSTR lpsz = strchr(m_pchData + nStart, ch);

	// return -1 if not found and index otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CString::FindOneOf(LPCSTR lpszCharSet) const
{
	LPTSTR lpsz = strpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CString::Find(LPCSTR lpszSub) const
{
	return Find(lpszSub, 0);
}


int CString::Find(LPCSTR lpszSub, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return -1;

	// find first matching substring
	LPTSTR lpsz = strstr(m_pchData + nStart, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void CString::MakeUpper()
{
	CopyBeforeWrite();
	UPPER(m_pchData);
}

void CString::MakeLower()
{
	CopyBeforeWrite();
	LOWER(m_pchData);
}


int CString::Replace(LPCSTR lpszOld, LPCSTR lpszNew)
{
	// can't have empty or NULL lpszOld

	int nSourceLen = SafeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = SafeStrlen(lpszNew);

	// loop once to figure out the size of the result string
	int nCount = 0;
	LPTSTR lpszStart = m_pchData;
	LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
	LPTSTR lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = strstr(lpszStart, lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}
		lpszStart += strlen(lpszStart) + 1;
	}

	// if any changes were made, make them
	if (nCount > 0)
	{
		CopyBeforeWrite();

		// if the buffer is too small, just
		//   allocate a new buffer (slow but sure)
		int nOldLength = GetData()->nDataLength;
		int nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
		if (GetData()->nAllocLength < nNewLength || GetData()->nRefs > 1)
		{
			CStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, pOldData->nDataLength);
			CString::Release(pOldData);
		}
		// else, we just do it in-place
		lpszStart = m_pchData;
		lpszEnd = m_pchData + GetData()->nDataLength;

		// loop again to actually do the work
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = strstr(lpszStart, lpszOld)) != NULL)
			{
				int nBalance = nOldLength - (lpszTarget - m_pchData + nSourceLen);
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
					nBalance);
				memcpy(lpszTarget, lpszNew, nReplacementLen);
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += strlen(lpszStart) + 1;
		}
		GetData()->nDataLength = nNewLength;
	}

	return nCount;
}


CString CString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;


	// optimize case of returning entire string
	if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CString CString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

CString CString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

void CString::TrimRight()
{
	// find beginning of trailing spaces by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

//	while (*lpsz != '\0') {
//		if (isspace(*lpsz)) {
//			if (lpszLast == NULL)
//				lpszLast = lpsz;
//		}
//		else
//			lpszLast = NULL;
//		lpsz++;
//	}
	while (*lpsz != '\0') 
    {
		if (isspace(*lpsz)) //空格
        {
			if (lpszLast == NULL)
				lpszLast = lpsz;
            lpsz++;
            continue;
		}

        lpszLast = NULL;
        if ((unsigned char)(*lpsz) & 0x80) //汉字
        {
    		lpsz++;
            if (*lpsz == '\0') //应该不会出现
                break;
        }
        lpsz++;
	}


	if (lpszLast != NULL) {
		// truncate at trailing space start
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}

void CString::TrimRightChar(char ch/* = ' ' */)
{
	// find beginning of trailing spaces by starting at beginning (DBCS aware)
	
	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	
	while (*lpsz != '\0') 
    {
		if (*lpsz == ch) 
        {
			if (lpszLast == NULL)
				lpszLast = lpsz;
            lpsz++;
            continue;
		}
		
        lpszLast = NULL;
        if ((unsigned char)(*lpsz) & 0x80) //汉字
        {
			lpsz++;
            if (*lpsz == '\0') //应该不会出现
                break;
        }
        lpsz++;
	}
	
	
	if (lpszLast != NULL) {
		// truncate at trailing space start
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}


void CString::TrimLeft()
{
	// find first non-space character

	CopyBeforeWrite();
	LPCSTR lpsz = m_pchData;

	while (isspace(*lpsz))
		lpsz++;

	if (lpsz != m_pchData) {
		// fix up data and length
		int nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1));
		GetData()->nDataLength = nDataLength;
	}
}

void CString::TrimLeftChar(char ch)
{
	CopyBeforeWrite();
	LPCSTR lpsz = m_pchData;
	
//	while (isspace(*lpsz))
	while(lpsz[0] == ch)
		lpsz++;
	
	if (lpsz != m_pchData) {
		// fix up data and length
		int nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1));
		GetData()->nDataLength = nDataLength;
	}	
}

void CString::Trim()
{
	TrimLeft();
	TrimRight();
}

// formatting (using wsprintf style formatting)
#define MAX_STRLEN_IN_FORMAT 4096
//void CString::Format(LPCSTR lpszFormat, ...)
void CString::FormatV(LPCSTR lpszFormat, va_list argList)
{
//	va_list argList;
//	va_start(argList, lpszFormat);
//	va_list argListSave = argList;
	
	//	va_list argList,argListSave;
	va_list argListSave;
//	va_start(argList, lpszFormat);
  // argListSave = argList;
     va_copy(argListSave,argList);
     
	int nMaxLen = 0;
	for (LPCSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz++)
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(++lpsz) == '%')
		{
			nMaxLen ++;
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz++)
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
				;
		}

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz++;

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz++;
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
					;
			}
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if (strncmp(lpsz, "I64", 3) == 0)
		{
			lpsz += 3;
			nModifier = 1;
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case 'h':
				nModifier = 1;
				lpsz++;
				break;
			case 'l':
				nModifier = 1;
				lpsz++;
				break;

			// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz++;
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
		//	va_arg(argList, char);
			va_arg(argList,int);
			break;

		// strings
		case 's':
		case 'S':
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = strlen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, double);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				va_arg(argList, double);
				nItemLen = 128; // width isn't truncated
				// 312 == strlen("-1+(309 zeroes).")
				// 309 zeroes == max precision of a double
				nItemLen = max(nItemLen, 312+nPrecision);
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(argList, int*);
				break;
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	GetBuffer(max(nMaxLen, MAX_STRLEN_IN_FORMAT));
#ifndef WIN32
	vsnprintf(m_pchData, MAX_STRLEN_IN_FORMAT-1,lpszFormat, argListSave);
#else
    vsprintf(m_pchData,lpszFormat, argListSave);
#endif
	ReleaseBuffer();
//	va_end(argList);
	va_end(argListSave);
}

//add by Mac 20130509
void CString::Format(LPCSTR lpszFormat,...)
{ 
    va_list argList;
    va_start(argList, lpszFormat);
    FormatV(lpszFormat, argList);
    va_end(argList);
}

/*
// 2008.7.29 added by aps-lgr
BOOL CString::IsDigit() const
{
    int len = GetLength();
    if (len <= 0)
        return false;

	LPTSTR pos = GetBuffer();
    pos += len - 1;
    while (len-- > 0) {
        if (*pos > '9' || *pos < '0')
            return false;
        pos--;
    }
    return true;
}


// 2008.7.29 added by aps-lgr
BOOL CString::IsDecimal(char chDot) const
{
    int len = GetLength();
    if (len <= 0)
        return false;

	LPTSTR pos = GetBuffer();
    int dotflag = 0;
    pos += len - 1;
    while (len-- > 0) {
        if (*pos == chDot) {
            if (dotflag)
                return false;   // duplicate decimal
            dotflag = 1;
        } else if (*pos > '9' || *pos < '0')
            return false;
        pos--;
    }
    return true;
}
*/
