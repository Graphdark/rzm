#ifndef RX_UTIL_H
#define RX_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !defined(_ACAD2000) && !defined(_ACAD2004) && !defined(_ACAD2007) && !defined(_ACAD2010) && !defined(_ACAD2013) && !defined(_ACAD2015)
#pragma message("!!!! Не специфицирована версия AutoCAD !!!!")
#error Не специфицирована версия AutoCAD! Используйте #define _ACAD20##
#endif

#ifndef ACDBLIB
  #include <afx.h>
  #include "adslib.h"             // RXADS definitions
  #include "actrans.h"

  #include "AcadStr.h"
  #include "SdSysVar.h"
  #include "SdStrObjId.h"
  //#include "ArxDbgUtils.h" // используем isPaperSpase()
#endif


#ifndef ARXDBG_UTILS_H
typedef AcDbObjectPointer<AcDbObject>      AcDbObjectPtr;
typedef AcDbObjectPointer<AcDbEntity>      AcDbEntityPtr;

typedef AcDbObjectPointer<AcDbText>        AcDbTextPtr;
typedef AcDbObjectPointer<AcDbMText>       AcDbMTextPtr;

typedef AcDbObjectPointer<AcDbCircle>      AcDbCirclePtr;

typedef AcDbObjectPointer<AcDbBlockReference>   AcDbBlockReferencePtr;
typedef AcDbObjectPointer<AcDbBlockTableRecord> AcDbBlockTableRecordPtr;
typedef AcDbObjectPointer<AcDbAttributeDefinition> AcDbAttributeDefinitionPtr;

namespace ArxDbgUtils {
  bool                 isPaperSpace(AcDbDatabase* db);
  Acad::ErrorStatus    symbolIdToName(const AcDbObjectId& symbolId, CString& name);
  Acad::ErrorStatus    mlineStyleIdToName(const AcDbObjectId& mlineStyleId, CString& name);
  Acad::ErrorStatus    nameToSymbolId(AcRxClass* symTblClass, LPCTSTR name,  AcDbObjectId& symbolId, AcDbDatabase* db);
  AcDbSymbolTable*     openSymbolTable(AcRxClass* symTblClass, AcDb::OpenMode mode, AcDbDatabase* db);
  Acad::ErrorStatus    collectSymbolIds(AcRxClass* symTblClass, AcDbObjectIdArray& objIds, AcDbDatabase* db);
  inline void alertBox(LPCTSTR s) { acutPrintf(s); }
  inline void rxErrorMsg(Acad::ErrorStatus msgId)
  {
    if (msgId != Acad::eOk)
      acutPrintf(_T("\nARX ERROR: %s"), acadErrorStatusText(msgId));
  }
};

inline void
copyAdsPt(ads_point pt1, ads_point pt2)
{
  pt1[0] = pt2[0];
  pt1[1] = pt2[1];
  pt1[2] = pt2[2];
}

/////////////////////////////////////////////////////////////////////

inline void
copyEname(ads_name ename1, ads_name ename2)
{
  ename1[0] = ename2[0];
  ename1[1] = ename2[1];
}
#endif



#include "acgi.h"
#include "gept3dar.h"
#include "dbents.h"
#include "dbapserv.h"

//#ifndef ACHAR
//#define ACHAR TCHAR
//#endif

#ifndef EXPORTPROC
  #ifdef _DLLEXPORT
    #define EXPORTPROC(type) __declspec( dllexport ) type
  #else
    #define EXPORTPROC(type) type
  #endif
#endif

#ifndef ZERO
#define ZERO    1e-10
#endif

#ifndef PI
#define PI      ((double)3.14159265358979323846)
#endif

#ifndef HALFPI
#define HALFPI PI/2.0
#endif

#ifndef SIN45
#define SIN45  0.7071067811865
#endif

#ifndef WORKDWG
#define WORKDWG acdbHostApplicationServices()->workingDatabase()
#endif

#ifndef OSMODEP_OFF
#define OSMODEP_OFF  0x4000
#endif

#ifndef MAX_SNAP_OBJECTS
#define MAX_SNAP_OBJECTS 128
#endif


//---------------------------------------------------------------------------
#ifndef DEF_TOL
#define DEF_TOL 0.0001
#endif
// math methods
inline bool Equ(double r1, double r2, double rTolerance = DEF_TOL)
{
  return (fabs(r2-r1) <= rTolerance) ? true : false;
}

inline double Round(double rVal, double rTolerance = DEF_TOL)
{
  if(rVal > 0)
    return (double)((__int64)(rVal/rTolerance+0.500001))*rTolerance;
  else
    return (double)((__int64)(rVal/rTolerance-0.500001))*rTolerance;
}
//---------------------------------------------------------------------------

#define MAKEUCSCOORD(x, y)      tmpPoint = (y); \
acdbWcs2Ecs(asDblArray(tmpPoint), asDblArray(tmpPoint), asDblArray(m_vecUnitZ),\
Adesk::kFalse); x[X] = tmpPoint[X]; x[Y] = tmpPoint[Y];

//----- Disable the compiler warning complaining
//----- about the ignored throw specification
#if _MSC_VER <= 1200
#pragma warning (disable:4290)
#endif

#if /*(_MSC_VER >= 1300) &&*/ (_MSC_VER < 1500) 
#if (defined _ACAD2000) || (defined _ACAD2004)
//VC7 or 7.1, building with pre-VC7 runtime libraries
extern "C" long _ftol( double ); //defined by VC6 C libs
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }
  #ifndef _tstol
    #define _tstol      atol
  #endif
  #ifndef _tstoi
    #define _tstoi      atoi
  #endif
  #ifndef _tstof
    #define _tstof      atof
  #endif
#endif
#endif


#ifndef ACDBLIB


#if (_MSC_VER >= 1300) && (_MSC_VER < 1400) && (WINVER < 0x0500)
//VC7 or 7.1, building with pre-VC7 runtime libraries

extern "C" HWND adsw_acadMainWnd();
extern "C" const TCHAR*  acadErrorStatusText(Acad::ErrorStatus);

#endif

//Недокументированные ф-ции Acad 
extern "C" BOOL AcadOnIdle(void); //возвращает TRUE если AutoCAD находится в процессе простоя
extern bool AcadIsQuitting(void); //возвращает TRUE если AutoCAD находится в процессе закрытия

extern void ads_regen(void);      //выполняет регенерацию всего чертежа.

