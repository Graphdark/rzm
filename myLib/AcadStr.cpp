//
// (C) Copyright 1998-1999 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
//

#include "stdafx.h"
#include "AcadStr.h"

    // AutoCAD system variables
const TCHAR* AcadVar::adserr	  = _T("errno");
const TCHAR* AcadVar::cmdecho	  = _T("cmdecho");
const TCHAR* AcadVar::cmdactive = _T("cmdactive");
const TCHAR* AcadVar::cmddia    = _T("cmddia");
const TCHAR* AcadVar::cvport		= _T("cvport");
const TCHAR* AcadVar::filedia	  = _T("filedia");
const TCHAR* AcadVar::orthomode = _T("orthomode");
const TCHAR* AcadVar::snapmode  = _T("snapmode");
const TCHAR* AcadVar::snapunit  = _T("snapunit");
const TCHAR* AcadVar::snapang	  = _T("snapang");
const TCHAR* AcadVar::autosnap	= _T("autosnap");
const TCHAR* AcadVar::viewctr   = _T("viewctr");
const TCHAR* AcadVar::viewdir	  = _T("viewdir");
const TCHAR* AcadVar::viewsize	= _T("viewsize");
const TCHAR* AcadVar::vsmax     = _T("vsmax");
const TCHAR* AcadVar::vsmin	    = _T("vsmin");
const TCHAR* AcadVar::screensize = _T("screensize");
const TCHAR* AcadVar::ucsicon   = _T("ucsicon");
const TCHAR* AcadVar::rasterpreview = _T("rasterpreview");
const TCHAR* AcadVar::pdmode    = _T("pdmode");
const TCHAR* AcadVar::pdsize    = _T("pdsize");
const TCHAR* AcadVar::limcheck  = _T("limcheck");
const TCHAR* AcadVar::limmin    = _T("limmin");
const TCHAR* AcadVar::limmax    = _T("limmax");
const TCHAR* AcadVar::gridmode  = _T("gridmode");
const TCHAR* AcadVar::attdia    = _T("attdia");
const TCHAR* AcadVar::attmode   = _T("attmode");
const TCHAR* AcadVar::attreq    = _T("attreq");
const TCHAR* AcadVar::lastpoint = _T("lastpoint");
const TCHAR* AcadVar::osmode    = _T("osmode");
const TCHAR* AcadVar::pickstyle = _T("pickstyle");
const TCHAR* AcadVar::textsize  = _T("textsize");
const TCHAR* AcadVar::expert    = _T("expert");
const TCHAR* AcadVar::lwdisplay = _T("lwdisplay");
const TCHAR* AcadVar::dbmod     = _T("dbmod");


    // AutoCAD common strings
const TCHAR* AcadString::continuous	 = _T("Continuous");		// needs to be translated
const TCHAR* AcadString::bylayer		 = _T("ByLayer");        // needs to be translated
const TCHAR* AcadString::byblock		 = _T("ByBlock");        // needs to be translated
const TCHAR* AcadString::zero        = _T("0");				// default layer
const TCHAR* AcadString::wcAll		   = _T("*");				// wild card for all
const TCHAR* AcadString::nullStr		 = _T("");				// ""
const TCHAR* AcadString::openBrace   = _T("{");
const TCHAR* AcadString::endBrace    = _T("}");
const TCHAR* AcadString::standard    = _T("Standard");
const TCHAR* AcadString::none        = _T("*NONE*");
const TCHAR* AcadString::varies1     = _T("*VARIES*");
const TCHAR* AcadString::varies2     = _T("VARIES");

    // AutoCAD files
const TCHAR* AcadFile::ltypesStandard	 = _T("acad.lin");
const TCHAR* AcadFile::ltypesComplex	 = _T("ltypeshp.lin");

