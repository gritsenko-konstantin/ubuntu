/*************************************************
	      
		CString Class From Microsoft MFC 

*************************************************/

#ifndef __CSTRING__
#define __CSTRING__
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifndef LPCSTR
#define LPCSTR const char*
#define LPTSTR char*
#endif

#ifndef BOOL 
#define BOOL int
#endif

#ifndef TRUE 
#define TRUE -1
#define FALSE 0
#endif

#define NOTFOUND -1			//在字符串中找不到指定的字符或子串

class SafeBuffer {
public:
	SafeBuffer(int nBufferLen) { if (nBufferLen > 0) m_pBuffer = new char[nBufferLen]; 
									else m_pBuffer = 0; m_nLength = nBufferLen; }
	~SafeBuffer() { if(m_pBuffer) delete[] m_pBuffer; }
	char* GetBuffer() { return m_pBuffer; }
	operator char*() { return m_pBuffer; }
	int GetLength() { return m_nLength; }
private:
	char* m_pBuffer;
	int m_nLength;
};

	
struct CStringData
{
	long nRefs;             // reference count
	int nDataLength;        // length of data (including terminator)
	int nAllocLength;       // length of allocation

	char* data()           // TCHAR* to managed data
		{ return (char*)(this+1); }
};


class CString {
public:
// Constructors

	// constructs empty CString
	CString();
	// copy constructor
	CString(const CString& CStringSrc);
	// from a single character
	CString(char ch, int nRepeat = 1);
	// from an ANSI CString (converts to char)
	CString(LPCSTR lpsz);
	// subset of characters from an ANSI CString (converts to char)
	CString(LPCSTR lpch, int nLength);
	// from unsigned characters
	CString(const unsigned char* psz);

// Attributes & Operations

	// get data length
	int GetLength() const { return GetData()->nDataLength; }
	// TRUE if zero length
	BOOL IsEmpty() const { return GetData()->nDataLength == 0; }
	// clear contents to empty
	void Empty();

	operator LPCSTR() const { return m_pchData; }

	// overloaded assignment

	// ref-counted copy from another CString
	const CString& operator=(const CString& CStringSrc);
	// set CString content to single character
	const CString& operator=(char ch);
	// copy CString content from ANSI CString (converts to char)
	const CString& operator=(LPCSTR lpsz);
	// copy CString content from unsigned chars
	const CString& operator=(const unsigned char* psz);

	// CString concatenation

	// concatenate from another CString
	const CString& operator+=(const CString& CString);

	// concatenate a single character
	const CString& operator+=(char ch);
	// concatenate a UNICODE character after converting it to char
	const CString& operator+=(LPCSTR lpsz);

	//是否是数字，不包括"."
	BOOL IsDigit() const;

	//是否是数值
	//chDot: 小数点字符，比如'.'或','
	BOOL IsDecimal(char chDot) const;

	friend CString  operator+(const CString& CString1,
			const CString& CString2);
	friend CString  operator+(const CString& CString, char ch);
	friend CString  operator+(char ch, const CString& CString);
	friend CString  operator+(const CString& CString, LPCSTR lpsz);
	friend CString  operator+(LPCSTR lpsz, const CString& CString);

	// CString comparison

	// straight character comparison
	int Compare(LPCSTR lpsz) const { return strcmp(m_pchData, lpsz); };
	int CompareN(LPCSTR lpsz, int n) const { return strncmp(m_pchData, lpsz, n); };
	// simple sub-CString extraction

	// return nCount characters starting at zero-based nFirst
	CString Mid(int nFirst, int nCount) const;
	// return all characters starting at zero-based nFirst
	CString Mid(int nFirst) const;
	// return first nCount characters in CString
	CString Left(int nCount) const;
	// return nCount characters from end of CString
	CString Right(int nCount) const;


	// upper/lower/reverse conversion

	// NLS aware conversion to uppercase
	void MakeUpper();
	// NLS aware conversion to lowercase
	void MakeLower();

	// trimming whitespace (either side)

	// remove whitespace starting from right edge
	void TrimRight();
	// remove whitespace starting from left side
	void TrimLeft();

	void TrimLeftChar(char ch = '0');
	void TrimRightChar(char ch= ' ');

	void Trim();

	// advanced manipulation

	// replace occurrences of chOld with chNew
	int Replace(LPCSTR lpszOld, LPCSTR lpszNew);
	// remove occurrences of chRemove
	int Remove(char chRemove);
	// insert character at zero-based index; concatenates
	// if index is past end of CString
	int Delete(int nIndex, int nCount = 1);

	// searching