#ifdef _NCAD_BUILD_
extern long acdbSetDbmod(class NcDbDatabase* db, long lParam);
#else
//extern "C" long __cdecl acdbSetDbmod(class AcDbDatabase *db,long new_dbmod);
//__declspec(dllimport) long acdbSetDbmod(class AcDbDatabase* db, long val);
extern Adesk::Int32 acdbSetDbmod(AcDbDatabase *db, Adesk::Int32 new_dbmod);
#endif

Acad::ErrorStatus RTtoStatus (int rt);

inline void ADSOK(int what)/* throw(Acad::ErrorStatus)*/
{
    Acad::ErrorStatus es=RTtoStatus(what);
    if (es!=Acad::eOk) 
        throw es;
}
#endif //ACDBLIB
 
inline void ARXOK(Acad::ErrorStatus what)
{
    if (what!=Acad::eOk) 
        throw what;
}

#ifndef ACDBLIB
#define MESSAGE_ES(x) acadErrorStatusText(x)
#else
#define MESSAGE_ES(x) NULL
#endif //ACDBLIB

#ifdef _DEBUG
#define AC_FAILRU  acutPrintf(_T("(%s@%d)"), _T(__FILE__), __LINE__);
#else
#define AC_FAILRU  ;
#endif // _DEBUG

#define PRINT_SPACE acutPrintf(_T(" "));
#define PRINT_EMPTY acutPrintf(_T(""));

#ifndef ARXDBG_UTILS_H 
  #ifdef _NCAD_BUILD_
    #define PRINT_CARET acutPrintf(_T("\n"));
  #else
    #define PRINT_CARET ;
  #endif
#define CARETCOLON(s) (s)
#else
#define PRINT_CARET ArxDbgUtils::printCaret();
#define CARETCOLON(s) ArxDbgUtils::BegCaretAndEndColonString(s)
#endif // ARXDBG_UTILS_H

#define PRINT_NODEFINEDEXCEPTION { PRINT_CARET acutPrintf(_LCT("Неопределённое исключение!")); AC_FAILRU }
#define PRINT_EXCEPTIONSTRING(s) { if (s && *s ) { PRINT_CARET acutPrintf(_LCT("Исключение:%s"), s); AC_FAILRU } }

#define CATCH_STR       catch (LPCTSTR s)             { if (s && *s ) { PRINT_CARET acutPrintf(s);} }
#define CATCH_STRBOX    catch (LPCTSTR s)             { if (s && *s ) ArxDbgUtils::alertBox(s); }
#define CATCH_STRING    catch (CString s)            { if (!s.IsEmpty()) { PRINT_CARET acutPrintf(s); } }
#define CATCH_STRINGBOX catch (CString s)            { if (!s.IsEmpty()) ArxDbgUtils::alertBox(s); }
#define CATCH_STREXCEPT catch (LPCTSTR s)            { PRINT_EXCEPTIONSTRING(s) }
#define CATCH_ERRSTATUS catch (Acad::ErrorStatus es) { PRINT_EXCEPTIONSTRING(MESSAGE_ES(es)); ASSERT(es == Acad::eOk); }
#define CATCH_EXCEPT    catch (CException *e)        { rx_pushException( e, 1 ); AC_FAILRU }
#define CATCH_COMERROR  catch (_com_error cerr)      { PRINT_EXCEPTIONSTRING (cerr.ErrorMessage()) }
#if defined(_NCAD_BUILD) && defined(_NCAD_USE_EXTNRX)
#define CATCH_ODRESULT  catch (OdResult odr)         { PRINT_EXCEPTIONSTRING((LPCTSTR)OdError(odr).description()); ASSERT(odr == eOk); }
#define CATCH_ODERROR   catch (OdError err)          { PRINT_EXCEPTIONSTRING((LPCTSTR)err.description()); ASSERT(err.code() == eOk); }
#else
#define CATCH_ODRESULT
#define CATCH_ODERROR
#endif
#define CATCH_OTHER     catch (...)                  { PRINT_NODEFINEDEXCEPTION; ASSERT(FALSE); }
#define CATCH_BLOCKALL  \
  CATCH_ODRESULT  \
  CATCH_ODERROR   \
  CATCH_STR       \
  CATCH_STRING    \
  CATCH_ERRSTATUS \
  CATCH_COMERROR  \
  CATCH_EXCEPT    \
  CATCH_OTHER     

//#define CATH_COMBLOCKALL \
//  CATCH_COMERROR \
//  CATCH_BLOCKALL



#ifdef _UNICODE
#define S_STATUS_ERROR   "!%ls@%d: %ls"
#define S_STATUS_ERROR_P "%ls@%d->%ls"
#else // Not UNICODE
#define S_STATUS_ERROR   "!%s@%d: %s"
#define S_STATUS_ERROR_P "%s@%d->%s"
#endif //UNICODE

//#define PRINT_STATUS_ERROR(x) { \
//	TCHAR errorBuffer[256];	\
//	_stprintf(errorBuffer, _T("!%s@%d: %s"), _T(__FILE__), __LINE__, MESSAGE_ES(x)); \
//  acdbFail(errorBuffer); \
//	}

#define PRINT_STATUS_ERROR(x)  acutPrintf(_T(S_STATUS_ERROR), _T(__FILE__), __LINE__, MESSAGE_ES(x));
	

#if defined(_DEBUG) || defined(_FULLDEBUG_) || defined(_DEBUG_WAS_DEFINED)
#define PRINT_DBG_STATUS_ERROR(x) { \
      TCHAR errorBuffer[256];	\
      _stprintf(errorBuffer, _T(S_STATUS_ERROR), _T(__FILE__), __LINE__, (TCHAR*)MESSAGE_ES(x)); \
      acdbFail(errorBuffer); \
	}
#else
  #define PRINT_DBG_STATUS_ERROR(x) ((void)0)
#endif //ACDBLIB

#define INTERNAL_STATUS_ERROR(x)  acrx_abort(_T(S_STATUS_ERROR_P), _T(__FILE__), __LINE__, (TCHAR*)MESSAGE_ES(x))


#ifndef ACDBLIB
#define RETURN_STATUS_ERROR(x) { \
	TCHAR errorBuffer[256];	\
	_stprintf(errorBuffer, _T(S_STATUS_ERROR_P), _T(__FILE__), __LINE__, (TCHAR*)MESSAGE_ES(x)); \
	acedAlert(errorBuffer); \
	return x; }
