#include "stdafx.h"
#include <wchar.h>

#include "dbmtext.h"


#include "str_utils.h"


int str_countSymbol(const TCHAR *szSource, TCHAR ch)
{
  int i = 0;
  TCHAR *str = (TCHAR *)szSource;
  while(str && *str ) 
  {
    if (*str == ch) i++;
    *str++;
  }
  return i;
}


TCHAR* str_ltrim(TCHAR *szSource, TCHAR ch)
{
  while( *szSource  == ch ) *szSource++;
  return szSource;
}

TCHAR* str_rtrim(TCHAR *szSource, TCHAR ch)
{
  TCHAR *pszEOS;
  pszEOS = szSource + _tcslen( szSource ) - 1;
  while( pszEOS >= szSource && *pszEOS == ch ) *pszEOS-- = '\0';
  return szSource;
}

TCHAR* str_trim(TCHAR *s, TCHAR ch)
{
  return str_rtrim(str_ltrim(s, ch), ch);
}

int str_spaces(TCHAR *s)
{
   while (*s == ' ')    s++;
   return !*s;
}

/*------------¬ыравнивание вправо с заполнением пробелами------*/
TCHAR* str_right_justify(TCHAR *s)
{
  size_t len = _tcslen(s);
  while (*s == ' ' || *s == '0' && len)  {
    len--;
    *s++ = ' ';
  }
  if (len)
    while (*(s+(len-1)) == ' ') {
#ifdef UNICODE
      wmemmove((wchar_t*)s, (wchar_t*)(s + 1), len-1);
#else
      memmove((char*)s, (char*)(s + 1), len-1);
#endif
      *s = ' ';
    }
  return s;
}

/*-----------¬ыравнивание вправо с заполнением нул€ми----------*/
TCHAR* str_right_justify_zero_fill(TCHAR *s)
{
    size_t len;
    if (str_spaces(s))
        return s;
    len = _tcslen(s);
    while (*(s + len - 1) == ' ')   {
#ifdef UNICODE
      wmemmove((wchar_t*)s, (wchar_t*)(s + 1), len-1);
#else
      memmove((char*)s, (char*)(s + 1), len-1);
#endif
      *s = '0';
    }
	return s;
}

//-----------------------------------------------------------------------------
// Returns an index for the keyWord in initList.
//
int  indexOfKeyWord(LPCTSTR keyWord, LPCTSTR initList, LPCTSTR DelimStr)
{
    TCHAR* spaceAfterKW = NULL;
    TCHAR* tmpPtr       = NULL;
    TCHAR* kwPosition   = NULL;
    int i = 0;

    // Set the tmpPtr to the character after the first underscore. This
    // will be the start of the non-translated strings. 
    // acedGetInput() returns English!
    //
    // Bypass the const'ness of initList.
    tmpPtr = (TCHAR*)_tcsstr(initList, *&(initList));
    kwPosition = _tcsstr(tmpPtr, keyWord);

    while(spaceAfterKW < kwPosition) {
        spaceAfterKW = _tcsstr(tmpPtr, DelimStr);
        if (spaceAfterKW == NULL || spaceAfterKW > kwPosition) 
            break;
        tmpPtr = spaceAfterKW + _tcslen(DelimStr);
        i++;
    }
    return i;
}

//подстрока значени€ по ключу из строки вида "Key=Value\n..."
TCHAR* strGetByKey(TCHAR *bufVal, const TCHAR* keyWord, const TCHAR* iniStr, const TCHAR* delimetr,
                   size_t MaxIniStr )
{
  TCHAR *tmpPtr, *eq;
  // получаем указатель на первый символ в inistr.
  tmpPtr = (TCHAR *)_tcsstr(iniStr, *&(iniStr));
  do
  {
    _tcsncpy(bufVal, tmpPtr,  MaxIniStr);
    tmpPtr = _tcsstr(bufVal, delimetr);
    eq = _tcschr(bufVal, '=');
    if ( eq && !_tcsnicmp( keyWord, bufVal, eq-bufVal) )
    {
      int len = eq - bufVal + 1;
      wmemmove((wchar_t*)bufVal, (wchar_t*)(bufVal + len), _tcslen(bufVal)-1);
      if ( tmpPtr )
        *(tmpPtr - len) = 0;
      return bufVal;
    }
    if ( tmpPtr )
     tmpPtr += _tcslen(delimetr);
  }
  while (tmpPtr);

  return NULL;
}


