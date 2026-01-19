// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////
//
// Define the sole extension module object.
AC_IMPLEMENT_EXTENSION_MODULE(SVGEDLL);

extern void command_exportToSvg();

/////////////////////////////////////////////////////////////////////////////
// ObjectARX EntryPoint
//////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#define DLLIMPEXP __declspec(dllexport)
#else
#define DLLIMPEXP __attribute__((visibility("default")))
#endif

extern "C" DLLIMPEXP AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
  switch (msg)
  {
  case AcRx::kInitAppMsg:
    acrxDynamicLinker->unlockApplication(appId);
    acrxDynamicLinker->registerAppMDIAware(appId);

    acedRegCmds->addCommand(L"RZT", L"RZT_MSVG", L"RZT_MSVG", ACRX_CMD_MODAL, command_exportToSvg);
    break;

  case AcRx::kUnloadAppMsg:
    acedRegCmds->removeGroup(L"RZT_MSVG");
    break;
  }

  return AcRx::kRetOK;
}

//////////////////////////////////////////////////////////////////////////