#else //ACDBLIB
#define RETURN_STATUS_ERROR(x) { \
	TCHAR errorBuffer[256];	\
	_stprintf(errorBuffer, _T(S_STATUS_ERROR_P),_T(__FILE__), __LINE__, (TCHAR*)MESSAGE_ES(x)); \
  acdbFail(errorBuffer); \
	return x; }
#endif //ACDBLIB


#define AOK(what)  {Acad::ErrorStatus stat = what; if (stat != Acad::eOk) INTERNAL_STATUS_ERROR(stat); }
#define ISOK(what) {Acad::ErrorStatus stat = what; if (stat != Acad::eOk) RETURN_STATUS_ERROR(stat);   }
#define ISOKv(what) {Acad::ErrorStatus stat = what; if (stat != Acad::eOk) { PRINT_STATUS_ERROR(stat); return; } }

#define DBGSHOWMSG { \
TCHAR errorBuffer[256];	\
  _stprintf(errorBuffer, _T(S_STATUS_ERROR_P), _T(__FILE__), __LINE__, _T("DebugMsg")); \
  acedAlert(errorBuffer); \
}

inline void rxPrint(const TCHAR* lpszFormat, ...)
{
  TCHAR szBuffer[1024];
  memset(szBuffer, 0, sizeof(szBuffer));
  va_list args;
  va_start(args, lpszFormat);
  if (_vsntprintf(szBuffer, sizeof(szBuffer) - 1, lpszFormat, args) > 0)
  {
#ifndef _NCAD_BUILD_
    acutPrintf(_T("\n"));
#endif
    acutPrintf(_T("(T) %s"), szBuffer);
  }
  va_end(args);
}

#define rxPRINT ::rxPrint


#ifdef _DEBUG 
#define DEBUG_RXPRINT
#else
#define DEBUG_RXPRINT
#endif


#if defined(DEBUG_RXPRINT) 

extern int rx_iRXPRINT__MODE__;

inline void rxDbgPrint(const TCHAR* lpszFormat, ...)
{
  if (!rx_iRXPRINT__MODE__) return;

  TCHAR szBuffer[1024];
  memset(szBuffer, 0, sizeof(szBuffer));
  va_list args;
  va_start(args, lpszFormat);
  if (_vsntprintf(szBuffer, sizeof(szBuffer)-1, lpszFormat, args) > 0)
  {
#ifndef _NCAD_BUILD_
    acutPrintf(_T("\n"));
#endif
    acutPrintf(_T("_D(%s)"), szBuffer);
  }
  va_end(args);
}

inline void rxDbgPrintAr(const AcDbIntArray intAr, LPCTSTR  s = NULL)
{
  if (!rx_iRXPRINT__MODE__) return;

  CString st, msg;
  if (s)
  {
    st.Format(_T(":%s:"), s);
    msg += st;
  }
  for (int i=0; i < intAr.length(); i++)
  {
    st.Format(_T(" %i"), intAr[i]);
    msg += st;
  }
  rxDbgPrint(msg);
}

inline void rxDbgPrintAr(const AcGePoint3dArray& ptAr, LPCTSTR  s = NULL)
{
  if (!rx_iRXPRINT__MODE__) return;

  CString st, msg;
  if (s)
  {
    st.Format(_T(":%s:"), s);
    msg += st;
  }
  msg += _T("(");
  for (int i=0; i < ptAr.length(); i++)
  {
    st.Format(_T("[%i](%2.f,%2.f,%2.f) "), i, ptAr[i].x, ptAr[i].y, ptAr[i].z);
    msg += st;
  }
  rxDbgPrint(msg);
}

inline void rxDbgPrintVec(const AcGeVector3d& v, LPCTSTR  s = _T(""))
{
  if (!rx_iRXPRINT__MODE__) return;
  acutPrintf(_T("%s(%.2f, %.2f, %.2f)"), s, v.x, v.y, v.z);
}

inline void rxDbgPrintPt(const AcGePoint3d& v, LPCTSTR  s = _T(""))
{
  if (!rx_iRXPRINT__MODE__) return;
  acutPrintf(_T("%s(%.2f, %.2f, %.2f)"), s, v.x, v.y, v.z);
}

//#ifdef ASSERT
//#undef ASSERT
//#define ASSERT RXASSERT
//#endif

//#define mAssert0(x, s)    _ASSERT(x); _RPT1(_CRT_WARN, _T("\n \n %s\n"), s)
 #define rxDPRINT      ::rxDbgPrint
 #define rxDPRINTAR    ::rxDbgPrintAr
 #define rxDPRINTPT    ::rxDbgPrintPt
 #define rxDPRINTVEC   ::rxDbgPrintVec
 #define rxASSERT0(x, s)    if(!(x)) { PRINT_CARET  acutPrintf(_LCT("Ошибка: %s"), s); RXASSERT(x);} 
#define rxIfAcASSERT(x, s)  {Acad::ErrorStatus stat = x; if (stat != Acad::eOk){ PRINT_CARET acutPrintf(_LCT("Ошибка %s: %s"), MESSAGE_ES(stat), s );}} \
                            if((x)!=Acad::eOk)
#else 
 #define rxDPRINT
 #define rxDPRINTAR
 #define rxDPRINTPT
 #define rxDPRINTVEC
//((void)0)
 #define rxASSERT0(x, s)     if(!(x)) acutPrintf(_T("\nRZM* Ошибка: %s"), s ) 
 #define rxIfAcASSERT(x, s)  if((x)!=Acad::eOk) 
#endif

#ifdef __AFX_H__
void rx_pushException(CException* e, int msgLevel = 1, LPCTSTR  nameFunction = NULL);
#endif __AFX_H__

Acad::ErrorStatus getNOD (AcDbDictionary *&pNOD, const AcDb::OpenMode mode);


__inline bool IsKeyDownControl() {return (GetKeyState(VK_CONTROL)&0x8000)==0x8000;};
__inline bool IsKeyDownShift() {return (GetKeyState(VK_SHIFT)&0x8000)==0x8000;};
__inline bool IsKeyDownAlt() {return (GetKeyState(VK_MENU)&0x8000)==0x8000;};


inline bool intToBool(int intBoolVal)
{
	return (intBoolVal == 0) ? false : true;
}

inline int boolToInt(bool boolVal)
{
	return (boolVal == false) ? 0 : 1;
}