//разделение строки на 2 подстроки по разделителю Delim. –азделитель в состав подстрок не входит
bool strSplit2(const TCHAR* Str, TCHAR *Str1, TCHAR *Str2, TCHAR *Delim /* ="=" */)
{
  TCHAR *tmpPtr = (TCHAR *)_tcsstr(Str, Delim);
  if (tmpPtr)
  {
    _tcscpy(Str2, (TCHAR*)(tmpPtr+_tcslen(Delim)));
    int len = tmpPtr - Str;
    _tcsncpy(Str1, Str, len );
    Str1[len] = 0;
  }
  return (tmpPtr != NULL);
}


int strCompareNum(LPCTSTR  s1, LPCTSTR  s2)
{
  ASSERT(s1 != NULL && s2 != NULL);
  return _tstoi(s1) > _tstoi(s2);
}

bool strIsWild(LPCTSTR string, LPCTSTR wild)
{
  const TCHAR *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return false;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return true;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}


#ifdef _INC_STRING


int strCompareAlphaNum(CString strItem1, CString strItem2)
{
  CString strvern(_T("0123456789"));

   int pos1 = strItem1.FindOneOf(strvern);
   int pos2 = strItem2.FindOneOf(strvern);

   if ( pos1 > -1 )
   { 
     CString s1, s2;
     s1 = strItem1.Left(pos1);
     s2 = strItem2.Left(pos2);
     if (!s1.Compare(s2))
     {
       s1 = strItem1.Mid(pos1);
       s2 = strItem2.Mid(pos2);
       int i1 = _tstoi(s1);
       int i2 = _tstoi(s2);
       if (i1 != i2)
         return i1 - i2;
       s1.MakeReverse();
       s2.MakeReverse();
       int pos1 = s1.FindOneOf(strvern);
       int pos2 = s2.FindOneOf(strvern);
       s1.MakeReverse();
       s2.MakeReverse();
       strItem1 = s1.Mid(pos1);
       strItem2 = s2.Mid(pos2);
     }
   }
   return strItem1.Compare(strItem2);
}


bool strSplit2(const CString& Str, CString& Str1, CString& Str2, LPCTSTR  Delim /* ="=" */)
{
  int i = Str.Find(Delim);
  if ( i == -1) return false;
  Str1 = Str.Left(i);
  Str2 = Str.Mid(i+1);
  return true;
}


//—обирает список строк в строку раздел€€ строки символами delimetr
//≈сли delimAsBounds == true, то символы delimetr добавл€ютс€ как спереди так и сзади строки
void StrArrayToString (const CStringArray &lstStr, CString& str, LPCTSTR  delimetr,
                       bool delimAsBounds /*= false*/, bool TrimStr /*= false*/ )
{
  CString s;
  str = delimAsBounds ? delimetr : _T("");
  int i;
  for ( i = 0; i < lstStr.GetSize(); i++ )
  {
    s = lstStr.GetAt(i);
    if (TrimStr)
    {
      s.TrimLeft();
      s.TrimRight();
    }
    str += s + delimetr;
  }
  if ( i > 0 && !delimAsBounds)
    str = str.Left(str.GetLength() - _tcslen(delimetr));
  if ( i == 0 && delimAsBounds)
    str += delimetr;
}

//разбивает строку S на массив подстрок, разделенные символоми delimetr
//≈сли delimAsBounds == true, то символы delimetr рассматриваютс€ как пол€ таблицы, т.е. 
//если строка с них начинаетс€, то пуста€ строка не добавл€етс€ (то же в конце).


int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {

   if (code == EXCEPTION_ACCESS_VIOLATION) {
      AfxMessageBox(_T("EXCEPTION_ACCESS_VIOLATION"));
      return EXCEPTION_EXECUTE_HANDLER;
   }
   else {
      AfxMessageBox(_T("EXCEPTION_CONTINUE_SEARCH"));
      return EXCEPTION_CONTINUE_SEARCH;
   };
}


