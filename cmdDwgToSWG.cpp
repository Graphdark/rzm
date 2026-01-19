#include "stdafx.h"

#include "sdsysvar.h"

#include "rx_util.h"
#include <string>

//class LC_MakerCamSVG;

//#include "lib\generators\makercamsvg\lc_xmlwriterqxmlstreamwriter.h"
//#include "lib\generators\makercamsvg\lc_makercamsvg.h"

#include "ac_svg_generator.h"

void rx_saveToSvg(LPCTSTR fileName, const AcDbObjectIdArray& objIds)
{
  //CSVG svg;
  //svg.DwgToSWG(objIds, fileName);

  //auto svg = std::make_unique<LC_MakerCamSVG>(std::make_unique<LC_XMLWriterQXmlStreamWriter>());
  //svg->generate(curDoc()->database());
  //ofstream test_svg;
  //test_svg.open(fileName);
  //string s = svg->resultAsString();
  //test_svg.write(s.c_str(), s.size());
  //test_svg.close();

  svg_gen svg;
  svg.svg_generate(fileName, objIds);
}

void command_exportToSvg()
{
  try
  {
    AcDbObjectIdArray objIds;
    rx_collectEntites(objIds);
    if (objIds.length() == 0)
      throw _T("*Пустой набор*");

    CString fName = "e:\\test.svg";
    if (fName.IsEmpty())
    {
      resbuf* rb = acutNewRb(RTSTR);
      int flags;
      flags = 2;			// disable the silly "type it" button
      flags += 128;	  // disable the silly "type it" button
      //if (isForWrite)
      flags += 1;
      //if (allowArbitraryExt)
      flags += 4;		// enable arbitrary extension
      //if (defIsDirectory)
      flags += 16;    // interpret default path as directory, not filename
      if (acedGetFileD(_T("Укажите файл для сохранения"), fName, _T("svg"), flags, rb) != RTNORM)
      {
        acutRelRb(rb);
        return;
      }
      else
      {
        fName = rb->resval.rstring;
        acutRelRb(rb);
      }
    }

    CWaitCursor wc;
    acutPrintf(_T("Сохранение фрагмента..."));
    rx_saveToSvg(fName, objIds);
  }
  catch (LPCTSTR s) { if (s && *s) { acutPrintf(s); } }
}