inline int boolToIntA(Adesk::Boolean boolVal)
{
  return (boolVal == Adesk::kFalse) ? 0 : 1;
}

inline Adesk::Boolean intToBoolA(int boolVal)
{
  return (boolVal == 0) ? Adesk::kFalse : Adesk::kTrue;
}


inline double& rx_normalize(double& d)
{
  double tol = AcGeContext::gTol.equalPoint();
  if ( 0.0 - tol < d && d <  0.0 + tol) d =  0.0;
  else
  if ( 1.0 - tol < d && d <  1.0 + tol) d =  1.0;
  else
  if (-1.0 - tol < d && d < -1.0 + tol) d = -1.0;
  return d;
}

inline AcGePoint3d& rx_normalize(AcGePoint3d& pt)
{
  rx_normalize(pt.x);
  rx_normalize(pt.y);
  rx_normalize(pt.z);
  return pt;
}

inline AcGeVector3d& rx_normalize(AcGeVector3d& v)
{
  rx_normalize(v.x);
  rx_normalize(v.y);
  rx_normalize(v.z);
  return v;
}

double  rx_fixangle(double angle);
void    rx_fixindex(int& index, int maxIndex);

double  rx_roundByStep ( double value, double step, double minToMin = 0.1);
AcGePoint3d rx_roundByStep ( const AcGePoint3d &pt, double step, double minToMin = 0.1 );

double  rx_round( double value,  int numdecimalplaces = 2);
AcGePoint3d rx_round(const AcGePoint3d &pt, int numdecimalplaces = 2);
AcGeVector3d rx_round(const AcGeVector3d &v, int numdecimalplaces = 2);

double rx_roundAngleByAxisXY(double ang);
AcGeVector3d rx_roundAxisXY(double ang);
AcGeVector3d rx_roundAxisXY(const AcGeVector3d &v);

struct resbuf* rx_nthRb (int pos, struct resbuf* pRb, struct resbuf* &resRb);
struct resbuf* rx_assocRb (struct resbuf* pRb, int restype);

bool rx_univec(AcGeVector3d& ap, AcGeVector3d& bp);

// Given the db id of an AcDbTextStyleTableRecord, construct
// an AcGiTextStyle structure out of that text style.
Acad::ErrorStatus rx_getTextStyle(AcGiTextStyle &ts, const AcDbObjectId& styleId);
Acad::ErrorStatus rx_getTextStyleId(LPCTSTR styleName, AcDbObjectId &styleId, AcDbDatabase* pDwg =NULL);
Acad::ErrorStatus	rx_getTextStyleName(const AcDbObjectId& styleId, LPCTSTR styleName);

Acad::ErrorStatus rx_getLayerId(LPCTSTR pStr, AcDbObjectId& layerId, bool getErasedRecord = false, AcDbDatabase* pDwg = NULL);
bool rx_isAvailableLayer(LPCTSTR pStr, bool mess = true, AcDbDatabase* db = NULL);
bool rx_isAvailableLayer(const AcDbObjectId& layerId, bool mess = true);

bool rx_isOffLayer(const AcDbObjectId& layerId);
bool rx_isOffLayer(LPCTSTR  pStr, bool mess =true, AcDbDatabase* db = NULL);
Acad::ErrorStatus rx_setOffLayer(const AcDbObjectId& layerId, bool bOff = true);
Acad::ErrorStatus rx_setOffLayer(LPCTSTR pStr, bool bOff = true, AcDbDatabase* db = NULL);
Acad::ErrorStatus rx_setOffLayers(bool bOff, int layerCount, ...);

AcGePoint3d rx_pt2Dto3D (const AcGePoint2d& p);
AcGePoint2d rx_pt3Dto2D (const AcGePoint3d& p);

void rx_drawLine(AcGiGeometry& g, const AcGePoint3d& p1, const AcGePoint3d& p2);
void rx_drawRect(AcGiGeometry& g, const AcGePoint3d& p1, const AcGePoint3d& p2);
void rx_drawBox (AcGiGeometry& g, const AcGePoint3d* pVerts, const AcGeVector3d* pViewDir = NULL);
void rx_drawX(AcGiGeometry& g, const AcGePoint3d& pt, double r, bool bAsPlus = false, bool b3d = false);

void rx_extentsArray(const AcDbObjectIdArray& objIdAr, AcDbExtents &extents, bool bBestFitBlk = false);
bool rx_extentsBlkRef(const AcDbObjectId &objId, AcDbExtents &extents, bool offAttr = true);

bool rx_eraseEntity(const AcDbObjectId& objId, bool unlockLayer = false);
void rx_eraseEntities(AcDbObjectIdArray& objIdAr, bool unlockLayer = false);

void rx_boxPoints(const AcGePoint3d& pt1, const AcGePoint3d& pt2, 
                  AcGePoint3dArray& ptArray, double dH, double dL);

bool rx_textBox(LPCTSTR  pStr,
                const AcGiTextStyle &textStyle, 
                AcGePoint3d& ptMin,
                AcGePoint3d& ptMax,
                AcGiWorldDraw* ctxt = NULL);
bool rx_textBox(AcDbText* pText,
                AcGePoint2d& ext,
                AcGiWorldDraw* ctxt = NULL);

double rx_pdSize(AcDbDatabase* pDb = NULL, double percentageOfViewsize = 0.1);

// rx_utilMyEnt.cpp
bool rx_pointNearEntities(const AcDbObjectIdArray& objIdAr, AcGePoint3d& pt);
bool rx_pointNearEntities(const AcDbObjectId& objId, AcGePoint3d& pt);
Acad::ErrorStatus rx_getEntityCenter(const AcDbEntity* pEnt, AcGePoint3d& pt, double dMin = 1.0);
Acad::ErrorStatus rx_getEntityPosition(const AcDbEntity* pEnt, AcGePoint3d& pt);
Acad::ErrorStatus rx_setEntityPosition(AcDbEntity* pEnt, const AcGePoint3d& pt);
Acad::ErrorStatus rx_getEntityRotation(const AcDbEntity* pEnt, double& ang);
Acad::ErrorStatus rx_setEntityRotation(AcDbEntity* pEnt, const double& ang);
//------Работают с AcDbText и AcDbMText -------
bool rx_textPosByAlignmentPoint(AcDbText* pText);
Acad::ErrorStatus rx_getTextPosition(const AcDbEntity* pEnt, AcGePoint3d& pt);
Acad::ErrorStatus rx_setTextPosition(AcDbEntity* pEnt, const AcGePoint3d& pt);
Acad::ErrorStatus rx_getTextRotation(AcDbEntity* pEnt, double& angle);
Acad::ErrorStatus rx_setTextRotation(AcDbEntity* pEnt, double angle);
Acad::ErrorStatus rx_getTextHeight(AcDbEntity* pEnt, double& height);
Acad::ErrorStatus rx_setTextHeight(AcDbEntity* pEnt, double height);