void StringToStrArray (LPCTSTR  str, CStringArray& lstStr, LPCTSTR  delimetr, 
                       bool delimAsBounds /*= false*/, bool TrimStr /*= false*/ )
{
  lstStr.RemoveAll();
  if (!str || !delimetr || 
    *str == 0 || *delimetr == 0 ||
     *str == '\0' || *delimetr == '\0' )
    return;

/*
  CString s, sTmp;
  s = str;
  s.LockBuffer();
  int dlen, ib = 0, ie = 0;
  dlen = _tcslen(delimetr);
  if ( delimAsBounds && s.Left(_tcslen(delimetr)) == delimetr )
    s = s.Mid(_tcslen(delimetr));
  while ( (ie = s.Find( delimetr, ib )) > -1 )
  {
    if ( ib + (ie-ib) > s.GetLength() )
      break;
    sTmp = s.Mid(ib,ie-ib);
    lstStr.Add( sTmp );
    ib = ie + dlen;
  }
  if ( ib > 0 && (ib != s.GetLength() || !delimAsBounds ) )
    lstStr.Add( s.Mid(ib) );
  else
  if ( ib == 0 && s.GetLength() )
    lstStr.Add( s );
*/

  int slen, dlen, nn;
  slen = _tcslen(str);
  dlen = _tcslen(delimetr);
  ASSERT(dlen < 512);
  TCHAR s[512];
  TCHAR *sA = 0, *ib = 0, *ie = 0;
  ib = (TCHAR*)str;
  if ( delimAsBounds )
  {
    if (dlen >= slen)
      return;

    ie = _tcsstr(ib, delimetr);
    if (!ie)
      return;

    ib = ie + dlen;
    ie = 0;
  }

  while ( ib != 0 && (ie=_tcsstr( ib, delimetr)) != 0 )
  {
    nn = ie-ib;
    ASSERT( nn >= 0);
    nn = min(ie-ib, 511);
    _tcsncpy(s, ib, nn);
    s[nn] = '\0';
    sA = s;
    if (TrimStr)
    {
      sA = str_trim(sA, ' ');
      sA = str_trim(sA, '\t');
    }
    lstStr.Add(CString(sA));

    ib = ie + dlen;
    if (ib-str >= slen)
      ib = 0;
  }

  if (ib && *ib && _tcslen(ib)>0 && 
      ((ib-ie) == slen || !delimAsBounds ))
    sA = ib;
  else
    return;

  if (sA && *sA && _tcslen(sA))
  {
    if (TrimStr)
    {
      sA = str_trim(sA, ' ');
      sA = str_trim(sA, '\t');
    }
    lstStr.Add(CString(sA));
  }
}


void StrDataSetValue(CString& sData, LPCTSTR sName, LPCTSTR sValue, LPCTSTR delimData /*= _T("\t")*/, LPCTSTR delimVal /*= _T("=")*/)
{
  CString s, sN, sV;
  CStringArray lstStr, lstStrOut;
  bool bExistVal = false;

  StringToStrArray(sData, lstStr, delimData);

#if _MFC_VER == 0x0600 // Microsoft Foundation Classes version 6.00, VC++ 6.0
  for(int i=0; i < lstStr.GetSize(); i++)
#else
  for(int i=0; i < lstStr.GetCount(); i++)
#endif
  {
    strSplit2(lstStr[i], sN, sV, delimVal);
    if (sN == sName)
    {
      sV = sValue;
      bExistVal = true;
    }
    s.Format(_T("%s%s%s"), sN, delimVal, sV);
    lstStrOut.Add(s);
  }

  if (!bExistVal)
  {
    s.Format(_T("%s%s%s"), sName, delimVal, sValue);
    lstStrOut.Add(s);
  }

  StrArrayToString(lstStrOut, sData, delimData);
}

bool StrDataGetValue(const CString& sData, CString sName, CString &sValue, LPCTSTR delimData /*= _T("\t")*/, LPCTSTR delimVal /*= _T("=")*/)
{
  CString sN, sV;
  CStringArray lstStr;
  StringToStrArray(sData, lstStr, delimData);
//#if _MFC_VER == 0x0600 // Microsoft Foundation Classes version 6.00, VC++ 6.0
  for(int i=0; i < lstStr.GetSize(); i++)
//#else
//  for(int i=0; i < lstStr.GetCount(); i++)
//#endif
  {
    strSplit2(lstStr[i], sN, sV, delimVal);
    if (sN == sName)
    {
      sValue = sV;
      return true;
    }
  }
  return false;
}


