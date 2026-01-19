// pch.h: это предварительно скомпилированный заголовочный файл.
// Перечисленные ниже файлы компилируются только один раз, что ускоряет последующие сборки.
// Это также влияет на работу IntelliSense, включая многие функции просмотра и завершения кода.
// Однако изменение любого из приведенных здесь файлов между операциями сборки приведет к повторной компиляции всех(!) этих файлов.
// Не добавляйте сюда файлы, которые планируете часто изменять, так как в этом случае выигрыша в производительности не будет.

#ifndef PCH_H
#define PCH_H

// Добавьте сюда заголовочные файлы для предварительной компиляции
//#include "framework.h"

#if _MSC_VER > 1000
#pragma once
#endif

//-----------------------------------------------------------------------------
#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#pragma message("Building debug version of this module to be used with non-debug/Prod AutoCAD")
#define DEBUG_THIS_ONLY
#undef _DEBUG
#endif

#ifndef _AFXDLL
#define _AFXDLL
#pragma message("Сборка должна содержать _AFXDLL !")
#endif

#ifndef _AFXEXT
#define _AFXEXT
//#undef _AFXEXT
#pragma message("Сборка должна содержать _AFXEXT !")
#endif

#pragma pack (push, 8)

#pragma warning(disable: 4786 4996)
//#pragma warning(disable: 4098)

//#pragma warning(disable: 4044 4070 4099)
#pragma warning(disable: 4275)
#pragma warning(disable: 4996)
#pragma warning(disable: 4005) 
#pragma warning(disable: 4141)
#pragma warning(disable: 4127)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4197)
#pragma warning(disable: 4099)

//#pragma warning(disable: 4098)
//#pragma warning(disable: 4251)
//#pragma warning(disable: 4275)
//#pragma warning(disable: 4812)
//#pragma warning(disable: 4430)

#pragma warning(disable: 4290)  //warning C4290: спецификация исключений C++ игнорируется, кроме случая, когда указывается, что функция не является __declspec(nothrow)

#define NOMINMAX


#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRECATE        // For VS 2005
//#define _CRT_NONSTDC_NO_DEPRECATE       // For VS 2005
#define _CRT_NON_CONFORMING_SWPRINTFS   // For VS 2005

//-----------------------------------------------------------------------------
#define STRICT

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            //- Exclude rarely-used stuff from Windows headers
#endif

//#define _CONVERSION_DONT_USE_THREAD_LOCALE  // Без этого падает на китайской локали !!! Правильная альтернатива: SetThreadLocale(LOCALE_SYSTEM_DEFAULT);

#ifdef _NCAD_BUILD_

#pragma message("Сборка под NanoCAD")

//#define USE_BASEDLG
//- Modify the following defines if you have to target a platform prior to the ones specified below.
//- Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  //- Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501           //- Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT            //- Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501     //- Change this to the appropriate value to target Windows 2000 or later.
#endif                        

#ifndef _WIN32_WINDOWS          //- Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501   //- Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               //- Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0501        //- Change this to the appropriate value to target IE 5.0 or later.
#endif

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
// Including SDKDDKVer.h defines the highest available Windows platform.
//#include <SDKDDKVer.h>
//#include "csafx.h"

#include <afxwin.h>
#include <afxext.h>
#include <AtlBase.h>
#include <AtlCom.h>
using namespace ATL;

#include "WinUser.h"
#include "dbxHeaders.h"
#include "AcExtensionModule.h"


//#include "my_nrx.h"

#else //_NCAD_BUILD_

#pragma message("Сборка под AutoCAD")

#ifndef _WIN32_WINNT
  #if defined(_ACAD2017)
    #define _WIN32_WINNT 0x0502
  #else
    #if defined(_ACAD2000) || defined(_ACAD2004) || defined(_ACAD2007)
      #define _WIN32_WINNT 0x0410
    #else
      #if defined(_ACAD2010) || defined(_ACAD2013) || defined(_ACAD2015)
        #define _WIN32_WINNT 0x0500
      #endif
    #endif 
  #endif
#endif

#endif //_NCAD_BUILD_


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
//#include <windows.h>
//#include <unknwn.h>

//----- Modify the following defines if you have to target a platform prior to the ones specified below.
//----- Refer to MSDN for the latest info on corresponding values for different platforms.


#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxtempl.h>