#ifndef ACDBLIB

bool rx_textBox ( LPCTSTR  text,
                  const double &height,
                  const AcGiTextStyle &textStyle, 
                  AcGePoint3d& ptMin,
                  AcGePoint3d& ptMax);

bool rx_textBox ( LPCTSTR  text,
                  const double &height,
                  const  AcDbObjectId &styleId,
                  AcGePoint3d& ptMin,
                  AcGePoint3d& ptMax);

bool rx_textBox(AcGePoint3d *ptMin, AcGePoint3d *ptMax,
                const TCHAR* text, 
                const AcDbObjectId& styleId,
                const double height,
                const double widthFactor, 
                const double obliquingAngle = 0.0,
                const AcGePoint3d& pt = AcGePoint3d::kOrigin,
                const double rotation = 0.0, 
                const AcDb::TextHorzMode hMode = AcDb::kTextLeft,
                const AcDb::TextVertMode vMode = AcDb::kTextBase,
                const AcGePoint3d& ptAlign = AcGePoint3d::kOrigin );

//bool rx_textRect (ads_name tname, AcGePoint3dArray& verts );
bool rx_textRect(const  TCHAR* text, 
                 const  AcGePoint3d& pt,
                 const  double &height,
                 const  AcGeVector3d& normal,
                 const  double &ang, 
                 const  AcDbObjectId &styleId,
                 AcGePoint3dArray& verts );


Acad::ErrorStatus getCurrentSpace(AcDbBlockTableRecord*& pSpaceRecord, AcDbDatabase *pDb = NULL);
Acad::ErrorStatus addToDb(AcDbEntity* ent);
Acad::ErrorStatus postToDb(AcDbEntity* ent);
Acad::ErrorStatus addToDb(AcDbEntity* ent, AcDbObjectId& objId, AcDbDatabase *pDb = NULL);
Acad::ErrorStatus postToDb(AcDbEntity* ent, AcDbObjectId& objId, AcDbDatabase *pDb = NULL);

Acad::ErrorStatus rx_scanPline(AcDb2dPolyline*    pline,
                               AcGePoint3dArray&  points,
                               AcGeDoubleArray&   bulges);

Acad::ErrorStatus rx_scanPline(AcDb3dPolyline*    pline, AcGePoint3dArray&  points);

Acad::ErrorStatus rx_setVertexPline(AcDbPolyline *pPLine, const AcGePoint3dArray &ptAr, bool bClosed);


Acad::ErrorStatus rx_makeArc(const AcGePoint3d    pt1,
                             const AcGePoint3d    pt2, 
                             double               bulge,
                             AcGeCircArc3d& arc);

Acad::ErrorStatus rx_makeArc(const AcGePoint3d    pt1, 
                             const AcGePoint3d    pt2,
                             double               bulge,
                             const AcGeVector3d   entNorm,
                             AcGeCircArc3d& arc);

Acad::ErrorStatus rx_makePline(const AcGePoint3dArray&  pts,
                               bool closed = false,
                               int colorIndex = 256,
                               AcDbObjectId *pObjId = NULL);


Acad::ErrorStatus rx_makeSpline(const AcGePoint3dArray& pts, AcDbSpline*& pSpline);

Acad::ErrorStatus rx_makeLine(const AcGePoint3d& startPt, const AcGePoint3d& endPt, 
                              int colorIndex = 256, bool HighLight = false,
                              AcDbObjectId *pObjId =NULL);

Acad::ErrorStatus rx_makeText(AcGePoint3d pt, 
                              LPCTSTR  text,
                              double height, 
                              double ang = 0.0, 
                              AcDb::TextHorzMode hmode = AcDb::kTextLeft, 
                              AcDb::TextVertMode vmode = AcDb::kTextBase,
                              bool italic = false,
                              LPCTSTR  layerName = NULL,
                              int colorIndex = 256,
                              bool HighLight = false,
                              AcDbObjectId *pObjId = NULL );

Acad::ErrorStatus rx_makeAttDef(AcGePoint3d pt, 
                              LPCTSTR  tag,
                              LPCTSTR  prompt,
                              LPCTSTR  text,
                              double height, 
                              double ang  = 0.0, 
                              AcDb::TextHorzMode hmode = AcDb::kTextLeft, 
                              AcDb::TextVertMode vmode = AcDb::kTextBase,
                              bool italic = false,
                              LPCTSTR  layerName = NULL,
                              int colorIndex = 256,
                              bool HighLight =false,
                              AcDbObjectId *objId =NULL);

Acad::ErrorStatus rx_textToAttDef(AcDbObjectId& textId,
                                LPCTSTR  tag,
                                LPCTSTR  prompt);
Acad::ErrorStatus rx_cloneText(AcDbText* pText, AcDbObjectId& newObjId);

void rx_setTextWidthFactorByMaxWidth(AcDbText* pText, double maxWidth, double defWidthFactor = 0.0);

Acad::ErrorStatus rx_textToMText(AcDbObjectId& textId);
Acad::ErrorStatus rx_setMTextByText(AcDbMText *pMText, const AcDbText *pText);
Acad::ErrorStatus rx_mtextToText(AcDbObjectId& textId);
Acad::ErrorStatus rx_setTextByMText(AcDbText *pText, const AcDbMText *pMText);

//------Работают с AcDbText и AcDbMText ------------------------------------------------------------------------------
Acad::ErrorStatus rx_getTextString(AcDbEntity *pEnt, CString& text);
Acad::ErrorStatus rx_setTextString(AcDbEntity *pEnt, CString text);
Acad::ErrorStatus rx_getTextStyle(AcDbEntity *pEnt, double height);
Acad::ErrorStatus rx_setTextStyleName(AcDbEntity *pEnt, LPCTSTR styleName, AcDbDatabase* db = NULL);
Acad::ErrorStatus rx_getTextStyleName(AcDbEntity *pEnt, CString& styleName);
Acad::ErrorStatus rx_updTextByStyle(AcDbText *pText);