void StrIntsToIntArray (LPCTSTR  strInts, CUIntArray& uintAr, LPCTSTR  delimetr /*","*/)
{
  CStringArray lstStr;
  StringToStrArray(strInts, lstStr, delimetr);
  uintAr.SetSize(lstStr.GetSize());
  for (int i = 0; i < lstStr.GetSize(); i++)
    uintAr[i] = _tstoi(lstStr[i]);
}

void IntArrayToStrInts (const CUIntArray& uintAr, CString& strInts, LPCTSTR  delimetr /*","*/)
{
  strInts.Empty();
  CStringArray lstStr;
  lstStr.SetSize(uintAr.GetSize());
  for (int i = 0; i < lstStr.GetSize(); i++)
    lstStr[i].Format(_T("%d"), uintAr[i]);
  StrArrayToString(lstStr, strInts, delimetr);
}


int findInStringArray(LPCTSTR  sFinded, const CStringArray &lstStr, bool bNoCase /*=true*/)
{
  for(int i=0; i < lstStr.GetSize(); i++)
    if (bNoCase)
    {
      if (!lstStr[i].CompareNoCase(sFinded))
        return i;
    }
    else
    {
      if (!lstStr[i].Compare(sFinded))
        return i;
    }
  return -1;
}


int findInStringArray(LPCTSTR  sFinded, const CString strList, LPCTSTR  delimetr, bool bNoCase /*=true*/)
{
  CStringArray lstStr;
  StringToStrArray (strList, lstStr, delimetr, false, true);
  return findInStringArray(sFinded, lstStr, bNoCase);
}


CString getFieldByName(LPCTSTR  fieldNames, LPCTSTR  fields, LPCTSTR  name, LPCTSTR  delim /*="\t"*/)
{
  CStringArray fieldAr;
  StringToStrArray ( fieldNames, fieldAr, delim );
  int i = 0;
  for ( ; i < fieldAr.GetSize(); i++ )
    if ( !fieldAr[i].CompareNoCase(name) )
      break;
  if ( i < fieldAr.GetSize() )
  {
    StringToStrArray ( fields, fieldAr, delim );
    if (i < fieldAr.GetSize())
      return fieldAr[i];
  }
  return _T("");
}

//¬озвращает расширение без точки
CString FileExt(LPCTSTR  fname)
{
  CString ext = fname;
  int i = ext.ReverseFind('.');
  if ( i >= 0)  return ext.Mid(i+1);
  return _T("");
}

//установка нового расширени€ 
void FileChangeExt(CString& fname, LPCTSTR  newExt)
{
  int iSlash = fname.ReverseFind('\\');
  int iPoint = fname.ReverseFind('.');
  if ( iSlash < iPoint && iPoint >= 0)  fname = fname.Left(iPoint);
  fname = fname  + (newExt[0] == '.' ? _T("") : newExt[0] == 0 ? _T("") : _T(".")) + newExt;
}

//извлечение имени файла из полного пути 
CString& FileNameExtract(CString& fname)
{
  int i = fname.ReverseFind('\\');
  if ( i >= 0 ) fname = fname.Mid(i+1);
  return fname;
}


//CString& FilePath(CString& fname, bool WitchBackSlash /*= true*/)
//{
//  int i = fname.ReverseFind('\\');
//  if ( fname.GetLength()-i > 3 &&  i >= 0 )  fname = fname.Left( WitchBackSlash ? i+1 : i);
//  return fname;
//}

//извлечение пути из имени файла c закрывающим слешом
CString& PathRemoveFileSpecS(CString& path, bool WitchBackSlash /*= true*/)
{
  int len = path.GetLength();
  int i = path.ReverseFind('\\');
  if (i >= 0 && len-i > 3 && i < len-1)  path = path.Left( WitchBackSlash ? i+1 : i);
  return path;
}

//добавление закрывающего слеша
CString& PathAddBackslashS(CString& path)
{
  int len = path.GetLength();
  if ( len && path[len-1] != '\\') path += _T("\\");
  return path;
}

//удаление закрывающего слеша
CString& PathRemoveBackslashS(CString& path)
{
  int len = path.GetLength();
  if ( len && path[len-1] == '\\') 
    path = path.Left(len-1);
  return path;
}


bool FileExists(LPCTSTR fname)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  hFind = FindFirstFile(fname, &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE) {
    return false;
  } else {
    FindClose(hFind);
  }
  return true;
}