	// find character starting at left, -1 if not found
	int Find(char ch) const;
	// find character starting at right
	int ReverseFind(char ch) const;
	// find character starting at zero-based index and going right
	int Find(char ch, int nStart) const;
	// find first instance of any character in passed CString
	int FindOneOf(LPCSTR lpszCharSet) const;
	// find first instance of subCString
	int Find(LPCSTR lpszSub) const;
	// find first instance of subCString starting at zero-based index
	int Find(LPCSTR lpszSub, int nStart) const;

	// simple formatting

    //add by Mac 20130426
    void FormatV(LPCSTR lpszFormat, va_list argList);//格式化字符串
    
	// printf-like formatting using passed CString
	void Format(LPCSTR lpszFormat, ...);

	LPTSTR GetBuffer(int nMinBufLength);
	// release buffer, setting length to nNewLength (or to first nul if -1)
	void ReleaseBuffer(int nNewLength = -1);
	// get pointer to modifiable buffer exactly as long as nNewLength
	LPTSTR GetBufferSetLength(int nNewLength);
	// release memory allocated to but unused by string
	void FreeExtra();

	// Use LockBuffer/UnlockBuffer to turn refcounting off

	// turn refcounting back on
	LPTSTR LockBuffer();
	// turn refcounting off
	void UnlockBuffer();

// Implementation
public:
	~CString();
	int GetAllocLength() const;

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data

	// implementation helpers
	CStringData* GetData() const { return ((CStringData*)m_pchData)-1; }
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data, int nSrc2Len, LPCSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void Release(CStringData* pData);
	static int SafeStrlen(LPCSTR lpsz) {return (lpsz == NULL) ? 0 : strlen(lpsz); }
	static void FreeData(CStringData* pData);
};


inline 
BOOL operator==(const CString& s1, const CString& s2)
{ return s1.Compare(s2) == 0; }
inline 
BOOL operator<=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) <= 0; }
inline 
BOOL operator<(const CString& s1, const CString& s2)
{ return s1.Compare(s2) < 0; }
inline 
BOOL operator>=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) >= 0; }
inline 
BOOL operator>(const CString& s1, const CString& s2)
{ return s1.Compare(s2) > 0; }
inline 
BOOL operator==(const CString& s1, LPCSTR s2)
{ return s1.Compare(s2) == 0; }
inline 
BOOL operator==(LPCSTR s1, const CString& s2)
{ return s2.Compare(s1) == 0; }

inline 
BOOL operator!=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) != 0; }
inline 
BOOL operator!=(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) != 0; }
inline 
BOOL operator!=(LPCSTR s1, const CString& s2)
{ return s2.Compare(s1) != 0; }

inline
CString Space(int nLength) { return CString(' ', nLength); }


class NameList {
public:
	NameList(LPCSTR pSeparator = ";"){m_strSeparator = pSeparator; }
	NameList(NameList& names);
	NameList& operator=(NameList& names);
	NameList& operator=(CString& s);

	void Put(LPCSTR pItem) { m_strList += pItem; m_strList += m_strSeparator; m_strListOld= m_strList; }
	CString Get();
	int GetItemCount();

	void Back() { m_strList = m_strListOld; }
	
	BOOL IsEmpty() { return m_strList.IsEmpty(); }

	CString ToString();
private:
	CString m_strListOld;
	CString m_strList;
	CString m_strSeparator;
};

inline
NameList::NameList(NameList& names)
{ 
	m_strList = names.m_strList;
	m_strSeparator = names.m_strSeparator;
}

inline
NameList& NameList::operator=(NameList& names) 
{ 
	m_strList = names.m_strList;
	m_strSeparator = names.m_strSeparator;
	return *this; 
}

inline
CString NameList::Get()
{
	int iPos = m_strList.Find(m_strSeparator);
	if (iPos == -1)
		return CString();
	CString sRet = m_strList.Left(iPos);
	m_strList = m_strList.Mid(iPos + strlen(m_strSeparator));
	return sRet;
}

inline 
CString NameList::ToString()
{
	return m_strList;
}

inline
NameList& NameList::operator=(CString& s)
{
	m_strList = s;
	return *this;
}

//全局函数
//将i转换为字符串，不足的长度前补0，iLen为字符串的长度
CString itoa0(int iNo, int iLen);

//将i转换为字符串
CString itoa(int iNo);

//将整数转换为字符串，存放到buff中
//iDec表示进制，目前只能处理10以下的进制
#ifndef WIN32
void itoa(int iNo, char* buffer, int iDec = 10);
#endif

//将strADate和strATime转换为tm，cDatetime必须在调用本函数之前分配内存
BOOL atot(tm& cDatetime, const CString& strADate, const CString& strATime = "", BOOL bHasCentury = TRUE);

//将tm转换为字符串, 如果pTime = 0, 则只转换pDate, 转换失败返回FALSE
BOOL ttoa(tm& cDatetime, CString& strDate, CString& strTime, BOOL bHasCentury = TRUE);

#endif