CString rx_mtextContentToTextLine(LPCTSTR contents);

void rx_attachmentByAlign(AcDb::TextHorzMode hmode, AcDb::TextVertMode vmode, AcDbMText::AttachmentPoint& attPoint);
void rx_alignByAttachment(AcDbMText::AttachmentPoint attPoint, AcDb::TextHorzMode& hmode, AcDb::TextVertMode& vmode);
Acad::ErrorStatus rx_getTextAttachment(const AcDbEntity *pEnt, AcDbMText::AttachmentPoint& attPoint);
Acad::ErrorStatus rx_setTextAttachment(AcDbEntity *pEnt, AcDbMText::AttachmentPoint attPoint);
CString rx_textAttachmentString(AcDbMText::AttachmentPoint attPoint);
//----------------------------------------------------------------------------------------------------------------------

Acad::ErrorStatus
getUniformKnots(int numCtrlPts, int degree, int form, AcGeDoubleArray& knots);

bool  univec(AcGeVector3d& ap, AcGeVector3d& bp);

int rx_findEndPoint(ads_point oldpt, ads_point newpt);

int getcYesOrNo(LPCTSTR  promptStr, bool& answer, int init = 1);
bool getPoint3d(LPCTSTR  prompt, AcGePoint3d& pt, AcGePoint3d* pFrom = NULL,
                int snapmode = -1, bool waitCan = true, bool bUseDashedLine = true, int *pRet = NULL);
bool rx_getCorner(LPCTSTR  prompt, AcGePoint3d& pt1, AcGePoint3d& pt2, int snapmode = -1, 
                  bool useStartPoint =false, int fInitGet = RSG_NONULL|RSG_DASH, int *pRet = NULL);

void rx_splitText(LPCTSTR  str, double width, CStringArray &resArray,
                  double height, LPCTSTR nameStyle, double scale, double obliquingAngle);

int SSGetByPoint(LPCTSTR  msg, AcGePoint3d &pt, ads_name &sset, Adesk::Int32 &slen, int snapmode = -1 );
int SSGetByPoint(LPCTSTR  msg, AcGePoint3d &pt, AcDbObjectIdArray &objIdAr, int snapmode = -1 );
int SSGetByPick(const struct resbuf* filter, ads_name& sset, Adesk::Int32& slen);

AcDbEntity*	selectNEntity(LPCTSTR  msg, AcDb::OpenMode openMode, AcGePoint3d &pt, int pickflag = FALSE, int snapmode = -1);
AcDbObjectId selectEntity();
AcDbEntity* selectEntity(LPCTSTR  msg, AcDb::OpenMode openMode);
AcDbEntity* selectEntity(AcDbObjectId& eId, LPCTSTR  msg, AcDb::OpenMode openMode);
AcDbEntity* selectEntity(AcDbObjectId& eId, LPCTSTR  msg, AcDb::OpenMode openMode, AcGePoint3d &pt, int snapmode = -1);
AcDbEntity* selectEntityPoint(AcDbObjectId& eId, LPCTSTR , AcDb::OpenMode openMode, AcGePoint3d &pt, int snapmode = -1);
AcDbEntity* getOneEntityInPoint(const AcGePoint3d &pt, AcDb::OpenMode openMode);


int rx_acadColorDlg (short *color , bool IncLogic = true);
bool rx_acedCurGraphWndBackColor(AcCmEntityColor& backcolor, AcDbDatabase* pDwg = NULL);


#ifndef _ACAD2000
void rx_DrawOrderInherit(const AcDbObjectId& objId, AcEdDrawOrderCmdType cmd = kDrawOrderBottom);
#else
void rx_DrawOrderInherit(const AcDbObjectId& objId, int cmd = 0);
#endif



Acad::ErrorStatus  rx_createLayer(/*[in]*/LPCTSTR  layerName, /*[out]*/ AcDbObjectId& id);
bool rx_isLockCurrentLayer(bool alert = true);

Acad::ErrorStatus rx_updLayerColor(LPCTSTR  layerName, int colorIndex, int* upd = NULL, AcDbDatabase* pDwg = NULL);
Acad::ErrorStatus rx_updLayerPlottable(LPCTSTR  layerName, bool bPlottable, int* upd = NULL, AcDbDatabase* pDwg = NULL);

Acad::ErrorStatus rx_updTextStyle(LPCTSTR textStyleName, 
                                  LPCTSTR fontFile = NULL, 
                                  double textSize = -1,
                                  double xScale = -1,
                                  double obliquingAngle = -1,
                                  AcDbDatabase* pDwg =NULL,
                                  bool *pIsChange = NULL);
Acad::ErrorStatus rx_setHeightTextStyle(const AcDbObjectId &textStyleId, double Height);
Acad::ErrorStatus rx_getHeightTextStyle(const AcDbObjectId &textStyleId, double &Height);


struct RXEnumProc
{
  typedef enum
  {
    eEnumCancel = 0,
    eEnumContinue,
    eEnumAdd
  } EnumCode;

  typedef enum
  {
    eCurrentSpace,
    eModelSpace,
    ePaperSpace,
    eModelAndPaperSpace
  } EnumArea;

  typedef EnumCode (*EnumProc)(const AcDbObjectId& objId, LPVOID param);
};

UINT rx_EnumerateEntityesBTR(AcDbBlockTableRecord* pBTR, RXEnumProc::EnumProc pEnumProc,
                             LPVOID param = NULL, LPCTSTR  messProgress = NULL);
UINT rx_EnumerateEntityes(RXEnumProc::EnumProc pEnumProc,
                          LPVOID param = NULL, AcDbDatabase* pDb=NULL, 
                          RXEnumProc::EnumArea where = RXEnumProc::eCurrentSpace, LPCTSTR  messProgress = NULL);

UINT rx_collectEntites(AcDbBlockTableRecord *pBTR, AcDbObjectIdArray& objIds, 
                      AcRxClass* rxClass = NULL, bool bEqClass = false);
UINT rx_collectEntites(AcDbObjectIdArray& objIds, 
                      AcRxClass* rxClass = NULL,
                      bool bEqClass = false,
                      RXEnumProc::EnumArea where = RXEnumProc::eCurrentSpace,
                      AcDbDatabase* pDb = NULL);
bool rx_collectEntites(AcDbObjectIdArray& objList, const AcDbObjectId& begObjId, bool ExludeFirst = false, bool OnlyMain = true);
//bool rx_collectEntites(AcDbObjectIdArray& objList, const ads_name beg_aname,     bool ExludeFirst = false, bool OnlyMain = true);
bool rx_lastId(AcDbObjectId& objId);