int FileTimeCompare(LPCTSTR fn1, LPCTSTR fn2)
{
  int retVal = -2;
  HANDLE hF1, hF2;
  WIN32_FIND_DATA FindFileData1, FindFileData2;
  hF1 = FindFirstFile(fn1, &FindFileData1);
  if (hF1 != INVALID_HANDLE_VALUE) {
    hF2 = FindFirstFile(fn2, &FindFileData2);
    if (hF2 != INVALID_HANDLE_VALUE) {
      retVal = CompareFileTime(&FindFileData1.ftLastWriteTime, &FindFileData2.ftLastWriteTime);
      FindClose(hF2);
    }
    FindClose(hF1);
  }
  return retVal;
}


//получение списка файлов по маске
void setDirListFile( LPCTSTR  mask, CStringList& list)
{
  CFileFind finder;
  BOOL bWorking = finder.FindFile(mask);
  while (bWorking)
  {
    bWorking = finder.FindNextFile();
    list.AddTail(finder.GetFileName());
  }
}


#ifdef _CONVERSION_USES_THREAD_LOCALE

static int CALLBACK
BrowseCallbackProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    TCHAR szPath[_MAX_PATH];
    switch (uMsg) {
    case BFFM_INITIALIZED:
        if (lpData)
            SendMessage(hWnd,BFFM_SETSELECTION,TRUE,lpData);
        break;
    case BFFM_SELCHANGED:
        SHGetPathFromIDList(LPITEMIDLIST(lParam),szPath);
        SendMessage(hWnd, BFFM_SETSTATUSTEXT, NULL, LPARAM(szPath));
        break;
    }
    return 0;
}

BOOL GetFolder (LPCTSTR szTitle, LPTSTR szPath, LPCTSTR szRoot, HWND hWndOwner)
{
    if (szPath == NULL)
        return false;

    bool result = false;

    LPMALLOC pMalloc;
    if (::SHGetMalloc(&pMalloc) == NOERROR) {
        BROWSEINFO bi;
        ::ZeroMemory(&bi,sizeof bi);
        bi.ulFlags   = BIF_RETURNONLYFSDIRS;

        // дескриптор окна-владельца диалога
        bi.hwndOwner = hWndOwner;

        // добавление заголовка к диалогу
        bi.lpszTitle = szTitle;

        // отображение текущего каталога
        bi.lpfn      = BrowseCallbackProc;
        bi.ulFlags  |= BIF_STATUSTEXT;

        // установка каталога по умолчанию
        bi.lParam    = LPARAM(szPath);

        // установка корневого каталога
        if (szRoot != NULL) {
            IShellFolder *pDF;
            if (SHGetDesktopFolder(&pDF) == NOERROR) {
                LPITEMIDLIST pIdl = NULL;
                ULONG        chEaten;
                ULONG        dwAttributes;

                USES_CONVERSION;
                LPOLESTR oleStr = T2OLE(szRoot);

                pDF->ParseDisplayName(NULL,NULL,oleStr,&chEaten,&pIdl,&dwAttributes);
                pDF->Release();

                bi.pidlRoot = pIdl;
            }
        }

        LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
        if (pidl != NULL) {
            if (::SHGetPathFromIDList(pidl,szPath))
                result = true;
            pMalloc->Free(pidl);
        }
        if (bi.pidlRoot != NULL)
            pMalloc->Free((void *)bi.pidlRoot);
        pMalloc->Release();
    }
    return result;
}

#endif //_CONVERSION_USES_THREAD_LOCALE


void mtextContentsToString(AcDbMText* pMText, CString& contents)
{
  TCHAR *cbuf;
  cbuf = pMText->contents();
  contents = cbuf;
#ifndef _NCAD_BUILD_NODELETE
  free(cbuf);
#endif
  contents.TrimRight(_T('}'));
  contents.TrimLeft(_T('{'));
  //contents = contents.Mid(contents.Find(_T(';'))+1);
  contents.Replace( pMText->paragraphBreak(), _T("\r\n"));
  //contents.TrimLeft(_T("\r\n"));
}

void stringToMtextContents(CString &contents, AcDbMText *pMText, bool EraseClear /*= true*/)
{
  //contents.TrimRight(_T(" "));
  //contents.TrimLeft (_T(" "));
  if ( EraseClear && contents.IsEmpty() )
    pMText->erase();
  else 
  {
    contents.Replace(_T("\r\n"), pMText->paragraphBreak());
    //contents.TrimLeft(pMText->paragraphBreak());
    //contents = _T("{\\F") + ElOptions::getOpt()->schemFont +_T(";") + contents + _T("}");
    contents = _T("{") + contents + _T("}");
    if (!pMText->isWriteEnabled())
      pMText->upgradeOpen();
    RXASSERT(pMText->isWriteEnabled());
    pMText->setContents(contents); 
  }
}

