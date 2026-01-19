#ifndef _STR_UTILS_H
#define _STR_UTILS_H

TCHAR* str_ltrim(TCHAR *szSource, TCHAR ch = ' ');
TCHAR* str_rtrim(TCHAR *szSource, TCHAR ch = ' ');
TCHAR* str_trim(TCHAR *c, TCHAR ch = ' ');
int   str_spaces(TCHAR *c);

int str_countSymbol(const TCHAR *szSource, TCHAR ch);

/*------------Выравнивание вправо с заполнением пробелами------*/
TCHAR* str_right_justify(TCHAR *s);

/*-----------Выравнивание вправо с заполнением нулями----------*/
TCHAR* str_right_justify_zero_fill(TCHAR *s);

//индекс ключа в строке с ключами, разделёнными пробелами ( с 0 )
int  indexOfKeyWord(LPCTSTR keyWord, LPCTSTR initList, LPCTSTR DelimStr = _T(" "));

//подстрока значения по ключу из строки вида "Key=Value\n..."
TCHAR* strGetByKey(LPTSTR bufVal, LPCTSTR keyWord, LPCTSTR iniStr, LPCTSTR delimetr, size_t MaxIniStr);

//разделение строки на 2 подстроки по разделителю Delim. Разделитель в состав подстрок не входит
bool strSplit2(const TCHAR* Str, TCHAR *Str1, TCHAR *Str2, TCHAR *Delim = _T("="));

int strCompareNum(LPCTSTR  s1, LPCTSTR  s2);
bool strIsWild(LPCTSTR string, LPCTSTR wild);

#ifdef _INC_STRING

int strCompareAlphaNum(CString strItem1, CString strItem2);

//разделение строки на 2 подстроки по разделителю Delim. Разделитель в состав подстрок не входит
bool strSplit2(const CString& Str, CString& Str1, CString& Str2, LPCTSTR  Delim = _T("="));

//получение списка файлов по маске
void setDirListFile( LPCTSTR  mask, CStringList& list);

//выбор каталога
BOOL GetFolder(LPCTSTR szTitle,LPTSTR szPath,LPCTSTR szRoot=NULL,HWND hWndOwner=NULL);

//Возвращает расширение без точки
CString FileExt(LPCTSTR  fname);

//изменение расширения имени файла
void FileChangeExt(CString& fname, LPCTSTR  newExt);

//извлечение имени файла из полного пути 
CString& FileNameExtract(CString& fname);

//извлечение пути из имени файла 
//CString& FilePath(CString& fname, bool WitchBackSlash = true);
CString& PathRemoveFileSpecS(CString& path, bool WitchBackSlash = true);
CString& PathRemoveBackslashS(CString& path);
CString& PathAddBackslashS(CString& path);

int FileTimeCompare(LPCTSTR fn1, LPCTSTR fn2);
bool FileExists(LPCTSTR  fname);

//Собирает список строк в строку разделяя строки символами delimetr
//Если delimAsBounds == true, то символы delimetr добавляются как спереди так и сзади строки
void StrArrayToString ( const CStringArray &lstStr, CString& str, LPCTSTR  delimetr = _T("\n"), bool delimAsBounds = false, bool TrimStr = false );

//разбивает строку S на массив подстрок, разделенные символоми delimetr
//Если delimAsBounds == true, то символы delimetr рассматриваются как поля таблицы, т.е. 
//если строка с них начинается, то пустая строка не добавляется (то же в конце).
void StringToStrArray ( LPCTSTR  str, CStringArray& lstStr, LPCTSTR  delimetr = _T("\n"), bool delimAsBounds = false, bool TrimStr = false );

void StrIntsToIntArray (LPCTSTR  strInts, CUIntArray& uintAr, LPCTSTR  delimetr = _T(","));
void IntArrayToStrInts (const CUIntArray& uintAr, CString& strInts, LPCTSTR  delimetr = _T(","));

bool StrDataGetValue(const CString& sData, CString sName, CString &sValue, LPCTSTR delimData = _T("\t"), LPCTSTR delimVal = _T("="));
void StrDataSetValue(CString& sData, LPCTSTR sName, LPCTSTR sValue, LPCTSTR delimData = _T("\t"), LPCTSTR delimVal = _T("="));

int findInStringArray(LPCTSTR  sFinded, const CStringArray &lstStr, bool bNoCase = true);
int findInStringArray(LPCTSTR  sFinded, const CString lstStr, LPCTSTR  delimetr = _T("\n"), bool bNoCase = true);

CString getFieldByName(LPCTSTR  fieldNames, LPCTSTR  fields, LPCTSTR  name, LPCTSTR  delimetr = _T("\t"));

void mtextContentsToString(AcDbMText* pMText, CString& contents);
void stringToMtextContents(CString &string, AcDbMText *pMText, bool EraseClear = true);
void mtextContentsToStrArray(AcDbMText* pMText, CStringArray& strAr);
void strArrayToMtextContents(const CStringArray &strAr, AcDbMText *pMText, bool EraseClear = true);
//Разбивает newStr на подстроки, подобные contents
void splitStringByContents(CString& newStr, const CString& contents );

int  StrFindSpace(LPCTSTR s, int minCount, int* realCount = NULL);
bool StrSplitBySpace(LPCTSTR s, CStringArray& lstStr, AcDbIntArray* startIdxs = NULL, AcDbIntArray* lenSpacesCounts = NULL);

CString GenGUID();

#endif //_INC_STRING

#endif //_STR_UTILS_H