void rx_collectRemoveNoReadyIds(AcDbObjectIdArray& objIdAr);
void rx_collectRemoveNoCastIds(AcDbObjectIdArray& objIdAr, AcRxClass* rxClass = NULL, bool bEqClass = false );

Acad::ErrorStatus rx_collectAttributes(const AcDbBlockReference* blkRef, AcDbObjectIdArray& subEnts, LPCTSTR  tag = NULL, int len=0);
Acad::ErrorStatus rx_collectAttributes(const AcDbObjectId &blkRefId, AcDbObjectIdArray& subEnts, LPCTSTR  tag = NULL, int len=0);

Acad::ErrorStatus rx_getBlockAttDefId(const AcDbAttribute* pAttr, AcDbObjectId &attId);

Acad::ErrorStatus rx_getBlockAttrId(const AcDbObject* blockRef, LPCTSTR  tag, AcDbObjectId &attId, bool openErased = false);
Acad::ErrorStatus rx_getBlockAttrId(const AcDbObjectId blockId, LPCTSTR  tag, AcDbObjectId &attId, bool openErased = false);

Acad::ErrorStatus rx_setAttrValue(const AcDbObjectId blockId, LPCTSTR  name, LPCTSTR  strVal, int len=0 );
Acad::ErrorStatus rx_getAttrValue(const AcDbObjectId blockId, LPCTSTR  name, LPTSTR &strVal );

bool rx_getTransformBlkRef(const AcDbObjectId &blkRefId, AcGeMatrix3d& blockTransform);

void rx_redrawTr(AcDbEntity *pEnt = NULL);
void rx_redrawTr(const AcDbObjectId& objId);
void rx_redrawTr(const AcDbObjectIdArray& objIdAr);

void rx_refreshDisplay();
void rx_redraw(const AcDbObjectIdArray& objIdAr);
void rx_redraw(resbuf* filter);

bool rx_isCurDocQuiescent();
int  rx_setPickfirst(const AcDbObjectIdArray& objIdAr);
int  rx_getPickfirst(AcDbObjectIdArray& objIdAr);
void rx_clearPickFirst();
void rx_clearUndoCmd();
void rx_clearUndoCmdSend();

int rx_commandCmd(LPCTSTR cmd, bool bCmdEchoOff = true);

// cmd active flags
#define CMDSTATE_REGULAR      0x00001
#define CMDSTATE_TRANSPARENT  0x00002
#define CMDSTATE_SCRIPT       0x00004
#define CMDSTATE_DIALOG       0x00008
#define CMDSTATE_DDE          0x00010
#define CMDSTATE_LISP         0x00020
#define CMDSTATE_ARX          0x00040
// additional flags
#define CMDSTATE_MENU         0x01000 //
#define CMDSTATE_MANUAL       0x02000 // this bit is set when it looks like a human invoked the command

int rx_getCurrentCommandStateFlags();
int rx_setCurrentCommandStateFlagsAppend(int flag);

int rx_commandFromStack(LPCTSTR cmd);


//bool rx_addBlockReference(const AcDbObjectId& blockId,
//                          double angle = 0.0, 
//                          const AcGePoint3d *insertPoint = NULL);
bool rx_exlode(AcDbEntity* pEnt, bool bErase = true, AcDbObjectIdArray* pObjIdAr = NULL, AcDbDatabase *pDb = NULL);
bool rx_explodeCmd(const AcDbObjectId& objId, 
                   AcDbObjectIdArray* pObjIdAr = NULL, bool eraseBlkRef = false, bool eraseBlkDef = false, bool bCmdEchoOff =false);
bool rx_explodeCmd(ads_name en, 
                   AcDbObjectIdArray* pObjIdAr = NULL, bool eraseBlkRef = false, bool eraseBlkDef = false, bool bCmdEchoOff =false);
Acad::ErrorStatus rx_explodeBlkRef(const AcDbObjectId& blkRefId,
                                   AcDbObjectIdArray* pObjIdAr = NULL, bool eraseBlkRef = false, bool eraseBlkDef = false);

void rx_ident_init(ads_matrix id);
int rx_dragGen(LPCTSTR  prompt, ads_name sset, AcGePoint3d &pt, bool move = false, int cursor = 0, bool bUseDragVec = false);
int rx_dragGen(LPCTSTR  prompt, const AcDbObjectId& objId, AcGePoint3d &pt, bool move = false, int cursor = 0, bool bUseDragVec = false);
int rx_dragGen(LPCTSTR  prompt, const AcDbObjectIdArray& objIdAr, AcGePoint3d &pt, bool move = false, int cursor = 0, bool bUseDragVec = false);

bool rx_moveTextJig(const AcDbObjectId& objId, const AcGePoint3d *firstPosPt = NULL, bool hideFirstEnt = false);
bool rx_moveEntityJig(const AcDbObjectId& entId, const AcGePoint3d *firstPosPt = NULL, bool hideFirstEnt = false);

bool rx_insert(LPCTSTR  prompt, 
               AcDbObjectId* objId,
               AcRxClass* classType, 
               LPCTSTR  text,
               double height, 
               AcDbObjectId* textStyleId,
               AcDbObjectId* layerId = NULL,
               double* angle = NULL, 
               AcGePoint3d* insertPoint = NULL,
               const AcDb::TextHorzMode hmode = AcDb::kTextLeft,
               const AcDb::TextVertMode vmode = AcDb::kTextBase);
bool rx_insert(LPCTSTR  prompt, 
               AcDbObjectId* objId,
               AcRxClass* classType, 
               LPCTSTR  text,
               double height = 0.0, 
               LPCTSTR  textStyleName = NULL,
               LPCTSTR  layerName = NULL,
               double* angle = NULL, 
               AcGePoint3d* insertPoint = NULL,
               const AcDb::TextHorzMode hmode = AcDb::kTextLeft,
               const AcDb::TextVertMode vmode = AcDb::kTextBase);
bool rx_insertBlkDef(LPCTSTR  blkName, AcDbObjectId *blkRefId = NULL, bool bReplaceExist = false, bool asHideUniqueName = false);
bool rx_insertBlk(LPCTSTR blkName, AcDbObjectId* blkRefId = NULL,
                  bool useUserInput = true, AcGePoint3d *ptIns = NULL, double *ang = NULL, double *scaleFactors = NULL,
                  LPCTSTR prompt = NULL, bool bReplaceExist = false, bool asHideUniqueName = false);