void mtextContentsToStrArray(AcDbMText* pMText, CStringArray& strAr)
{
  CString contents;
  mtextContentsToString(pMText, contents);
  StringToStrArray(contents, strAr, _T("\n"));
}

void strArrayToMtextContents(const CStringArray &strAr, AcDbMText *pMText, bool EraseClear /*= true*/)
{
  int i;
  if (EraseClear)
  {
    for (i=0; i < strAr.GetSize(); i++)
      if ( _tcslen(strAr[i]) )
        break;
    if ( i == strAr.GetSize() )
    {
      pMText->erase();
      return;
    }
  }
  CString contents;
  contents = pMText->blockBegin();
  //contents += _T("\\F") + ElOptions::getOpt()->schemFont + _T(";");
  for (i=0; i < strAr.GetSize(); i++)
  {
    if ( i > 0 )
      contents += pMText->paragraphBreak();
    contents += strAr[i];
  }
  contents += pMText->blockEnd();
  pMText->setContents(contents);
}

void splitStringByContents(CString& newStr, const CString& contents )
{
  int i = contents.Find(_T("\r\n"));
  if ( i > -1 )
  {
    CString tmp = contents.Left(i);
    int count = 0;
    for (i = 0; (i = tmp.Find(',', i)) >= 0; i++ ) count++;
    if (count)
    {
      for(int f=0, cnt = 1; f+2 < newStr.GetLength(); f++, cnt++)
      {
        f = newStr.Find(',', f);
        if ( f > -1 )
        {
          if ( cnt % count == 0 )
          {
            newStr.Insert(++f, '\r');
            newStr.Insert(++f, '\n');
          }
        }
        else
          break;
      }
    }
  }
}

int StrFindSpace(LPCTSTR s, int minCount, int* realCount)
{
  int retCount = 0, count = 0, i = 0; 
  if (realCount) *realCount = 0;
  if (s != NULL) 
    while(s[i])
    {
      if (_istspace(s[i]))
        count++;
      else
      {
        if (retCount >= minCount && retCount >= count)
          break;
        count = 0;
      }
      if (count >= minCount)
        retCount = count;
      i++;
    }
    if (retCount > 0)
    {
      if (realCount)
        *realCount = retCount;
      return i - retCount;
    }
    return -1;
}

bool StrSplitBySpace(LPCTSTR s, CStringArray& lstStr, AcDbIntArray* startIdxs /*= NULL*/, AcDbIntArray* lenSpacesCounts /*= NULL*/)
{
  lstStr.RemoveAll();
  if (startIdxs) startIdxs->setLogicalLength(0);
  if (lenSpacesCounts) lenSpacesCounts->setLogicalLength(0);
  int iSt, iCount = 0, iCountAll = 0;
  CString st, tt = s;
  bool bRet = false;
  while ( (iSt = StrFindSpace(tt, 3, &iCount)) >= 0 )
  {
    st = tt.Left(iSt);
    tt = tt.Mid(iSt + iCount);
    lstStr.Add(st);
    if (startIdxs) startIdxs->append(iCountAll + iSt);
    if (lenSpacesCounts) lenSpacesCounts->append(iCount);
    //acutPrintf(_T("\n\"%s\""), st);
    //acutPrintf(_T("\nЌачало:%i,  ол-во %i"), iCountAll + iSt, iCount);
    iCountAll += iSt + iCount;
    bRet = true;
  }
  if (!tt.IsEmpty())
  {
    lstStr.Add(tt);
    if (startIdxs) startIdxs->append(-1);
    if (lenSpacesCounts) lenSpacesCounts->append(0);
    //acutPrintf(_T("\n\"%s\""), tt);
    //acutPrintf(_T("\nЌачало:%i,  ол-во %i"), -1, 0);
  }
  return bRet;
}

CString GenGUID()
{
  CString s;
  CLSID ClassID;
  LPOLESTR p;
  CoCreateGuid(&ClassID);
  StringFromCLSID(ClassID, &p);
  s = p;
  CoTaskMemFree(p);
  return s;
}

#endif //_INC_STRING