#ifdef _ACAD2000
//#include <afxdlgs.h>
//#include <afxdtctl.h>			//----- MFC support for Internet Explorer 4 Common Controls
//#include <afxdhtml.h>
#endif

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
//#include <afxdb.h>            // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#pragma warning(push)
#pragma warning(disable:4265)//disable missing virtual destructor warning
//#include <afxdao.h>         // MFC DAO database classes
#pragma warning(pop) // C4265
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>            // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//-----------------------------------------------------------------------------
//----- Include ObjectDBX/ObjectARX headers
//----- Uncomment one of the following lines to bring a given library in your project.
//#define _BREP_SUPPORT_			//- Support for the BRep API
//#define _HLR_SUPPORT_				//- Support for the Hidden Line Removal API
//#define _IE_SDK_SUPPORT_			//- Support for the Autodesk Image Engine API
//#define _AMODELER_SUPPORT_		//- Support for the AModeler API
//#define _ASE_SUPPORT_				//- Support for the ASI/ASE API
//#define _RENDER_SUPPORT_			//- Support for the AutoCAD Render API
//#define _ARX_CUSTOM_DRAG_N_DROP_	//- Support for the ObjectARX Drag'n Drop API
//#define _INC_LEAGACY_HEADERS_		//- Include legacy headers in this project
//-----------------------------------------------------------------------------

#include <math.h>

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif

#ifdef _ACAD2000
#include "dbobjptr.h"           //!!!взят из ObjectArxSDK 2007 лежии в LIB!!!
#include "AcadStdafx.h"
//#include "..\HTML_HELP_API\HtmlHelp.h"
#include "acad15.h"             // AutoCAD Automation interfaces
#include "acaplmgr.h"
#else
#include "arxHeaders.h"
#include "ol_errno.h"
#include "acadi.h"
#endif  //_ACAD2000

//#pragma comment(linker, "/NODEFAULTLIB:MSVCRTD") 

// RX Includes
#include "StdArx.h"

#include "acdb.h"               // acdb definitions
#include "rxregsvc.h"           // ARX linker
#include "dbapserv.h"           // Host application services
#include "aced.h"               // aced stuff
#include "adslib.h"             // RXADS definitions
#include "acdocman.h"           // MDI document manager
#include "rxmfcapi.h"           // ObjectARX MFC support
#include "AcExtensionModule.h"  // Utility class for extension dlls
#include "dbptrar.h"            // void pointer array        
#include "axlock.h"             // Document locking class (no operation in ObjectDBX)
#include "dbidar.h"
//#include "axdb.h"
//#include "acuidock.h"

//-----------------------------------------------------------------------------
//#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
// You may derive a class from CComModule and use it if you want to override
// something, but do not change the name of _Module
//extern CComModule _Module;
//#include <atlcom.h>

//-----------------------------------------------------------------------------

#include <math.h>


#pragma pack (pop)


// Turn on the _DEBUG symbol if it was defined, before including
// non-MFC header files.
//
#ifdef DEBUG_THIS_ONLY
#define _DEBUG
#undef DEBUG_THIS_ONLY
#endif

#include "rxdebug.h"
#include "rx_util.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_PI_2
#define M_PI_2 (M_PI*2)
#endif


//#include "AcSmartPtr.h"


template<class T_OBJECT>Acad::ErrorStatus rxOpenObject(AcDbObjectPointerBase<T_OBJECT>& pObj, const AcDbObjectId& id, AcDb::OpenMode mode = AcDb::kForRead, bool open_erased = false)
{
  return pObj.open(id, mode, open_erased);
}

template<class T_OBJECT>Acad::ErrorStatus rxCloseObject(AcDbObjectPointerBase<T_OBJECT>& pObj)
{
  return pObj.close();
}

Acad::ErrorStatus  addToDb(AcDbEntity* ent);

template<class T_OBJECT>Acad::ErrorStatus
postObjPtrToDb(AcDbObjectPointerBase<T_OBJECT>& pObj, AcDbObjectId& objId, AcDbDatabase* pDb = NULL)
{
  Acad::ErrorStatus es = addToDb(pObj, objId, pDb);
  if (es != Acad::eOk) return es;
  return pObj.close();
}

template<class T_OBJECT>Acad::ErrorStatus
postObjPtrToDb(AcDbObjectPointerBase<T_OBJECT>& pObj)
{
  AcDbObjectId objId;
  return postObjPtrToDb(pObj, objId);
}


extern ULONG AcDbHandleToULONG(const AcDbHandle& handle);

//#define _BUILD_E8

#ifdef _BUILD_E8
#pragma message("Сборка под E8")
#else
#pragma message("Сборка под E7")
#endif



#endif //PCH_H