bool rx_insertBlk(const AcDbObjectId& blockId, AcDbObjectId* blkRefId = NULL,
                  bool useUserInput = true, AcGePoint3d *ptIns = NULL, double* ang = NULL, double *scaleFactors = NULL,
                  LPCTSTR  prompt = NULL);
bool rx_insertWMF(AcDbObjectId& objId, LPCTSTR fNameWmfFile, const AcGePoint3d& pt, double scaleX = 0, double scaleY = 0);

void rx_prepareColorDragBlockRef( AcDbBlockReference *pBlkRef );
void rx_prepareColorDragBlockRef(AcDbEntity *pEnt, const AcDbObjectId& blockTableRecordId);
void rx_moveBlkRef( AcDbBlockReference *blkRef, const AcGePoint3d& newPosition);
void rx_rotateBlkRef( AcDbBlockReference *blkRef, double newAngle);


Acad::ErrorStatus	rx_imageAttach (AcDbObjectId& parEntityId,
                                  LPCTSTR  ImagePath, 
                                  LPCTSTR  ImageName = NULL,
                                  int NumberOfEntities = 1, // Number of Entities that will be created.
                                  bool makeDefReactor = true
                                 );

bool rx_extentsIsValid(const AcDbExtents& ext);
void rx_extentsByLim(AcDbExtents& ext, AcDbDatabase* pDb = NULL);

// Set some viewport information.
void rx_setSomeViewport(AcDbDatabase* pDb = NULL);
bool rx_CurViewPortDataUpdate(AcDbViewTableRecord& cViewTableRecord, AcDbViewTableRecord *pViewTableRecord = NULL, bool bRead = true);
void rx_GetCurViewPortData(ads_point selPt,
                           AcGeVector3d& viewDir, AcGePoint2d& cenPt, ads_real& vScale,
                           ads_real& vHeight, ads_real& vWidth,
                           ads_point& target,
                           bool& paperspace,
                           ads_real& lenslength, ads_real& viewtwist, ads_real& frontz, ads_real& backz);
void rx_SetCurViewPortData(AcDbDatabase *pDb,
                           AcGeVector3d& viewDir, AcGePoint2d& cenPt, ads_real& vScale,
                           ads_real vHeight, ads_real vWidth,
                           ads_point target,
                           bool paperspace,
                           ads_real lenslength, ads_real viewtwist, ads_real frontz, ads_real backz);
double rx_GetViewScale();
void rx_zoomExtentsXY();
void rx_zoomExtents(AcDbViewport *vp);


bool rx_groupIsExists(AcDbDatabase *pDb, LPCTSTR  name, AcDbObjectIdArray* ids = NULL, AcDbObjectId* id = NULL);
bool rx_groupGenerateUniqieName(AcDbDatabase *pDb, LPCTSTR prefix, CString& name);
Acad::ErrorStatus rx_groupCollectEntites(const AcDbObjectId& groupId, AcDbObjectIdArray &ids);
Acad::ErrorStatus rx_groupMake(AcDbDatabase *pDb, const AcDbObjectIdArray& objIdAr,  LPCTSTR  name = NULL, AcDbObjectId* id = NULL);
Acad::ErrorStatus rx_groupDelete(AcDbDatabase *pDb, LPCTSTR name, bool removeWitchObjects = false);
Acad::ErrorStatus rx_groupFindEntity(AcDbDatabase *pDb, const AcDbObjectId& objId, AcDbObjectIdArray& resGroupIds, LPCTSTR mask = _T("*"));
Acad::ErrorStatus rx_groupFindEntity(AcDbDatabase *pDb, const AcDbObjectId& objId, SdStrObjIdList& list, LPCTSTR mask = _T("*"));
Acad::ErrorStatus rx_groupCollect(AcDbDatabase *pDb, SdStrObjIdList& list, LPCTSTR mask = _T("*"));
Acad::ErrorStatus rx_groupClearAllEmpty(AcDbDatabase *pDb);

bool rx_mslide(LPCTSTR fName = _T(""), bool bAsWMF = false, LPCTSTR  sZoomOpt = _T("_e"),
               bool bExludeXREF = false, AcDbObjectIdArray* pObjIds = NULL,
               int *pWMFFOREGND = NULL, int *pWMFBKGND = NULL);

Acad::ErrorStatus rx_getNOD (AcDbDictionary *&pNOD, const AcDb::OpenMode mode, bool bTransaction = false);

class HostDbTransaction {
public:
  HostDbTransaction(){
    m_nStart = actrTransactionManager->numActiveTransactions();
    m_pTransaction = actrTransactionManager->startTransaction();
  }    
  ~HostDbTransaction(){
    end(false);
  }
  void commit(bool fCommit=true){
    end(fCommit);
  }  
  void restart(bool fCommit=true){
    end(fCommit);
    m_pTransaction = actrTransactionManager->startTransaction();
  }  

  AcTransaction* transaction() const {
    return m_pTransaction;
  }

private:
  void end(bool fCommit){
    while(actrTransactionManager->numActiveTransactions() > m_nStart){
      fCommit ?  actrTransactionManager->endTransaction() : actrTransactionManager->abortTransaction();
    }  
    m_nStart = actrTransactionManager->numActiveTransactions();
  }  
  int           m_nStart;
  AcTransaction *m_pTransaction;
};  


class AcDbUndoDisable {
public:
  AcDbUndoDisable(){
    AcApDocument* pDoc = curDoc();
    RXASSERT(pDoc != NULL);
    start(pDoc->database());
  }    
  AcDbUndoDisable(AcApDocument* pDoc){
    RXASSERT(pDoc);
    start(pDoc->database());
  }  
  AcDbUndoDisable(AcDbDatabase *pDb){
    RXASSERT(pDb);
    start(pDb);
  }  
  ~AcDbUndoDisable(){
    end();
  }
  operator AcDbDatabase* (){
    return m_pDb;
  }
protected:
  void start(AcDbDatabase *pDb){
    RXASSERT(pDb);
    m_pDb = pDb;
    m_pDb->disableUndoRecording(true);
  }  
  void end(){
    m_pDb->disableUndoRecording(false);
  }  
  AcDbDatabase* m_pDb;
}; 


#endif // ACDBLIB

#endif //RX_UTIL_H
