#include "stdafx.h"

#include "math.h"

#include "gepnt3d.h"
#include "gearc2d.h"
#include "gearc3d.h"
#include "geassign.h"

#include "dbspline.h"
#include "dbents.h"
#include "dbsymtb.h"

#include "rx_util.h"
#include "str_utils.h"

#include "dbapserv.h"

#define ZERO    1e-10

#ifndef _LCT
#define _LCT _T
#endif

#ifdef DEBUG_RXPRINT 
int rx_iRXPRINT__MODE__ =
#ifdef _DEBUG 
  1
#else
  0
#endif
  ;
#endif

#ifdef __AFX_H__

// Set dwError value by calling: DWORD dwError = GetLastError();
LPCTSTR FetchLastErrorMessage(DWORD dwError, CString &strMessage)
{
  LPVOID lpMsgBuf = NULL; 
  // Search for the message description in the std windows
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dwError,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL );
  if (lpMsgBuf == NULL) // get the description from the WinHttp library (functions like WinHttpSendRequest)
    FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER  // allocate buffer (free with LocalFree())
    | FORMAT_MESSAGE_IGNORE_INSERTS // don't process inserts
    | FORMAT_MESSAGE_FROM_HMODULE,  // retrieve message from specified DLL
    GetModuleHandle(_T("winhttp.dll")), // module to retrieve message text from
    dwError,
    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0,
    NULL);
  if (lpMsgBuf == NULL) // get the description from the WinInet library
    FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER  // allocate buffer (free with LocalFree())
    | FORMAT_MESSAGE_IGNORE_INSERTS // don't process inserts
    | FORMAT_MESSAGE_FROM_HMODULE,  // retrieve message from specified DLL
    GetModuleHandle(_T("wininet.dll")), // module to retrieve message text from
    dwError,
    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0,
    NULL);
  if(lpMsgBuf != NULL)
  {
    strMessage = (LPCTSTR)lpMsgBuf;
    LocalFree( lpMsgBuf );
  }
  else
    if( dwError )
      strMessage.Format( _T("Last Error Number (%d)"), dwError );
  return strMessage;
}


void rx_pushException(CException* e, int msgLevel /*= 1*/, LPCTSTR  nameFunction /*= NULL*/)
{
  if (msgLevel > 0)
  {
    CString msg;
    TCHAR   szCause[255];
    e->GetErrorMessage(szCause,255);
    if ( e->IsKindOf(RUNTIME_CLASS(COleException)) || e->IsKindOf(RUNTIME_CLASS(COleDispatchException)) )
    {
      if ( nameFunction != NULL && nameFunction[0] != NULL )
        msg.Format(_LCT("»сключение в %s:%s"), nameFunction, szCause);
      else
        msg.Format(_LCT("»сключение:%s"), szCause);
      if (msgLevel == 1)
        acdbFail((LPCTSTR)msg);
      else
        AfxMessageBox(msg);
    }
    else
      if (szCause[0] != 0)
        if (msgLevel == 1)
          acdbFail(szCause);
        else
          AfxMessageBox(szCause);
  }
  e->Delete();
}
#endif __AFX_H__

#ifndef ARXDBG_UTILS_H

bool ArxDbgUtils::isPaperSpace(AcDbDatabase* db)
{
  if (db->tilemode() == 1)
    return false;
  return (db->paperSpaceVportId() == acedGetCurViewportObjectId());
}

/****************************************************************************
**
**	ArxDbgUtils::symbolIdToName
**		given an objectId for a symbol table record, retrieve the name
**	associated with this symbol.
**
**	**jma
**
*************************************/

Acad::ErrorStatus
ArxDbgUtils::symbolIdToName(const AcDbObjectId& symbolId, CString& name)
{
  // shouldn't happen usually, but some debugging routines
  // want to print out this info
  if (symbolId == AcDbObjectId::kNull) {
    name = _T("AcDbObjectId::kNull");
    return Acad::eNullObjectId;
  }
  // get symbol name from symbol table record
  AcDbSymbolTableRecord* tblRec;
  Acad::ErrorStatus es;
  es = acdbOpenObject(tblRec, symbolId, AcDb::kForRead);
  if (es == Acad::eOk) {
    const TCHAR* locName;
    tblRec->getName(locName);
    name = locName;
    tblRec->close();
  }
  else {
    ArxDbgUtils::rxErrorMsg(es);
    es = Acad::eInvalidInput;
  }

  return es;
}

Acad::ErrorStatus
ArxDbgUtils::nameToSymbolId(AcRxClass* symTblClass, LPCTSTR name,
  AcDbObjectId& recordId, AcDbDatabase* db)
{
  ASSERT(symTblClass != NULL);
  ASSERT(db != NULL);

  AcDbSymbolTable* symTbl;
  if ((symTbl = openSymbolTable(symTblClass, AcDb::kForRead, db)) == NULL)
    return Acad::eInvalidInput;

  Acad::ErrorStatus es;
  es = symTbl->getAt(name, recordId);
  symTbl->close();
  return es;
}

/****************************************************************************
**
**	ArxDbgUtils::openSymbolTable
**		get a generic symbol table pointer from a specific symbol
**	table.
**
**	**jma
**
*************************************/

AcDbSymbolTable* ArxDbgUtils::openSymbolTable(AcRxClass* symTblClass,  AcDb::OpenMode mode, AcDbDatabase* db)
{
  ASSERT(symTblClass != NULL);
  ASSERT(db != NULL);

  AcDbSymbolTable* symTbl = NULL;
  Acad::ErrorStatus es;

  if (symTblClass == AcDbBlockTableRecord::desc()) {
    AcDbBlockTable* blkTbl;
    es = db->getBlockTable(blkTbl, mode);
    symTbl = blkTbl;
  }
  else if (symTblClass == AcDbDimStyleTableRecord::desc()) {
    AcDbDimStyleTable* dimTbl;
    es = db->getDimStyleTable(dimTbl, mode);
    symTbl = dimTbl;
  }
  else if (symTblClass == AcDbLayerTableRecord::desc()) {
    AcDbLayerTable* layerTbl;
    es = db->getLayerTable(layerTbl, mode);
    symTbl = layerTbl;
  }
  else if (symTblClass == AcDbLinetypeTableRecord::desc()) {
    AcDbLinetypeTable* ltypeTbl;
    es = db->getLinetypeTable(ltypeTbl, mode);
    symTbl = ltypeTbl;
  }
  else if (symTblClass == AcDbTextStyleTableRecord::desc()) {
    AcDbTextStyleTable* textTbl;
    es = db->getTextStyleTable(textTbl, mode);
    symTbl = textTbl;
  }
  else if (symTblClass == AcDbRegAppTableRecord::desc()) {
    AcDbRegAppTable* appTbl;
    es = db->getRegAppTable(appTbl, mode);
    symTbl = appTbl;
  }
  else if (symTblClass == AcDbUCSTableRecord::desc()) {
    AcDbUCSTable* ucsTbl;
    es = db->getUCSTable(ucsTbl, mode);
    symTbl = ucsTbl;
  }
  else if (symTblClass == AcDbViewTableRecord::desc()) {
    AcDbViewTable* viewTbl;
    es = db->getViewTable(viewTbl, mode);
    symTbl = viewTbl;
  }
  else if (symTblClass == AcDbViewportTableRecord::desc()) {
    AcDbViewportTable* vportTbl;
    es = db->getViewportTable(vportTbl, mode);
    symTbl = vportTbl;
  }
  else
    es = Acad::eInvalidInput;    // passed in a class type that is illegal

  ASSERT(es == Acad::eOk);    // should never fail!
  if (es != Acad::eOk) {
    ArxDbgUtils::rxErrorMsg(es);
    return NULL;
  }
  else
    return symTbl;
}

Acad::ErrorStatus ArxDbgUtils::collectSymbolIds(AcRxClass* symTblClass, AcDbObjectIdArray& objIds, AcDbDatabase* db)
{
  ASSERT(symTblClass != NULL);
  ASSERT(db != NULL);

  Acad::ErrorStatus retCode = Acad::eInvalidInput;

  AcDbSymbolTable* symTbl = openSymbolTable(symTblClass, AcDb::kForRead, db);
  if (symTbl != NULL) {
    // get an iterator over this symbol Table
    AcDbSymbolTableIterator* tblIter;
    if (symTbl->newIterator(tblIter) == Acad::eOk) {
      // walk table and just collect all the objIds
      // of the entries
      Acad::ErrorStatus es;
      AcDbObjectId tblRecId;
      for (; !tblIter->done(); tblIter->step()) {
        es = tblIter->getRecordId(tblRecId);
        if (es == Acad::eOk)
          objIds.append(tblRecId);
      }
#ifndef _NCAD_BUILD_NODELETE
      delete tblIter;
#endif
      retCode = Acad::eOk;
    }
    symTbl->close();
  }
  return retCode;
}

#endif // ARXDBG_UTILS_H


bool rx_isCurDocQuiescent()
{
  return (/*acedIsInputPending() &&*/ curDoc() && curDoc()->isQuiescent());
}

//*************************************************************************
// Database related utility routines 
//*************************************************************************

Acad::ErrorStatus getCurrentSpace(AcDbBlockTableRecord*& pSpaceRecord, AcDbDatabase *pDb /*= NULL*/)
{
  pSpaceRecord = NULL;
  Acad::ErrorStatus  es;
  AcDbBlockTable*    pBlockTable;
  LPCTSTR pSpace = ACDB_MODEL_SPACE;
  if( pDb == NULL)
    pDb = WORKDWG;

  if (ArxDbgUtils::isPaperSpace(pDb))
    pSpace = ACDB_PAPER_SPACE;

  if ((es = pDb->getSymbolTable(pBlockTable, AcDb::kForRead)) != Acad::eOk)
    return es;
  if ((es = pBlockTable->getAt(pSpace, pSpaceRecord, AcDb::kForWrite)) != Acad::eOk) 
  {
    pBlockTable->close();
    return es;
  }
  if ((es = pBlockTable->close()) != Acad::eOk)
  {
    pSpaceRecord->close();
    return es;
  }
  return es;
}

//  Append specified entity to current space of current drawing.
Acad::ErrorStatus 
addToDb(AcDbEntity* ent, AcDbObjectId& objId, AcDbDatabase *pDb /*= NULL*/)
{
  AcDbBlockTableRecord*  pSpaceRecord;
  Acad::ErrorStatus es = getCurrentSpace(pSpaceRecord, pDb);
  if (es == Acad::eOk)
  {
    es = pSpaceRecord->appendAcDbEntity(objId, ent);
    pSpaceRecord->close();
  }
  return es;
}

Acad::ErrorStatus 
addToDb(AcDbEntity* ent)
{
  AcDbObjectId objId;
  return addToDb(ent, objId);
}

//  Append specified entity to current space of current drawing.
Acad::ErrorStatus 
postToDb(AcDbEntity* ent, AcDbObjectId& objId, AcDbDatabase *pDb /*= NULL*/)
{
  Acad::ErrorStatus es = addToDb(ent, objId, pDb);
  if (es != Acad::eOk) return es;
  return ent->close();
}

Acad::ErrorStatus 
postToDb(AcDbEntity* ent)
{
  AcDbObjectId objId;
  return postToDb(ent, objId);
}

bool rx_eraseEntity(const AcDbObjectId& objId, bool unlockLayer)
{
  AcDbEntityPtr pEnt;
  Acad::ErrorStatus es = rxOpenObject(pEnt, objId, AcDb::kForWrite);
  if (es == Acad::eOnLockedLayer && unlockLayer)
  {
    AcDbLayerTableRecordPointer pLayer;
    es = rxOpenObject(pLayer, pEnt->layerId(), AcDb::kForWrite);
    if (es == Acad::eOk)
    {
      ASSERT(pLayer->isLocked());
      pLayer->setIsLocked(false);
      rxCloseObject(pLayer);
    }
    es = rxOpenObject(pEnt, objId, AcDb::kForWrite);
  }
  if (es == Acad::eOk && pEnt != NULL)
    return pEnt->erase() == Acad::eOk;
  return false;
}

void rx_eraseEntities(AcDbObjectIdArray& objIdAr, bool unlockLayer)
{
  for (int i=0; i < objIdAr.length(); i++)
    if (rx_eraseEntity(objIdAr[i], unlockLayer))
      objIdAr.removeAt(i--);
}


Acad::ErrorStatus rx_makeText(AcGePoint3d pt, 
                              LPCTSTR  text,
                              double height, 
                              double ang /* = 0.0*/, 
                              AcDb::TextHorzMode hmode /*= AcDb::kTextLeft*/, 
                              AcDb::TextVertMode vmode /*= AcDb::kTextBase*/,
                              bool italic /*= false*/,
                              LPCTSTR  layerName /*= NULL*/,
                              int colorIndex /*= 256*/,
                              bool HighLight /*=false*/,
                              AcDbObjectId *objId /*=NULL*/)
{
  AcDbText *pText = new AcDbText;
  pText->setDatabaseDefaults();
  pText->setTextString(text);
  pText->setHeight(height);
  pText->setRotation(ang);
  pText->setHorizontalMode(hmode);
  pText->setVerticalMode(vmode);
  pText->setPosition(pt);
  pText->setAlignmentPoint(pt);
  pText->setColorIndex(colorIndex);
  if (layerName)
    pText->setLayer(layerName);
  if ( italic )
    pText->setOblique( 15*PI/180.0 );

  Acad::ErrorStatus es;
  if ( objId != NULL )
    es = postToDb(pText, *objId);
  else
    es = postToDb(pText);
  if ( es != Acad::eOk ) 
    delete pText;
  return es;
}

Acad::ErrorStatus rx_makeAttDef(AcGePoint3d pt, 
                              LPCTSTR  tag,
                              LPCTSTR  prompt,
                              LPCTSTR  text,
                              double height, 
                              double ang  /*= 0.0*/, 
                              AcDb::TextHorzMode hmode /*= AcDb::kTextLeft*/, 
                              AcDb::TextVertMode vmode /*= AcDb::kTextBase*/,
                              bool italic /*= false*/,
                              LPCTSTR  layerName /*= NULL*/,
                              int colorIndex /*= 256*/,
                              bool HighLight /*=false*/,
                              AcDbObjectId *objId /*=NULL*/)
{
  Acad::ErrorStatus es = Acad::eOk;
  AcDbAttributeDefinition *pAttDef = new AcDbAttributeDefinition;
  pAttDef->setTag(tag);
  pAttDef->setPrompt(prompt);
  pAttDef->setTextString(text);
  pAttDef->setHeight(height);
  pAttDef->setRotation(ang);
  pAttDef->setHorizontalMode(hmode);
  pAttDef->setVerticalMode(vmode);
  pAttDef->setPosition(pt);
  pAttDef->setAlignmentPoint(pt);
  pAttDef->setColorIndex(colorIndex);
  if (layerName)
    pAttDef->setLayer(layerName);
  if ( italic )
    pAttDef->setOblique( 15*PI/180.0 );

  if ( objId != NULL )
    es = postToDb(pAttDef, *objId);
  else
    es = postToDb(pAttDef);
  if ( es != Acad::eOk ) 
    delete pAttDef;
  return es;
}

Acad::ErrorStatus rx_textToAttDef(AcDbObjectId& textId,
                                  LPCTSTR  tag,
                                  LPCTSTR  prompt)
{
  AcDbText *pText;
  Acad::ErrorStatus es = acdbOpenObject(pText, textId, AcDb::kForWrite);
  if (es != Acad::eOk) return es;
  AcDbAttributeDefinition *pAttDef = new AcDbAttributeDefinition;
  pAttDef->setTag(tag);
  pAttDef->setPrompt(prompt);
  TCHAR *text = pText->textString();
  pAttDef->setTextString(text);
  free(text);
  pAttDef->setLayer(pText->layerId());
  pAttDef->setColorIndex(pText->colorIndex());
  pAttDef->setHeight(pText->height());
  pAttDef->setRotation(pText->rotation());
  pAttDef->setOblique(pText->oblique());
  pAttDef->setHorizontalMode(pText->horizontalMode());
  pAttDef->setVerticalMode(pText->verticalMode());
  pAttDef->setPosition(pText->position());
  pAttDef->setAlignmentPoint(pText->alignmentPoint());
  es = postToDb(pAttDef, textId);
  if ( es == Acad::eOk ) 
  {
    resbuf *xd = pText->xData();
    if (xd)
    {
      if (acdbOpenObject(pText, textId, AcDb::kForWrite) == Acad::eOk)
      {
        pAttDef->setXData(xd);
        pAttDef->close();
      }
      acutRelRb(xd);
    }
    pText->erase();
    pText->close();
  }
  else
    delete pAttDef;
  return es;
}

Acad::ErrorStatus rx_cloneText(AcDbText* pText, AcDbObjectId& newObjId)
{
  AcDbText *pNewText = new AcDbText;
	pNewText->setPropertiesFrom(pText);
	TCHAR *text = pText->textString();
	pNewText->setTextString(text);
	free(text);
	pNewText->setLayer(pText->layerId());
	pNewText->setColorIndex(pText->colorIndex());
	pNewText->setHeight(pText->height());
	pNewText->setRotation(pText->rotation());
	pNewText->setOblique(pText->oblique());
	pNewText->setHorizontalMode(pText->horizontalMode());
	pNewText->setVerticalMode(pText->verticalMode());
	pNewText->setPosition(pText->position());
	pNewText->setAlignmentPoint(pText->alignmentPoint());
	Acad::ErrorStatus es = postToDb(pNewText, newObjId);
	if ( es == Acad::eOk ) 
	{
		resbuf *xd = pText->xData();
		if (xd)
		{
			if (acdbOpenObject(pNewText, newObjId, AcDb::kForWrite) == Acad::eOk)
			{
				pNewText->setXData(xd);
				pNewText->close();
			}
			acutRelRb(xd);
		}
	}
	else
		delete pNewText;
	return es;
}

Acad::ErrorStatus rx_setMTextByText(AcDbMText *pMText, const AcDbText *pText)
{
  pMText->setPropertiesFrom(pText);

  AcDb::TextVertMode vmode = pText->verticalMode();
  AcDb::TextHorzMode hmode = pText->horizontalMode();

  AcGePoint3d pt;
  rx_getTextPosition(pText, pt);

  pMText->setTextStyle(pText->textStyle());
  pMText->setTextHeight(pText->height());

#if defined( _ACAD2000 )
  pMText->setFromTextEntity((AcDbText*)pText);
  pMText->setVerticalMode(vmode);
  pMText->setHorizontalMode(hmode);
  pMText->setAlignmentPoint(pt);
  pMText->setOblique(pText->oblique());
#endif

  AcDbMText::AttachmentPoint attPoint;
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kTopLeft;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kTopCenter;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kTopRight;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kMiddleLeft;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kMiddleCenter;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kMiddleRight;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kTopLeft;//AcDbMText::kBottomLeft;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kTopCenter;//AcDbMText::kBottomCenter;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kTopRight;//AcDbMText::kBottomRight;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kBottomLeft;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kBottomCenter;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kBottomRight;
  else 
    attPoint = AcDbMText::kBottomLeft;
  //pMText->setAttachment( attPoint );
  rx_setTextAttachment(pMText, attPoint);

  pMText->setLocation(pt);

  pMText->setRotation(pText->rotation());
  pMText->setTextStyle(pText->textStyle());

  TCHAR* buf = NULL;
  buf = pText->textString();
  pMText->setContents(buf);
  free(buf);

  resbuf *rb = pText->xData();
  pMText->setXData(rb);
  acutRelRb(rb);

  return Acad::eOk;
}

Acad::ErrorStatus rx_textToMText(AcDbObjectId& objId)
{
  AcDbTextPtr pText;
  Acad::ErrorStatus es = rxOpenObject(pText, objId, AcDb::kForWrite);
  if (es != Acad::eOk)
    return es;
  AcDbMText *pMText = new AcDbMText;
  rx_setMTextByText(pMText, pText);
  pText->erase();
  return postToDb(pMText, objId);
}


/*
Acad::ErrorStatus _rx_textToMText(AcDbObjectId& textId)
{
  Acad::ErrorStatus es = Acad::eOk;
  AcDbText* pText = NULL;
  AcDbMText* pMText = NULL;
  AcDbExtents extents, extentsTmp;

  try
  {
    ARXOK(acdbOpenObject(pText, textId, AcDb::kForWrite));

    
    pMText->setLineSpacingFactor(0.9);

    if (ext.minPoint() != ext.maxPoint())
    {
      switch (hm)
      {
      case AcDb::kTextLeft   :
        pt.x = ext.minPoint().x; 
        break;
      case AcDb::kTextCenter :
        pt.x = (ext.minPoint() + (ext.maxPoint() - ext.minPoint())/2.0).x;
        break;
      case AcDb::kTextRight  :
        pt.x = ext.maxPoint().x; 
        break;
      case AcDb::kTextAlign  :
        pt.x = ext.minPoint().x; 
        break;
      case AcDb::kTextMid    :
        pt.x = ext.minPoint().x; 
        break;
      case AcDb::kTextFit    :
        pt.x = ext.minPoint().x; 
        break;
      default:
        ;
      }
      switch (vm)
      {
      case AcDb::kTextBase   :
        pt.y = ext.maxPoint().y;
        break;
      case AcDb::kTextBottom :
        pt.y = ext.minPoint().y;
        break;
      case AcDb::kTextVertMid:
        pt.y = (ext.minPoint() + (ext.maxPoint() - ext.minPoint())/2.0).y;
        break;
      case AcDb::kTextTop    :
        pt.y = ext.maxPoint().y;
        break;
      default:
        ;
      }
    }

#ifdef _ACAD2000
    if ( pMText->isDefaultAlignment() )
#endif //_ACAD2000
      pMText->setLocation(pt);
#ifdef _ACAD2000
    else
    {
      pMText->setAlignmentPoint(pt);
    }
#endif //_ACAD2000

    if ( ext.maxPoint().x - ext.minPoint().x  > 1.0 )
      pMText->setWidth(ext.maxPoint().x - ext.minPoint().x);

    stringToMtextContents(text, pMText);
    postToDb(pMText, objId);
  }
  catch(Acad::ErrorStatus es)
  {
    PRINT_STATUS_ERROR(es);
    ::es = es;
  }
  return es;
}
*/

Acad::ErrorStatus rx_setTextByMText(AcDbText* pText, const AcDbMText* pMText)
{
  Acad::ErrorStatus es;
  es = pText->setPropertiesFrom(pMText);
  if (es != Acad::eOk) return es;

  AcGePoint3d pt;
  es = rx_getTextPosition(pMText, pt);
  if (es != Acad::eOk) return es;

  pText->setRotation(pMText->rotation());
  rx_setTextHeight(pText, pMText->textHeight());
  pText->setTextString(pMText->contents());
  pText->setTextStyle(pMText->textStyle());

  es = rx_updTextByStyle(pText);

  AcDbMText::AttachmentPoint attPoint;
  es = rx_getTextAttachment(pMText, attPoint);
  if (es != Acad::eOk) return es;
  rx_setTextAttachment(pText, attPoint);

  pText->setPosition(pt);
  pText->setAlignmentPoint(pt);
  rx_setTextPosition(pText, pt);

  resbuf *rb = pMText->xData();
  pText->setXData(rb);
  acutRelRb(rb);

  return es;
}

Acad::ErrorStatus rx_mtextToText(AcDbObjectId& objId)
{
  //Acad::ErrorStatus es = eNotImplementedYet;
  AcDbMTextPtr pMText;
  Acad::ErrorStatus es = rxOpenObject(pMText, objId, AcDb::kForWrite);
  if (es != Acad::eOk)
    return es;

  AcDbText *pTextN = new AcDbText;
  AcDbTextPtr pText; pText.acquire(pTextN);
  es = rx_setTextByMText(pText, pMText);
  if (es == Acad::eOk)
  {
    pMText->erase();
    es = postToDb(pText, objId);
  }
  return es;
}

CString rx_mtextContentToTextLine(LPCTSTR contents)
{
  CString s = contents;
  int i = 0;
  while (i < s.GetLength())
  {
    if (s[i] == '\n')
      s.Delete(i);
    else
    if (s[i] == *AcDbMText::blockBegin() && s[i+1] == '\\') 
    {
      s.Delete(i, 3);
      int ie = i;
      while (ie < s.GetLength())
      { 
        if (s[ie] == *AcDbMText::blockEnd())
        {
          s.Delete(ie);
          break;
        }
        ie++;
      }
    }
    else
      i++;
  }
  s.TrimLeft();
  s.TrimRight();
  return s;
}

Acad::ErrorStatus rx_getTextString(AcDbEntity *pEnt, CString& text)
{
  TCHAR *t = NULL;
  if (pEnt->isKindOf(AcDbText::desc()))
  {
    t = ((AcDbText*)pEnt)->textString();
    text = t;
#ifndef _NCAD_BUILD_NODELETE
    acutDelString(t);
#endif
    return Acad::eOk;
  }
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
  {
    t = ((AcDbMText*)pEnt)->contents();
    text = t;
#ifndef _NCAD_BUILD_NODELETE
    acutDelString(t);
#endif
    return Acad::eOk;
  }
  return  Acad::eNotThatKindOfClass;
}

Acad::ErrorStatus rx_setTextString(AcDbEntity *pEnt, CString text)
{
  bool isRTF = !text.Left(5).CompareNoCase(_T("{\\rtf"));

  if (pEnt->isKindOf(AcDbText::desc()))
  {
    LPCTSTR s = ((AcDbText*)pEnt)->textString();
    if (text == s)
      return Acad::eOk;
    if (isRTF)
      text = _T("!rtf!");
    return ((AcDbText*)pEnt)->setTextString(text);
  }
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
  {
    AcDbMText* pMText = (AcDbMText*)pEnt;
    LPCTSTR s = pMText->contents();
    if (text != s)
    {
      if (isRTF)
      {
#ifdef _ACAD2000
        text = _T("!rtf!");
#else
        pMText->setContentsRTF(text);
        text = pMText->contents();
        //"\C0;ang1033{\H0.3333x;111\fSymath|b0|i0|c0|p0;\H1.5x;hang1049\fArial|b0|i0|c204|p0;\H0.6667x;\P}"
        text.Replace(_T("ang1049"), _T(""));
        text.Replace(_T("ang1033"), _T(""));
        text.Replace(_T("\\P}"), _T("}"));
#endif
        pMText->setContents(text);
      }
      else
      {
        LPCTSTR pFileFont = NULL;
        AcDbTextStyleTableRecord *pStyle;
        if (acdbOpenObject(pStyle, pMText->textStyle(), AcDb::kForRead) == Acad::eOk)
        {
          pStyle->fileName(pFileFont);
          pStyle->close();
        }
        if (pFileFont && -1 != text.Find(pFileFont) && !FileExt(pFileFont).CompareNoCase(_T("ttf")))
        {
          CString contents;
          contents += pMText->fontChange();
          contents += pFileFont;
          contents += ";";
          text = contents + text;
        }
        pMText->setContents(text);
      }
    }
    return Acad::eOk;
  }
  return Acad::eNotThatKindOfClass;
}

Acad::ErrorStatus rx_getTextStyleName(AcDbEntity *pEnt, CString& styleName)
{
  if (pEnt->isKindOf(AcDbText::desc())) 
    return ArxDbgUtils::symbolIdToName( ((AcDbText*)pEnt)->textStyle(), styleName);
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
    return ArxDbgUtils::symbolIdToName( ((AcDbMText*)pEnt)->textStyle(), styleName);
  else
    return  Acad::eNotThatKindOfClass;
}

Acad::ErrorStatus rx_setTextStyleName(AcDbEntity *pEnt, LPCTSTR styleName, AcDbDatabase* db)
{
  if (db == NULL) db = acdbHostApplicationServices()->workingDatabase();
  AcDbObjectId textStyleId;
  Acad::ErrorStatus es = ArxDbgUtils::nameToSymbolId( AcDbTextStyleTableRecord::desc(), styleName, textStyleId, db);
  if (es != Acad::eOk)
    return es;
  if (pEnt->isKindOf(AcDbText::desc())) 
    return ((AcDbText*)pEnt)->setTextStyle(textStyleId);
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
    return ((AcDbMText*)pEnt)->setTextStyle(textStyleId);
  else
    return  Acad::eNotThatKindOfClass;
}

Acad::ErrorStatus rx_updTextByStyle(AcDbText *pText)
{
  AcDbTextStyleTableRecord *pStyle;
  Acad::ErrorStatus es = acdbOpenObject(pStyle, pText->textStyle(), AcDb::kForRead);
  if (es == Acad::eOk)
  {
    double h = pStyle->textSize();
    if (h >= 0.1)
      pText->setHeight(h);

    pText->setOblique(pStyle->obliquingAngle());
    pText->setWidthFactor(pStyle->xScale());
    pStyle->close();
  }
  return es;
}


void rx_attachmentByAlign(AcDb::TextHorzMode hmode, AcDb::TextVertMode vmode, AcDbMText::AttachmentPoint& attPoint)
{
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kTopLeft;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kTopCenter;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kTopRight;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kMiddleLeft;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kMiddleCenter;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kMiddleRight;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kBottomLeft;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kBottomCenter;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kBottomRight;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextLeft)
    attPoint = AcDbMText::kBaseLeft;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextCenter)
    attPoint = AcDbMText::kBaseCenter;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextRight)
    attPoint = AcDbMText::kBaseRight;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextAlign)
    attPoint = AcDbMText::kBaseAlign;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextAlign)
    attPoint = AcDbMText::kBottomAlign;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextAlign)
    attPoint = AcDbMText::kMiddleAlign;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextAlign)
    attPoint = AcDbMText::kTopAlign;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextFit)
    attPoint = AcDbMText::kBaseFit;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextFit)
    attPoint = AcDbMText::kBottomFit;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextFit)
    attPoint = AcDbMText::kMiddleFit;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextFit)
    attPoint = AcDbMText::kTopFit;
  else 
  if (vmode == AcDb::kTextBase && hmode == AcDb::kTextMid)
    attPoint = AcDbMText::kBaseMid;
  else 
  if (vmode == AcDb::kTextBottom && hmode == AcDb::kTextMid)
    attPoint = AcDbMText::kBottomMid;
  else 
  if (vmode == AcDb::kTextVertMid && hmode == AcDb::kTextMid)
    attPoint = AcDbMText::kMiddleMid;
  else 
  if (vmode == AcDb::kTextTop && hmode == AcDb::kTextMid)
    attPoint = AcDbMText::kTopMid;
  else
  {
    RXASSERT( 1 <= attPoint && attPoint <= 23 );
    attPoint = AcDbMText::kMiddleCenter;
  }
}

void rx_alignByAttachment(AcDbMText::AttachmentPoint attPoint, AcDb::TextHorzMode& hmode, AcDb::TextVertMode& vmode)
{
  if (attPoint == AcDbMText::kTopLeft)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextLeft;
  }
  else
  if (attPoint == AcDbMText::kTopCenter)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextCenter;
  }
  else
  if (attPoint == AcDbMText::kTopRight)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextRight;
  }
  else
  if (attPoint == AcDbMText::kMiddleLeft)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextLeft;
  }
  else
  if (attPoint == AcDbMText::kMiddleCenter)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextCenter;
  }
  else
  if (attPoint == AcDbMText::kMiddleRight)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextRight;
  }
  else
  if (attPoint == AcDbMText::kBottomLeft)
  {
    vmode = AcDb::kTextBottom;
    hmode = AcDb::kTextLeft;
  }
  else
  if (attPoint == AcDbMText::kBottomCenter)
  {
    vmode = AcDb::kTextBottom;
    hmode = AcDb::kTextCenter;
  }
  else
  if (attPoint == AcDbMText::kBottomRight)
  {
    vmode = AcDb::kTextBottom;
    hmode = AcDb::kTextRight;
  }
  else
  if (attPoint == AcDbMText::kBaseLeft)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextLeft;
  }
  else
  if (attPoint == AcDbMText::kBaseCenter)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextCenter;
  }
  else
  if (attPoint == AcDbMText::kBaseRight)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextRight;
  }
  else
  if (attPoint == AcDbMText::kBaseAlign)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextAlign;
  }
  else
  if (attPoint == AcDbMText::kMiddleAlign)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextAlign;
  }
  else
  if (attPoint == AcDbMText::kTopAlign)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextAlign;
  }
  else
  if (attPoint == AcDbMText::kBaseFit)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextFit;
  }
  else
  if (attPoint == AcDbMText::kBottomFit)
  {
    vmode = AcDb::kTextBottom;
    hmode = AcDb::kTextFit;
  }
  else
  if (attPoint == AcDbMText::kMiddleFit)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextFit;
  }
  else
  if (attPoint == AcDbMText::kTopFit)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextFit;
  }
  else
  if (attPoint == AcDbMText::kBaseMid)
  {
    vmode = AcDb::kTextBase;
    hmode = AcDb::kTextMid;
  }
  else
  if (attPoint == AcDbMText::kBottomMid)
  {
    vmode = AcDb::kTextBottom;
    hmode = AcDb::kTextMid;
  }
  else
  if (attPoint == AcDbMText::kMiddleMid)
  {
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextMid;
  }
  else
  if (attPoint == AcDbMText::kTopMid)
  {
    vmode = AcDb::kTextTop;
    hmode = AcDb::kTextMid;
  }
  else
  {
    RXASSERT(AcDbMText::kTopLeft <= attPoint && attPoint <= AcDbMText::kTopMid);
    vmode = AcDb::kTextVertMid;
    hmode = AcDb::kTextCenter;
  }
}

Acad::ErrorStatus rx_getTextAttachment(const AcDbEntity *pEnt, AcDbMText::AttachmentPoint& attPoint)
{
  if (pEnt->isKindOf(AcDbText::desc())) 
    rx_attachmentByAlign( ((AcDbText*)pEnt)->horizontalMode(), ((AcDbText*)pEnt)->verticalMode(), attPoint );
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
    attPoint = ((AcDbMText*)pEnt)->attachment();
  else
    return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_setTextAttachment(AcDbEntity *pEnt, AcDbMText::AttachmentPoint attPoint)
{
  AcGePoint3d pos;
  rx_getTextPosition(pEnt, pos);

  if (pEnt->isKindOf(AcDbText::desc())) 
  {
    if ( !(AcDbMText::kTopLeft <= attPoint && attPoint <= AcDbMText::kTopMid) )
      attPoint = AcDbMText::kBaseLeft;
    AcDb::TextHorzMode hmode = AcDb::kTextCenter;
    AcDb::TextVertMode vmode = AcDb::kTextVertMid;
    rx_alignByAttachment( attPoint, hmode, vmode );
    
    //if ( ((AcDbText*)pEnt)->verticalMode() != vmode)
      ((AcDbText*)pEnt)->setVerticalMode(vmode);
    //if ( ((AcDbText*)pEnt)->horizontalMode() != vmode)
      ((AcDbText*)pEnt)->setHorizontalMode(hmode);
  }
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
  {
    if (attPoint == AcDbMText::kBaseLeft)
      attPoint = AcDbMText::kBottomLeft;
    else
    if (attPoint == AcDbMText::kBaseCenter)
      attPoint = AcDbMText::kBottomCenter;
    else
    if (attPoint == AcDbMText::kBaseRight)
      attPoint = AcDbMText::kBottomRight;
    else
    if (attPoint == AcDbMText::kBaseMid)
      attPoint = AcDbMText::kBottomCenter;
    else
    if (attPoint == AcDbMText::kBottomMid)
      attPoint = AcDbMText::kBottomCenter;
    else
    if (attPoint == AcDbMText::kMiddleMid)
      attPoint = AcDbMText::kMiddleCenter;
    else
    if (attPoint == AcDbMText::kTopMid)
      attPoint = AcDbMText::kTopCenter;

    if ( !(AcDbMText::kTopLeft <= attPoint && attPoint <= AcDbMText::kBottomRight))
      attPoint = AcDbMText::kMiddleCenter;
    //if ( ((AcDbMText*)pEnt)->attachment() != attPoint)
#if defined(_NCAD_BUILD_) && !defined(_NC24)
    try
    {
#endif
      ((AcDbMText*)pEnt)->setAttachment(attPoint);
#if defined(_NCAD_BUILD_) && !defined(_NC24)
  }
    catch (OdError e)
    {
      acutPrintf(_T("NCAD: pMText->setAttachment(%d) :%s"), (int)attPoint, (LPCTSTR)e.description());
    }
#endif
  }
  else
    return  Acad::eNotThatKindOfClass;
  return rx_setTextPosition(pEnt, pos);
}

CString rx_textAttachmentString(AcDbMText::AttachmentPoint attPoint)
{
  switch((int)attPoint)
  {
  case AcDbMText::kTopLeft     :  return _LCT("¬верх влево");
  case AcDbMText::kTopCenter   :  return _LCT("¬верх по центру");
  case AcDbMText::kTopRight    :  return _LCT("¬верх вправо");
  case AcDbMText::kMiddleLeft  :  return _LCT("—ередина влево");
  case AcDbMText::kMiddleCenter:  return _LCT("—ередина по центру");
  case AcDbMText::kMiddleRight :  return _LCT("—ередина вправо");
  case AcDbMText::kBottomLeft  :  return _LCT("¬низ влево");
  case AcDbMText::kBottomCenter:  return _LCT("¬низ по центру");
  case AcDbMText::kBottomRight :  return _LCT("¬низ вправо");
  case AcDbMText::kBaseLeft    :  return _LCT("Ѕаза влево");
  case AcDbMText::kBaseCenter  :  return _LCT("Ѕаза по центру");
  case AcDbMText::kBaseRight   :  return _LCT("Ѕаза вправо");
  case AcDbMText::kBaseAlign   :
  case AcDbMText::kBottomAlign :  return _LCT("");
  case AcDbMText::kMiddleAlign :  return _LCT("");
  case AcDbMText::kTopAlign    :  return _LCT("");
  case AcDbMText::kBaseFit     :  return _LCT("");
  case AcDbMText::kBottomFit   :  return _LCT("");
  case AcDbMText::kMiddleFit   :  return _LCT("");
  case AcDbMText::kTopFit      :  return _LCT("");
  case AcDbMText::kBaseMid     :  return _LCT("");
  case AcDbMText::kBottomMid   :  return _LCT("");
  case AcDbMText::kMiddleMid   :  return _LCT("ѕо геометр. центру");
  case AcDbMText::kTopMid      :  return _LCT("");
  default:                      
    return _T("");
  }
}

void rx_setTextWidthFactorByMaxWidth(AcDbText* pText, double maxWidth, double defWidthFactor)
{
  if (!pText || maxWidth < 1.0)
    return;

  AcDbMText::AttachmentPoint attPoint;
  rx_attachmentByAlign(pText->horizontalMode(), pText->verticalMode(), attPoint);
  if (attPoint == AcDbMText::kBaseFit   ||
    attPoint == AcDbMText::kBottomFit ||
    attPoint == AcDbMText::kMiddleFit ||
    attPoint == AcDbMText::kTopFit)
    return;

  CString text = (LPCTSTR)pText->textString();
  text.TrimLeft();
  text.TrimRight();
  if (!text.IsEmpty())
  {
    AcGePoint3d pt1, pt2, ptPos = AcGePoint3d::kOrigin;
    AcDbExtents ext;
    double d = pText->rotation(), e = 0.01, widthFactor = pText->widthFactor();

    if (defWidthFactor < e)
    {
      AcDbTextStyleTableRecord* pStyle;
      if (acdbOpenObject(pStyle, pText->textStyle(), AcDb::kForRead) == Acad::eOk)
      {
        defWidthFactor = pStyle->xScale();
        pStyle->close();
      }
    }
    if (defWidthFactor < e)
      defWidthFactor = 1.0;

    if ( fabs(widthFactor - defWidthFactor) < e && ((0-e <= d && d <= 0+e) || (PI-e <= d && d <= PI+e)) && pText->getGeomExtents(ext) == Acad::eOk)
      d = ext.maxPoint().x - ext.minPoint().x;
    else
    if ( fabs(widthFactor - defWidthFactor) < e && ((PI/2-e <= d && d <= PI/2+e) || (3*PI/2-e <= d && d <= 3*PI/2+e)) && pText->getGeomExtents(ext) == Acad::eOk)
      d = ext.maxPoint().x - ext.minPoint().x;
    else
    if ( rx_textBox(&pt1, &pt2, text, pText->textStyle(), pText->height(), defWidthFactor, pText->oblique()))
      d = pt2.x - pt1.x;
    else
      return;
    if ( d > maxWidth )
    {
      widthFactor = defWidthFactor * maxWidth / d;
      if (widthFactor < 0.4) widthFactor = 0.4;
      if (widthFactor > defWidthFactor) widthFactor = defWidthFactor;
      pText->setWidthFactor(widthFactor);
    }
  }
}


Acad::ErrorStatus rx_makeLine(const AcGePoint3d& startPt, const AcGePoint3d& endPt, 
                              int colorIndex /*= 256*/ , bool HighLight /*=false*/, AcDbObjectId *pObjId /*=NULL*/)
{
    Acad::ErrorStatus es = Acad::eOk;
    AcDbLine *pLine = new AcDbLine(startPt, endPt);
    pLine->setColorIndex(colorIndex);
    AcDbObjectId objId;
    es = postToDb(pLine, objId);
    if ( es != Acad::eOk ) delete pLine;
    else 
    {
      if ( pObjId ) *pObjId = objId;
      if ( HighLight && acdbOpenObject(pLine, objId, AcDb::kForRead) == Acad::eOk)
      {
        pLine->highlight();
        pLine->close();
      }
    }
    return es;
}


Acad::ErrorStatus rx_makePline(const AcGePoint3dArray&  pts,
                               bool closed /*= false*/,
                               int colorIndex /*= 256*/,
                               AcDbObjectId *objId /*=NULL*/)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGePoint2d pt;
    AcDbPolyline *pPline = new AcDbPolyline(pts.length());
    pPline->setColorIndex(colorIndex);
    for (int i = 0; i < pts.length(); i++) {
        pt = asPnt2d(asDblArray(pts[i]));
        pPline->addVertexAt(i, pt );
    }
    pPline->setClosed(closed);
    if ( objId != NULL )
      es = postToDb(pPline, *objId);
    else
      es = postToDb(pPline);
    if ( es != Acad::eOk ) 
      delete pPline;
    return es;
}

Acad::ErrorStatus rx_makeSpline(const AcGePoint3dArray&  pts,
			              AcDbSpline*&       pSpline)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGeDoubleArray knots, weights;
    for (int i = 0; i < pts.length(); i++) {
        weights.append(1.0);
    }

    getUniformKnots(pts.length(), 1, 0, knots);

    pSpline = new AcDbSpline(1, Adesk::kFalse, Adesk::kFalse,
                                Adesk::kFalse, pts, knots, weights);
 
    return es;
}

int getcYesOrNo(LPCTSTR promptStr, bool& answer, int init /*= 1*/)
{
    TCHAR option[131];
    acedInitGet(init, _T("Yes No ƒа Ќет"));
    int es = acedGetKword(promptStr, option);
    if (es == RTNORM)
      answer = ((_tcsicmp(option, _T("ƒа")) == 0) || (_tcsicmp(option, _T("Yes")) == 0));
    return es;
}

bool getPoint3d(LPCTSTR prompt, AcGePoint3d& pt, AcGePoint3d* pFrom,
                int snapmode /*= -1*/, bool waitCan /*= true*/, bool bUseDashedLine /*= true*/,
                int *Ret /*= NULL*/)
{
  int iRet, snapmodeS;

  if (snapmode != -1 )
  {
    getSysVar(AcadVar::snapmode, snapmodeS);
    setSysVar(AcadVar::snapmode, snapmode);
  }

  //_LCT("”кажите точку")
  CString msg = CARETCOLON(prompt);

  int initFlag = 0;
  //if (waitCan)
  //  initFlag += RSG_NONULL;
  initFlag += RSG_NOLIM;
  if (bUseDashedLine)
    initFlag += RSG_DASH;

  do 
  {
    acedInitGet(initFlag, NULL);
    iRet = acedGetPoint( (pFrom ? &(pFrom->x) : NULL), msg.IsEmpty() ? NULL : (LPCTSTR )msg, &pt.x);
  }
  while ( waitCan && (iRet != RTNORM && iRet != RTCAN) );

  if (snapmode != -1 )
    setSysVar(AcadVar::snapmode, snapmodeS);

  if (Ret)
    *Ret = iRet;
  return iRet == RTNORM;
}

bool rx_getCorner(LPCTSTR  prompt, AcGePoint3d& pt1, AcGePoint3d& pt2, int snapmode /*= -1*/, 
                  bool useStartPoint /*=false*/, int fInitGet /*= RSG_NONULL|RSG_DASH*/, int *pRet /*= NULL*/)
{
  int snapmodeS, iRet = RTCAN;
  if (snapmode != -1 )
  {
    getSysVar(AcadVar::snapmode, snapmodeS);
    setSysVar(AcadVar::snapmode, snapmode);
  }

  if (prompt) 
  { 
    PRINT_CARET 
    acutPrintf(prompt);
  }

  PRINT_CARET 
  CString msg1 = _LCT("”кажите 1-ю точку: ");
  CString msg2 = _LCT("”кажите 2-ю точку: ");

  acedInitGet( fInitGet, NULL);
  if (useStartPoint || (iRet = acedGetPoint( NULL, msg1, asDblArray(pt1))) == RTNORM)
    iRet = acedGetCorner( asDblArray(pt1), msg2, asDblArray(pt2));

  if (snapmode != -1 )
    setSysVar(AcadVar::snapmode, snapmodeS);

  if (pRet) *pRet = iRet;

  return (iRet == RTNORM);
}

int rx_findEndPoint(ads_point oldpt, ads_point newpt) 
{ 
    ads_point ptres; 
    int foundpt; 
    foundpt = acedOsnap(oldpt, _T("end"), ptres); 
    if (foundpt == RTNORM) { 
        ads_point_set(ptres, newpt); 
    } 
    return foundpt; 
} 

//-----------------------------------------------------------------------------


bool rx_isLockCurrentLayer(bool alert /*= true*/)
{
  bool isLocked = false;
  AcDbObject* obj;
  AcDbLayerTableRecord* layer;
  if (acdbOpenAcDbObject(obj, acdbHostApplicationServices()->workingDatabase()->clayer(), AcDb::kForRead) == Acad::eOk) 
  {
    layer = AcDbLayerTableRecord::cast(obj);
    if (layer)
      isLocked = layer->isLocked();
    else
      ASSERT(0);
    if ( isLocked && alert )
       ArxDbgUtils::alertBox(_LCT("“екущий слой блокирован."));
    obj->close();
  }
  else
    ASSERT(0);

  return isLocked;
}

Acad::ErrorStatus rx_createLayer(/*[in]*/LPCTSTR  layerName, /*[out]*/ AcDbObjectId& id)
{
	Acad::ErrorStatus es;
	id = AcDbObjectId::kNull;
	AcDbLayerTable* pTable;

	if ((es = acdbHostApplicationServices()->workingDatabase()->
                              getLayerTable(pTable, AcDb::kForRead)) == Acad::eOk)
	{
		// use the overload of AcDbLayerTable::getAt() that returns the id
		if ((es = pTable->getAt(layerName, id, Adesk::kFalse)) != Acad::eOk)
		{
			// create a new layer table record using the layer name passed in
			AcDbLayerTableRecord* pLTRec = new AcDbLayerTableRecord;
			if (pLTRec)
			{
				pLTRec->setName(layerName);
				// set other properties - color, linetype, state - if desired
				// this will require more input than this simple example provides
				if ((es = pTable->upgradeOpen()) == Acad::eOk)
				{
					es = pTable->add(id, pLTRec);
					// since the new layer was successfully added to the database,
					// close it - DON'T delete it
					pLTRec->close();
				}
				else
					delete pLTRec;
			}
			else
				es = Acad::eOutOfMemory;
		}
		pTable->close();
	}
	return es;
}

Acad::ErrorStatus rx_setHeightTextStyle(const AcDbObjectId &textStyleId, double Height)
{
  Acad::ErrorStatus es;
  AcDbTextStyleTableRecord* rec;
  es = acdbOpenObject(rec, textStyleId, AcDb::kForWrite);
  if ( es == Acad::eOk )
  {
    rec->setTextSize(Height);
    rec->close();
  }
  return es;
}

Acad::ErrorStatus rx_getHeightTextStyle(const AcDbObjectId &textStyleId, double &Height)
{
  Acad::ErrorStatus es;
  AcDbTextStyleTableRecord* rec;
  es = acdbOpenObject(rec, textStyleId, AcDb::kForRead);
  if ( es == Acad::eOk )
  {
    Height =rec->textSize();
    rec->close();
  }
  return es;
}

Acad::ErrorStatus rx_updTextStyle(LPCTSTR textStyleName, 
                                  LPCTSTR fontFile, 
                                  double textSize,
                                  double xScale,
                                  double obliquingAngle,
                                  AcDbDatabase* pDwg /*=NULL*/,
                                  bool *pIsChange /*= NULL*/)
{
  if (pDwg == NULL) 
    pDwg = acdbHostApplicationServices()->workingDatabase();
  if (pIsChange)
    *pIsChange = false;

  Acad::ErrorStatus es;
  AcDbTextStyleTableRecord* rec;
  AcDbTextStyleTable* tsTbl;

  es = pDwg->getTextStyleTable(tsTbl, AcDb::kForRead); 

  if (!tsTbl) { ASSERT(0);  return es; }

  if (!tsTbl->has(textStyleName)) 
  {
    es = tsTbl->upgradeOpen();
    if ( es != Acad::eOk) {
      tsTbl->close();
      return es;
    }
    AcDbTextStyleTableRecord* rec = new AcDbTextStyleTableRecord;
    
    if ( (es = rec->setName(textStyleName)) != Acad::eOk || (es = tsTbl->add(rec)) != Acad::eOk) {
      delete rec;
      tsTbl->close();
      return es;
    }
    rec->close();
  }

  es = tsTbl->getAt(textStyleName, rec, AcDb::kForRead);
  tsTbl->close();
  if (es != Acad::eOk)
    return es;

  const ACHAR* fontFileC = NULL;
  es = rec->fileName(fontFileC);
  if (es != Acad::eOk)
  {
    rec->close();
    return es;
  }
  //double ang =  italic ? 15.0*(PI/180.0) : 0.0;

  if (!fontFileC || !*fontFileC ||
      (fontFile && *fontFile && fontFileC && *fontFileC && _tcsicmp(fontFileC, fontFile)) ||
      fabs(obliquingAngle - rec->obliquingAngle()) > AcGeContext::gTol.equalPoint() ||
      fabs(textSize-rec->textSize()) > AcGeContext::gTol.equalPoint() ||
      fabs(xScale-rec->xScale()) > AcGeContext::gTol.equalPoint())
  {
      if ((es=rec->upgradeOpen()) != Acad::eOk || 
          (fontFile && *fontFile && (es = rec->setFileName(fontFile)) != Acad::eOk)) {
        rec->close();
        ArxDbgUtils::rxErrorMsg(es);
        return es;
      }
      rec->setBigFontFileName(_T(""));    // must explicitly set to ""
      if (textSize >= 0.1) rec->setTextSize(textSize);
      if (0.1 <= xScale && xScale < 50) rec->setXScale(xScale);
      if (0.0 <= obliquingAngle && obliquingAngle < 2*PI) rec->setObliquingAngle(obliquingAngle);
      if (pIsChange)
        *pIsChange = true;
  }
  rec->close();
  return es;
}

#ifdef ARXDBG_UTILS_H

Acad::ErrorStatus rx_updLayerColor(LPCTSTR  layerName, int colorIndex, 
                                   int* upd /*= NULL*/, AcDbDatabase* pDwg /*=NULL*/)
{
  if (pDwg == NULL)
    pDwg = acdbHostApplicationServices()->workingDatabase();
  RXASSERT(pDwg != NULL);

  Acad::ErrorStatus es;
  ArxDbgUtils::addNewLayer(layerName, pDwg);
  AcDbObjectId objId;
  es = rx_getLayerId(layerName, objId, false, pDwg);
  if ( es == Acad::eOk )
  {
    AcDbLayerTableRecord *pLayerTblRcd;
    es = acdbOpenObject(pLayerTblRcd, objId, AcDb::kForRead);
    if ( es == Acad::eOk )
    {
      if ( pLayerTblRcd->color().colorIndex() != colorIndex )
      {
         es = pLayerTblRcd->upgradeOpen();
         if ( es == Acad::eOk )
         {
           AcCmColor color;
           color.setColorIndex(colorIndex);
           pLayerTblRcd->setColor(color);
           if (upd) (*upd)++;
         }
      }
      pLayerTblRcd->close();
    }
  }
  return es;
}

Acad::ErrorStatus rx_updLayerPlottable(LPCTSTR  layerName, bool bPlottable, 
                                   int* upd /*= NULL*/, AcDbDatabase* pDwg /*=NULL*/)
{
  if (pDwg == NULL)
    pDwg = acdbHostApplicationServices()->workingDatabase();
  RXASSERT(pDwg != NULL);

  Acad::ErrorStatus es;
  ArxDbgUtils::addNewLayer(layerName, pDwg);
  AcDbObjectId objId;
  es = rx_getLayerId(layerName, objId, false, pDwg);
  if ( es == Acad::eOk )
  {
    AcDbLayerTableRecord *pLayerTblRcd;
    es = acdbOpenObject(pLayerTblRcd, objId, AcDb::kForRead);
    if ( es == Acad::eOk )
    {
      if ( pLayerTblRcd->isPlottable() != bPlottable )
      {
         es = pLayerTblRcd->upgradeOpen();
         if ( es == Acad::eOk )
         {
           pLayerTblRcd->setIsPlottable(bPlottable);
           if (upd) (*upd)++;
         }
      }
      pLayerTblRcd->close();
    }
  }
  return es;
}
#endif // define ARXDBG_UTILS_H

int rx_acadColorDlg ( short *color , bool Logic)
{
  if ( 0 > *color || *color > 256 ) *color = 7; 
  if ( !Logic && (*color == 0 || *color == 256) ) *color = 7; 
  struct resbuf *arglist, *rslt=NULL; 
  int rc; 
  arglist = acutBuildList(RTSTR, _T("acad_colordlg"), RTSHORT, *color, Logic? RTT : RTNIL, 0 ); 
  if (arglist == NULL) 
	  return RTERROR;
  rc = acedInvoke(arglist, &rslt); 
  if (rc == RTNORM && rslt != NULL) {
  	  *color = rslt->resval.rint; 
      return RTNORM;
  }
  return RTERROR;
}

// Set background to AutoCAD's Client workspace color.
bool rx_acedCurGraphWndBackColor(AcCmEntityColor& backcolor, AcDbDatabase* pDwg)
{
  if (!pDwg) pDwg = WORKDWG;  if (!pDwg) return FALSE;
	AcColorSettings pAcadColors;
	if (!acedGetCurrentColors(&pAcadColors))
    return FALSE;
  DWORD backcolorDW = ArxDbgUtils::isPaperSpace(pDwg) ? 
            pAcadColors.dwGfxLayoutBkColor : pAcadColors.dwGfxModelBkColor;
 #ifndef _ACAD2021
	backcolor.setColorMethod(AcCmEntityColor::kByColor);
#endif // _ACAD2021
  backcolor.setRGB( GetRValue(backcolorDW),GetGValue(backcolorDW),GetBValue(backcolorDW) );
  return TRUE;
}

#ifndef _ACAD2000
void rx_DrawOrderInherit(const AcDbObjectId& objId, AcEdDrawOrderCmdType cmd)
{
  AcDbObjectId curViewId = acedGetCurViewportObjectId();
  AcDbObjectIdArray objIdAr;
  objIdAr.append(objId);
  if (acedDrawOrderInherit(curViewId, objIdAr, cmd) != Acad::eOk)
    rxDPRINT(_T("ќшибка acedDrawOrderInherit"));
}
#else
  void rx_DrawOrderInherit(const AcDbObjectId& objId, int cmd)
  {
  }
#endif


//-----------------------------------------------------------------------------
// selectEntity - cервисные функции выбора примитива
//
// ¬ыбор примитива
//
AcDbObjectId
selectEntity()
{
  int snapmodeS;
  getSysVar(AcadVar::snapmode, snapmodeS);
  setSysVar(AcadVar::snapmode, 0);

	AcDbObjectId eId;
  ads_name en;
  ads_point pt;
  int rc = acedEntSel(NULL, en, pt);

  if (rc != RTNORM) {
      //PRINT_SPACE
      acutPrintf(_LCT("Ќичего не выбрано."), rc);
      return AcDbObjectId::kNull;
  }

  AOK(acdbGetObjectId(eId, en));

  setSysVar(AcadVar::snapmode, snapmodeS);
	return eId;
}

AcDbEntity*
selectEntity(AcDbObjectId& eId, LPCTSTR  msg, AcDb::OpenMode openMode, AcGePoint3d &pt, int snapmode /*= -1*/)
{
  int snapmodeS;
  if (snapmode != -1 )
  {
    getSysVar(AcadVar::snapmode, snapmodeS);
    setSysVar(AcadVar::snapmode, snapmode);
  }

  AcDbEntity * ent = NULL;
  ads_name en;
  if ( acedEntSel(CARETCOLON(msg), en, asDblArray(pt)) == RTNORM) 
  {
      acedOsnap(asDblArray(pt), _T("_nea"), asDblArray(pt)); 
      AOK(acdbGetObjectId(eId, en));
      AOK(acdbOpenObject(ent, eId, openMode));
  }
  else
  {
    acutPrintf(_LCT("Ќичего не выбрано."));
    //acutPrintf(_T("Nothing selected.\n"), rc);
    //PRINT_CARET
  }

  if (snapmode != -1 )
    setSysVar(AcadVar::snapmode, snapmodeS);

  return ent;
}

AcDbEntity*
selectEntity(AcDbObjectId& eId, LPCTSTR  msg, AcDb::OpenMode openMode)
{
  AcGePoint3d pt;
  return selectEntity(eId, msg, openMode, pt, 0);
}

AcDbEntity*
selectEntity(LPCTSTR  msg, AcDb::OpenMode openMode)
{
    AcDbObjectId eId;
    return selectEntity(eId, msg, openMode);
}

AcDbEntity*
selectEntityPoint(AcDbObjectId& eId, LPCTSTR  msg, AcDb::OpenMode openMode, AcGePoint3d &pt, int snapmode /*= -1*/)
{
  ads_name en, sset;
  Adesk::Int32 slen; //long
  if ( SSGetByPoint( msg, pt, sset, slen, snapmode) == RTNORM) 
  {  
    if ( slen > 1 ) 
    {
      PRINT_CARET
      acutPrintf(_LCT("“очка принадлежит более, чем одному примитиву. ”кажите один.")); 
    }
    else 
    if (acedSSName(sset, 0, en) == RTNORM) 
    {
      acedSSFree(sset);
      AOK(acdbGetObjectId(eId, en));
      AcDbEntity * ent;
      AOK(acdbOpenObject(ent, eId, openMode));
      return ent;
    }
    acedSSFree(sset);
  }
  return NULL;
}

AcDbEntity* getOneEntityInPoint(const AcGePoint3d &pt, AcDb::OpenMode openMode)
{
  AcDbObjectId eId;
  ads_name en, sset;
  Adesk::Int32 slen; //long
  AcDbEntity * ent = NULL;
  //if ( acedOsnap(asDblArray(pt), _T("_nea"), asDblArray(pt) ) == RTNORM)
    if (acedSSGet(_T(":E"), asDblArray(pt), NULL, NULL, sset) == RTNORM) 
    {  
      acedSSLength(sset, &slen);
      if ( slen < 1 ) 
      {
        //PRINT_SPACE
        acutPrintf(_LCT("Ќичего не выбрано.")); 
      }
      else 
      if ( slen > 1 ) 
      {
        PRINT_CARET
        acutPrintf(_LCT("“очка принадлежит более, чем одному примитиву. ”кажите один.")); 
      }
      else 
      if (acedSSName(sset, 0, en) == RTNORM) 
      {
        acedSSFree(sset);
        AOK(acdbGetObjectId(eId, en));
        AOK(acdbOpenObject(ent, eId, openMode));
      }
      acedSSFree(sset);
    }
  return ent;
}

//-----------------------------------------------------------------------------
int SSGetByPick(const struct resbuf* filter, ads_name &sset, Adesk::Int32 &slen)
{
  int ret = RTNORM;
  if (acedSSGet(_T("_I"), NULL, NULL, filter, sset) != RTNORM) 
    if (acedSSGet(NULL, NULL, NULL, filter, sset) != RTNORM) 
      ret = RTERROR;
  if (ret == RTNORM)
    acedSSLength(sset, &slen);
  return ret;
}

int SSGetByPoint(LPCTSTR  msg, AcGePoint3d &pt, ads_name &sset, Adesk::Int32 &slen, int snapmode /*= -1*/ )
{
  int snapmodeS, ret = RTERROR;
  if (snapmode != -1 )
  {
    getSysVar(AcadVar::snapmode, snapmodeS);
    setSysVar(AcadVar::snapmode, snapmode);
  }
  slen = 0;
  if ( (ret = acedGetPoint(NULL, CARETCOLON(msg), asDblArray(pt))) == RTNORM) 
    if ( (ret = acedOsnap(asDblArray(pt), _T("_nea"), asDblArray(pt))) == RTNORM)
    {
      if ((ret = acedSSGet(_T(":E"), asDblArray(pt), NULL, NULL, sset)) == RTNORM) 
        acedSSLength(sset, &slen);
      if ( slen < 1 ) 
      {
        acutPrintf(_LCT("Ќичего не выбрано."));
        //PRINT_CARET
      }
    }
    else
    {
      AcGePoint3d pt1(pt.x-1, pt.y-1, pt.z), pt2(pt.x+1, pt.y+1, pt.z);
      if ((ret = acedSSGet(_T("_C"), asDblArray(pt1), asDblArray(pt2), NULL, sset)) == RTNORM) 
        acedSSLength(sset, &slen);
      if ( slen < 1 ) 
      {
        //PRINT_SPACE
        acutPrintf(_LCT("Ќичего не выбрано."));
      }
    }

  if (snapmode != -1 )
    setSysVar(AcadVar::snapmode, snapmodeS);
  return ret;
}

int SSGetByPoint(LPCTSTR  msg, AcGePoint3d &pt, AcDbObjectIdArray &objIdAr, int snapmode /*= -1*/ )
{
  ads_name sset, en;
  Adesk::Int32 slen = 0; //long
  int ret = SSGetByPoint(msg, pt, sset, slen, snapmode );
  if ( ret != RTNORM )
    return ret;
  AcDbObjectId eId;
  for ( int i = 0; i < slen; i++)
    if (acedSSName(sset, i, en) == RTNORM) 
      if (acdbGetObjectId(eId, en) == Acad::eOk )
         objIdAr.append(eId);
  acedSSFree(sset);
  return ret;
}

AcDbEntity*	selectNEntity(LPCTSTR  msg, AcDb::OpenMode openMode, AcGePoint3d &pt, int pickflag, int snapmode /*= 0*/)
{
  int snapmodeS, ret = RTERROR;
  if (snapmode != -1 )
  {
    getSysVar(AcadVar::snapmode, snapmodeS);
    setSysVar(AcadVar::snapmode, snapmode);
  }

  ads_name entres;
  ads_matrix xformres;
  struct resbuf* refstkres;

  AcDbEntity *pEnt = NULL;
  if ( acedNEntSelP( CARETCOLON(msg), entres, asDblArray(pt), pickflag, xformres, &refstkres ) == RTNORM) 
  {  
    if ( refstkres != NULL ) acutRelRb (refstkres);
    AcDbObjectId objId;
    AOK(acdbGetObjectId(objId, entres));
    if (!objId.isNull())
      AOK(acdbOpenObject(pEnt, objId, openMode));
  }
  if (snapmode != -1 )
    setSysVar(AcadVar::snapmode, snapmodeS);
  return pEnt;
}

//-----------------------------------------------------------------------------
Acad::ErrorStatus RTtoStatus (int rt) {
#ifndef ACDBLIB
	//----- ObjectDBX does not have ADS functions and ADS error codes defined
	switch ( rt ) {
		case RTNORM: //----- 5100 - Request succeeded
			return (Acad::eOk) ;
		case RTERROR: //----- -5001 - Some other error
			return (Acad::eUnrecoverableErrors) ;
		case RTCAN: //----- -5002 - User cancelled request -- Ctl-C
			return (Acad::eUserBreak) ;
		case RTREJ: //----- -5003 - AutoCAD rejected request -- invalid
			return (Acad::eInvalidInput) ;
		case RTFAIL: //----- -5004 - Link failure -- Lisp probably died
			return (Acad::eNotApplicable) ;
		default:
		//case RTKWORD: //----- -5005 - Keyword returned from getxxx() routine
			//----- this function only intended to be called 
			//----- in an error checking situation. See ADSOK.
			RXASSERT ( FALSE ) ;
			return (Acad::eOk) ;
	}
#endif
	return (Acad::eOk);
}


//*******************************************************************//
//от Ћеонида из CS
UINT rx_EnumerateEntityesBTR (AcDbBlockTableRecord* pBTR, RXEnumProc::EnumProc pEnumProc,
                              LPVOID param/*= NULL*/, LPCTSTR  messProgress /*= NULL*/)
{
  if (!pBTR     ) {  rxASSERT0(FALSE, _T("Ќе определена запись таблицы блоков!"));  return 0;  }
  if (!pEnumProc) {  rxASSERT0(FALSE, _T("Ќе определена процедура перечислени€!")); return 0;  }

  AcDbBlockTableRecordIterator *pBlockIterator = NULL;
  //Acad::ErrorStatus es = pBTR->newIterator(pBlockIterator);
  rxIfAcASSERT( pBTR->newIterator(pBlockIterator), _T("pBTR->newIterator")) return 0;

  UINT i = 0, count = 0;
  BOOL done;
  if ( messProgress )
  {
    for (done = FALSE; (!done) && (!pBlockIterator->done()); pBlockIterator->step()) i++;
    pBlockIterator->start();
    acedSetStatusBarProgressMeter(messProgress, 0, i);
    i = 0;
  }
  AcDbObjectId objId;
  RXEnumProc::EnumCode enumCode;
  for (done = FALSE; (!done) && (!pBlockIterator->done()); pBlockIterator->step())
  {
    Acad::ErrorStatus es = pBlockIterator->getEntityId(objId);
    if (es != Acad::eOk)   continue;
    //rxIfAcASSERT(pBlockIterator->getEntityId(objId), _T("m_pBlockIterator->getEntityId"))  continue;
    enumCode = pEnumProc(objId, param);
    switch(enumCode)
    {
      case RXEnumProc::eEnumCancel   : done = TRUE; break;
      case RXEnumProc::eEnumAdd      : count++;  break;
      case RXEnumProc::eEnumContinue : break;
      default                      : rxASSERT0(FALSE, _T("Ќеправильный перечислимый тип!")); break;
    }
    if ( messProgress ) acedSetStatusBarProgressMeterPos(i++);
  }
#ifndef _NCAD_BUILD_NODELETE
  delete pBlockIterator;
#endif
  if ( messProgress ) acedRestoreStatusBar();
  return count;
}

/*******************************************************************/
UINT rx_EnumerateEntityes (RXEnumProc::EnumProc pEnumProc, LPVOID param/*= NULL*/, AcDbDatabase* pDb/*= NULL*/, 
                           RXEnumProc::EnumArea where/*= RXEnumProc::eCurrentSpace*/, LPCTSTR  messProgress /*= NULL*/)
{
  if (!pEnumProc) {  rxASSERT0(FALSE, _T("Ќе определена процедура перечислени€!")); return 0;  }

  if (!pDb) pDb = acdbHostApplicationServices()->workingDatabase();

  if (where == RXEnumProc::eCurrentSpace)
  {
    if (ArxDbgUtils::isPaperSpace(pDb)) 
      where = RXEnumProc::ePaperSpace;
    else                
      where = RXEnumProc::eModelSpace;
  }

  UINT count = 0;
  AcDbBlockTable *pBlockTable;
  Acad::ErrorStatus es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
  rxIfAcASSERT(es, _T("pDatabase->getBlockTable"))
    return FALSE;
  AcDbBlockTableRecord *pBlockTableRecord;
  if (where == RXEnumProc::eModelSpace || where == RXEnumProc::eModelAndPaperSpace)
  {
    pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForRead);
    count += rx_EnumerateEntityesBTR(pBlockTableRecord, pEnumProc, param, messProgress);
    pBlockTableRecord->close();
  }
  if (where == RXEnumProc::ePaperSpace || where == RXEnumProc::eModelAndPaperSpace)
  {
    pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord, AcDb::kForRead);
    count += rx_EnumerateEntityesBTR(pBlockTableRecord, pEnumProc, param, messProgress);
    pBlockTableRecord->close();
  }
  pBlockTable->close();
  return count;
}

//----------------------------------------------------------------------------------------------
// Ћичное сочинение
//----------------------------------------------------------------------------------------------

UINT rx_collectEntites(AcDbBlockTableRecord *pBTR, AcDbObjectIdArray& objIds, 
                      AcRxClass* rxClass /*= NULL*/, bool bEqClass /*= true*/)
{
  if (!pBTR     ) {  rxASSERT0(FALSE, _T("Ќе определена запись таблицы блоков!"));  return 0;  }

  AcDbBlockTableRecordIterator *pBlockIterator;
  Acad::ErrorStatus es = pBTR->newIterator(pBlockIterator);
  rxIfAcASSERT(es, _T("pBTR->newIterator")) return 0;

  AcDbObject *pObj;
  AcDbObjectId objId;
  bool bAdd = true;
  for (;!pBlockIterator->done(); pBlockIterator->step())
  {
    es = pBlockIterator->getEntityId(objId);
    rxIfAcASSERT(es, _T("m_pBlockIterator->getEntityId")) continue;
    if (rxClass)
    {
      pObj = NULL;
      es = acdbOpenObject(pObj, objId, AcDb::kForRead);
      if (es != Acad::eOk)
      {
        if (pObj) pObj->close();
        rxASSERT0(FALSE, _T("ќшибка обращени€ к объекту!"));
        continue;
      }

      if (bEqClass)
        bAdd = pObj->isA() == rxClass;
      else
        bAdd = pObj->isKindOf(rxClass);
      pObj->close();
    }
    if (bAdd) objIds.append(objId);
  }
#ifndef _NCAD_BUILD_NODELETE
  delete pBlockIterator;
#endif
  return objIds.length();
}

UINT rx_collectEntites(AcDbObjectIdArray& objIds, 
                      AcRxClass* rxClass /*= NULL*/,
                      bool bEqClass /*= true*/,
                      RXEnumProc::EnumArea where /*= RXEnumProc::eCurrentSpace*/,
                      AcDbDatabase* pDb /*= NULL*/)
{
  if (!pDb) pDb = acdbHostApplicationServices()->workingDatabase();

  if (where == RXEnumProc::eCurrentSpace)
  {
    if (ArxDbgUtils::isPaperSpace(pDb)) 
      where = RXEnumProc::ePaperSpace;
    else                
      where = RXEnumProc::eModelSpace;
  }

  UINT count = 0;
  AcDbBlockTable *pBlockTable;
  Acad::ErrorStatus es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
  rxIfAcASSERT(es, _T("pDatabase->getBlockTable"))
    return 0;
  AcDbBlockTableRecord *pBlockTableRecord;
  if (where == RXEnumProc::eModelSpace || where == RXEnumProc::eModelAndPaperSpace)
  {
    pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForRead);
    count += rx_collectEntites(pBlockTableRecord, objIds, rxClass, bEqClass);
    pBlockTableRecord->close();
  }
  if (where == RXEnumProc::ePaperSpace || where == RXEnumProc::eModelAndPaperSpace)
  {
    pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord, AcDb::kForRead);
    count += rx_collectEntites(pBlockTableRecord, objIds, rxClass, bEqClass);
    pBlockTableRecord->close();
  }
  pBlockTable->close();
  return count;
}


bool rx_lastId(AcDbObjectId& objId)
{
  ads_name ename;
  return (acdbEntLast(ename) == RTNORM && acdbGetObjectId(objId, ename) == Acad::eOk);
}


bool rx_collectEntites (AcDbObjectIdArray &objList, const AcDbObjectId &begId, bool ExludeFirst /*= false*/, bool OnlyMain /*= true*/ )

{
  //ads_name beg_aname;
  //acdbNameClear(beg_aname);
  //if (!begId.isNull() && acdbGetAdsName(beg_aname, begId) != Acad::eOk)
  //{
  //  ASSERT(FALSE);
  //  return FALSE;
  //}
  //return rx_collectEntites(objList, beg_aname, ExludeFirst, OnlyMain);

  objList.setLogicalLength(0);

  AcDbObjectId ownerId;
  AcDbDatabase* pDb;

  bool bUseBegId = begId.isValid();
  if (bUseBegId)
  {
    pDb = begId.database();
    if (pDb)
    {
      AcDbEntityPtr pEnt;
      if (rxOpenObject(pEnt, begId, AcDb::kForRead, true) == Acad::eOk)
        ownerId = pEnt->ownerId();
    }
  }

  if (ownerId.isNull())
  {
    ExludeFirst = false;
    pDb = WORKDWG;
    ownerId = pDb->currentSpaceId();
    if (ownerId.isNull())
    {
      AcDbBlockTable *pBlockTable = NULL;
      if (pDb->getSymbolTable(pBlockTable, AcDb::kForRead) == Acad::eOk)
      {
        LPCTSTR pSpace = ArxDbgUtils::isPaperSpace(pDb) ? ACDB_PAPER_SPACE : ACDB_MODEL_SPACE;
        pBlockTable->getAt(pSpace, ownerId);
      }
      if (pBlockTable)
        pBlockTable->close();
    }
  }

  AcDbBlockTableRecordPtr blkrec;
  if (rxOpenObject(blkrec, ownerId) != Acad::eOk)
    return false;
  if (!blkrec)
    return false;
  AcDbBlockTableRecordIterator* iter = NULL;
  blkrec->newIterator(iter, true, false);
  if (bUseBegId)
  {
    AcDbObjectId entId;
    for(;!iter->done(); iter->step(true, false))
      if (iter->getEntityId(entId) == Acad::eOk)
        if (entId == begId)
          break;
  }
  if (bUseBegId && ExludeFirst)
    iter->step();
  for(;!iter->done(); iter->step())
  {
    AcDbEntity* pEnt = NULL;
    if (iter->getEntity(pEnt, AcDb::kForRead) == Acad::eOk)
      if (pEnt->ownerId() == ownerId)
        objList.append(pEnt->objectId());
    if (pEnt) pEnt->close();
  }

#ifndef _NCAD_BUILD_NODELETE
  delete iter;
#endif

  return (objList.length()>0);
}

//bool rx_collectEntites( AcDbObjectIdArray &objList, const ads_name beg_aname, bool ExludeFirst /*= false*/, bool OnlyMain /*= true*/ )
/*
{
  objList.setLogicalLength(0);

  ads_name ent0, ent1;

  AcDbObject *pObj;
  AcDbObjectId objId, ownerId = AcDbObjectId::kNull;
  bool flagFirst = true;
  if ( OnlyMain )
  {
    AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();

    LPCTSTR pSpace = ArxDbgUtils::isPaperSpace(pDb) ? ACDB_PAPER_SPACE : ACDB_MODEL_SPACE;

    AcDbBlockTable *pBlockTable;
    pDb->getSymbolTable(pBlockTable, AcDb::kForRead);
    pBlockTable->getAt(pSpace, ownerId);
    pBlockTable->close();
    flagFirst = false;
  }
  if ( acdbNameNil(beg_aname) )
  {
    if (acdbEntNext(NULL, ent0) != RTNORM) // ѕустой рисунок
      return FALSE;
  }
  else
  if (ExludeFirst)
  {
    if (acdbEntNext(beg_aname, ent0) != RTNORM) // ѕустой рисунок
      return FALSE;
  }
  else
    acdbNameSet(beg_aname, ent0);

  do
  {
    if( acdbGetObjectId(objId, ent0) == Acad::eOk &&
        acdbOpenObject(pObj, objId, AcDb::kForRead) == Acad::eOk)
    {
      // ѕроверка, чтобы убедитьс€ это имеет того же самого владельца как первый объект.
      if ( flagFirst ) 
      {
          ownerId = pObj->ownerId();
          objList.append(objId);
          flagFirst = false;
      }
      else if (pObj->ownerId() == ownerId)
        objList.append(objId);
      pObj->close();
    }
    acdbNameSet(ent0, ent1); // Bump the name.
  } while (acdbEntNext(ent1, ent0) == RTNORM);

  return ( !objList.isEmpty() );
}
*/


Acad::ErrorStatus
rx_getBlockAttDefId(const AcDbAttribute* pAttr, AcDbObjectId &attId)
{
  AcDbBlockReferencePtr pBlkRef(pAttr->ownerId(), AcDb::kForRead);
  Acad::ErrorStatus es = pBlkRef.openStatus();
  if ( es != Acad::eOk)
    return es;

  AcDbObjectId objId = pBlkRef->blockTableRecord();
  pBlkRef.close();

  AcDbBlockTableRecordPtr pBlkRec(objId, AcDb::kForRead);
  es = pBlkRef.openStatus();
  if ( es != Acad::eOk)
    return es;

  es = Acad::eNotInBlock;
  AcDbBlockTableRecordIterator *pIterator;
  pBlkRec->newIterator(pIterator);
  AcDbEntity *pEnt;
  AcDbAttributeDefinition *pAttDef;
  const TCHAR *tag = pAttr->tag();

  for (pIterator->start(); !pIterator->done(); pIterator->step())
  {
    es = pIterator->getEntity(pEnt, AcDb::kForRead);
    if (es == Acad::eOk)
    {
      // Make sure the entity is an attribute definition and not a constant.
      pAttDef = AcDbAttributeDefinition::cast(pEnt);
      if (pAttDef != NULL) 
      {
        if ( !_tcsicmp(pAttDef->tag(), tag) ) {
          attId = pAttDef->objectId();
          pAttDef->close();
          es = Acad::eOk;
          break;
        }
      }
      pEnt->close(); // use pEnt... pAttdef might be NULL
    }
  }
#ifndef _NCAD_BUILD_NODELETE
  delete pIterator;
#endif  
  return es;
}


Acad::ErrorStatus
rx_getBlockAttrId(const AcDbObject* ent, LPCTSTR  tag, AcDbObjectId &attId, bool openErased /*= false*/ )
{
  Acad::ErrorStatus es;
  AcDbBlockReference *pBlockRef = AcDbBlockReference::cast(ent);
  if( pBlockRef == NULL) return Acad::eNotThatKindOfClass;

  AcDbObjectIterator* attribIter = pBlockRef->attributeIterator();
  es = Acad::eNotInBlock;
  if (attribIter != NULL) {
    AcDbAttribute *pAtt;
    while (attribIter->done() == false) {
      attId = attribIter->objectId();
      if ( acdbOpenObject(pAtt, attId, AcDb::kForRead, openErased) == Acad::eOk) {
        if ( !_tcsicmp(pAtt->tag(), tag) ) {
          pAtt->close();
          es = Acad::eOk;
          break;
        }
        pAtt->close();
      }
      attribIter->step();
    }
#ifndef _NCAD_BUILD_NODELETE
    delete attribIter;
#endif
  }
  return es;
}

Acad::ErrorStatus
rx_getBlockAttrId(const AcDbObjectId blockId, LPCTSTR  tag, AcDbObjectId &attId, bool openErased /*= false*/ )
{
  Acad::ErrorStatus es;

  AcDbBlockReference *pBlockRef;
  es = acdbOpenObject(pBlockRef, blockId, AcDb::kForRead, openErased);
  if (es != Acad::eOk)
    return es;
  es = rx_getBlockAttrId(pBlockRef, tag, attId, openErased);
  pBlockRef->close();
  return es;
}
//----------------------------------------------------------------------------------------------
Acad::ErrorStatus
rx_setAttrValue(const AcDbObjectId blockId, LPCTSTR  name, LPCTSTR  strVal, int len /*=0*/)
{
  Acad::ErrorStatus es;
  AcDbAttribute *pAtt;

  AcDbObjectIdArray objIdAr;
  es = rx_collectAttributes( blockId, objIdAr, name, len);
  for (int i = 0; i < objIdAr.length(); i++)
    if ( (es = acdbOpenObject(pAtt, objIdAr[i], AcDb::kForWrite)) == Acad::eOk )  {
      es = pAtt->setTextString(strVal);
      pAtt->close();
    }
  return es;
}

Acad::ErrorStatus
rx_getAttrValue(const AcDbObjectId blockId, const LPCTSTR  name, LPTSTR &strVal )
{
  Acad::ErrorStatus es;
  AcDbAttribute *pAtt;
  AcDbObjectId objId;
  if ( (es = rx_getBlockAttrId( blockId, name, objId )) == Acad::eOk )
    if ( (es = acdbOpenObject(pAtt, objId, AcDb::kForRead)) == Acad::eOk )  {
      strVal = pAtt->textString();
      pAtt->close();
    }
  return es;
}

//----------------------------------------------------------------------------------------------
Acad::ErrorStatus
rx_collectAttributes(const AcDbObjectId &blkRefId, AcDbObjectIdArray& subEnts, LPCTSTR  tag /*=NULL*/, int len/*=0*/)
{
  RXASSERT(blkRefId != NULL);

  Acad::ErrorStatus es;

  AcDbBlockReference *blkRef;
  if ( (es = acdbOpenObject(blkRef, blkRefId, AcDb::kForRead)) != Acad::eOk)
    return es;

  rx_collectAttributes( blkRef, subEnts, tag, len);
  blkRef->close();
  return es;
}

Acad::ErrorStatus 
rx_collectAttributes(const AcDbBlockReference* blkRef, AcDbObjectIdArray& subEnts, LPCTSTR  tag /*=NULL*/, int len/*=0*/)
{
  RXASSERT(blkRef != NULL);
  Acad::ErrorStatus es = Acad::eOk;

  AcDbObjectIterator* attribIter = blkRef->attributeIterator();
  if (attribIter != NULL) {
    while (attribIter->done() == false) {
      if ( tag ) {
        AcDbObjectId objId = attribIter->objectId();
        AcDbAttribute *pAtt;
        if ((es = acdbOpenObject(pAtt, objId, AcDb::kForRead)) == Acad::eOk)
        {
		  if ((len != 0 && !_tcsnicmp( pAtt->tag(), tag, len)) ||
			  (len == 0 && !_tcsicmp( pAtt->tag(), tag) ) )
				subEnts.append(objId);
          pAtt->close();
        }
        else
          break;
      }
      else subEnts.append(attribIter->objectId());
      attribIter->step();
    }
#ifndef _NCAD_BUILD_NODELETE
    delete attribIter;
#endif
  }
  return es;
}

int rx_textRect (ads_name tname, AcGePoint3dArray& verts ) 
{ 
    struct resbuf *textent, *tent; 
    AcGePoint3d p0, p1, p2; 
    double ang; 

    textent = acdbEntGet(tname); 
    if (textent == NULL) { 
        //acdbFail(_T("Couldn't retrieve Text entity\n")); 
        acdbFail(_T(" примиитив не €вл€етс€ текстом.\n")); 
        return RTERROR; 
    } 
    tent = rx_assocRb(textent, 10); 
    p0.set( tent->resval.rpoint[X], tent->resval.rpoint[Y], 0.0); //ECS coordinates 

    tent = rx_assocRb(textent, 50); 
    ang = tent->resval.rreal; 

    if (acedTextBox(textent, asDblArray(p1), asDblArray(p2) ) != RTNORM) { 
        //acdbFail(_T("Couldn't retrieve text box coordinates\n"));
        acdbFail(_T("Ќе могу отыскать координаты текстового пол€\n"));
        acutRelRb(textent); 
        return RTERROR; 
    } 
    acutRelRb(textent); 

    p1 += p0.asVector();
    p2 += p0.asVector();

    verts.setLogicalLength(4);
    verts[0] = AcGePoint3d(p1[X], p1[Y], 0.0);
    verts[1] = AcGePoint3d(p2[X], p1[Y], 0.0);
    verts[2] = AcGePoint3d(p2[X], p2[Y], 0.0);
    verts[3] = AcGePoint3d(p1[X], p2[Y], 0.0);
    AcGeVector3d normal(0,0,1);
    for (int i = 0; i < 4; i++ )
      verts[i].rotateBy ( ang , normal, p0 );

    return RTNORM; 
} 

bool rx_textRect(const  TCHAR* text, 
                 const  AcGePoint3d& pt,
                 const  double &height,
                 const  AcGeVector3d& normal,
                 const  double &ang, 
                 const  AcDbObjectId &styleId,
                 AcGePoint3dArray& verts )
{
  AcDbTextStyleTableRecord *style;
  acdbOpenAcDbObject((AcDbObject *&)style, styleId, AcDb::kForRead);
  const TCHAR *nameStyle;
  style->getName(nameStyle);

  resbuf *text_lst = acutBuildList( 
        RTDXF0, _T("TEXT"), 
      //5020, _T("TEXT"), 
        10, asDblArray(pt),
        40, (height == 0.0 ? style->textSize() : height),
         1, text,
        50, ang,
         7, nameStyle,
        41, style->xScale(),
        51, style->obliquingAngle(),
         0); 
    style->close();

    if ( !text_lst ) return false;

    AcGePoint3d p1, p2;
    acedTextBox( text_lst, asDblArray(p1), asDblArray(p2) );
    acutRelRb(text_lst);

    p1 += pt.asVector();
    p2 += pt.asVector();
    const int count = 4;
    verts.setLogicalLength(count);
    verts[0] = AcGePoint3d(p1[0], p1[1], 0.0);
    verts[1] = AcGePoint3d(p2[0], p1[1], 0.0);
    verts[2] = AcGePoint3d(p2[0], p2[1], 0.0);
    verts[3] = AcGePoint3d(p1[0], p2[1], 0.0);
    for (int i = 0; i < count; i++ )
      verts[i].rotateBy ( ang , normal, pt );
    return true;
}



bool rx_textBox(AcGePoint3d *ptMin, AcGePoint3d *ptMax,
                const  TCHAR* text, 
                const  AcDbObjectId& styleId,
                const  double height,
                const  double widthFactor, 
                const  double obliquingAngle,
                const  AcGePoint3d& pt,
                const  double rotation, 
                const  AcDb::TextHorzMode hMode,
                const  AcDb::TextVertMode vMode,
                const  AcGePoint3d& ptAlign)
{
  if (styleId.isNull())
    return false;

  AcDbTextPtr pText; pText.create();
  pText->setDatabaseDefaults(WORKDWG);
  pText->setTextStyle(styleId);
  pText->setTextString(text);
  pText->setHeight(height);
  pText->setWidthFactor(widthFactor);
  pText->setOblique(obliquingAngle);
  pText->setPosition(pt);
  pText->setAlignmentPoint(ptAlign);
  pText->setHorizontalMode(hMode);
  pText->setVerticalMode(vMode);
  pText->setRotation(rotation);

  AcDbExtents extents; 
  if (pText->getGeomExtents(extents) == Acad::eOk)
  {
    *ptMin = extents.minPoint();
    *ptMax = extents.maxPoint();
    return true;
  }
  else
    return false;
}
/*
bool rx_textBox(AcGePoint3d *pt1, AcGePoint3d *pt2,
                const  TCHAR* text, 
                const  AcGePoint3d& pt,
                const  TCHAR* nameStyle,
                const  double height,
                const  double ang, 
                const  double xScale, 
                const  double obliquingAngle,
                const  AcDb::TextHorzMode hMode,
                const  AcDb::TextVertMode vMode,
                const  AcGePoint3d& ptAlign)
{
  resbuf *text_lst = acutBuildList( 
        RTDXF0, _T("TEXT"), 
        10, asDblArray(pt),
        40, height,
         1, text,
        50, ang,
         7, nameStyle,
        41, xScale,
        51, obliquingAngle,
        72, hMode,
        11, asDblArray(ptAlign),
        73, vMode,
         0); 
  if ( !text_lst ) return false;
  acedTextBox( text_lst, asDblArray(*pt1), asDblArray(*pt2) );
  acutRelRb(text_lst);
  return true;
}
*/

bool rx_textBox ( LPCTSTR  text,
                  const double &textHeight,
                  const  AcDbObjectId &styleId,
                  AcGePoint3d& ptMin,
                  AcGePoint3d& ptMax)
{
  bool ret = false;
  AcDbTextStyleTableRecord *style = NULL;
  if (acdbOpenAcDbObject((AcDbObject *&)style, styleId, AcDb::kForRead) == Acad::eOk) 
  {
    const TCHAR *styleName;
    style->getName(styleName);
    double height = (textHeight <= 0.01 ? style->textSize() : textHeight);
    double xScale = style->xScale();
    double obliquingAngle = style->obliquingAngle();
    style->close();

    ads_point pt; pt[X] = pt[Y] = pt[Z] = 0.0;
    resbuf *text_lst = acutBuildList( 
    RTDXF0, _T("TEXT"), 
        40, height,
         1, text,
        //50, 0, //-- умолчание 
         7, styleName,
        41, xScale,
        51, obliquingAngle,
         0);
    if ( text_lst ) 
    {
      ret = acedTextBox( text_lst, asDblArray(ptMin), asDblArray(ptMax) ) == RTNORM;
      acutRelRb(text_lst);
    }
  }
  
  return ret;
}

bool rx_textBox ( LPCTSTR  text,
                  const double &height,
                  const AcGiTextStyle &textStyle, 
                  AcGePoint3d& ptMin,
                  AcGePoint3d& ptMax)
{
  resbuf *text_lst = acutBuildList( 
        RTDXF0, _T("TEXT"), 
        40, (height == 0.0 ? textStyle.textSize() : height),
         1, text,
        //50, 0, //-- умолчание 
         7, textStyle.styleName(),
        41, textStyle.xScale(),
        51, textStyle.obliquingAngle(),
         0); 
  if ( !text_lst ) return false;
  acedTextBox( text_lst, asDblArray(ptMin), asDblArray(ptMax) );
  acutRelRb(text_lst);
  return true;
}


void rx_splitTextOld(LPCTSTR  str, double width, CStringArray &resArray, AcDbObjectId styleId, double height)
{
  if ( !str ) return;
  resArray.RemoveAll();

  double angle = 0.0, xScale = 1.0, obliquingAngle = 0.0;
  TCHAR *nameStyle = NULL;
  AcDbTextStyleTableRecord *style = NULL;
  if ( acdbOpenAcDbObject((AcDbObject *&)style, styleId, AcDb::kForRead) == Acad::eOk)
  {
    style->getName(nameStyle);
    xScale = style->xScale();
    obliquingAngle = style->obliquingAngle();
  }
  if (style) style->close();

  if (nameStyle == NULL)
    nameStyle = _tcsdup(_T("STANDART"));

  CString s1, s2, s = str;
  AcGePoint3d pt, pt1, pt2;
  int i, f;
  bool endOn = false;

  rx_textBox(&pt1, &pt2, s, styleId, height, xScale, obliquingAngle);
  while ( pt2.x - pt1.x > width && !s.IsEmpty())
  {
    i = int(s.GetLength() * ((double)width / (pt2.x - pt1.x)) + 1);
    s1 = s.Left(i);
    f = s1.ReverseFind('.');
    if ( f == -1)
      f = s1.ReverseFind(';');
    if ( f == -1)
      f = s1.ReverseFind(',');
    if ( f == -1)
      f = s1.ReverseFind(' ');
    if ( f == -1)
      f = s1.ReverseFind('/');
    if ( f == -1)
      f = s1.ReverseFind('\\');
    if ( f > -1)
      i = f;
    else
    {
      f = s.Find('.', i);
      if ( f == -1)
        f = s1.Find(';', i);
      if ( f == -1)
        f = s1.Find(',', i);
      if ( f == -1)
        f = s1.Find('.', i);
      if ( f == -1)
        f = s1.Find('/', i);
      if ( f == -1)
        f = s1.Find('\\', i);
      if ( f > -1)
        i = f;
      else
      {
        resArray.Add(s);
        endOn = true;
        break;
      }
    }
    if (s[i] == ' ')
    {
      resArray.Add(s.Left(i));
      s = s.Mid(i);
      s.TrimLeft();
    }
    else
    {
      resArray.Add(s.Left(i+1));
      s = s.Mid(i+1);
      s.TrimLeft();
    }
    rx_textBox(&pt1, &pt2, s, styleId, height, xScale, obliquingAngle);
  }
  if (!endOn && !s.IsEmpty()) 
    resArray.Add(s);

  if (nameStyle) free(nameStyle);
}

/*
#define RUS_A CString("[абвгдеЄжзийклмнопрстуфхцчшщъыьэю€]")
#define RUS_V CString("[аеЄиоуыэю€]")
#define RUS_N CString("[бвгджзклмнпрстфхцчшщ]")
#define RUS_X CString("[йъь]")

CString RegExp(CString s, bool CaseInsensitive)
{
  return CString("");
}

CString Hhyphenate(CString& text)
{
  bool CaseInsensitive = true;
  CString re1 = CRegExp(CString("(") + RUS_X + ")(" + RUS_A +  RUS_A + ")",  CaseInsensitive);
  CString re2 = RegExp(CString("(") + RUS_V + ")(" + RUS_V +   RUS_A + ")",  CaseInsensitive);
  CString re3 = RegExp(CString("(") + RUS_V + RUS_N + ")(" + RUS_N + RUS_V + ")",  CaseInsensitive);
  CString re4 = RegExp(CString("(") + RUS_N + RUS_V + ")(" + RUS_N + RUS_V + ")", CaseInsensitive);
  CString re5 = RegExp(CString("(") + RUS_V + RUS_N + ")(" + RUS_N + RUS_N + RUS_V + ")",  CaseInsensitive);
  CString re6 = RegExp(CString("(") + RUS_V + RUS_N + RUS_N + ")(" + RUS_N + RUS_N + RUS_V + ")",  CaseInsensitive);
  CString hypher = "\\1-\\2";
  text.Replace(re1, hypher);
  text.Replace(re2, hypher);
  text.Replace(re3, hypher);
  text.Replace(re4, hypher);
  text.Replace(re5, hypher);
  text.Replace(re6, hypher);
  return text;
}
*/

typedef enum { st_Empty,
               st_NoDefined,
               st_Glas,
               st_Sogl,
               st_Spec
} TSymbol;

typedef AcArray<TSymbol> TSymbAR;

const TCHAR HypSymb = _T('-');

const TCHAR Spaces[] = _T(" ,;:.?!/\r\n");
const TCHAR GlasChar[] = _T("…й”у≈еёюјаќоЁэя€»иeEuUiIoOaAjJ");
const TCHAR SoglChar[] = _T("√г÷ц кЌнЎшщў«з’х‘ф¬вѕп–рЋлƒд∆ж„ч—сћмтTбЅqQwWrRtTyYpPsSdDfFgGhHkKlLzZxXcCvVbBnNmM");
const TCHAR SpecSign[] = _T("џы№ьЏъ");

bool isSogl(TCHAR c) { return _tcschr(SoglChar, c) != NULL;  }
bool isGlas(TCHAR c) { return _tcschr(GlasChar, c) != NULL;  }
bool isSpecSign(TCHAR c) { return _tcschr(SpecSign, c) != NULL;  }
bool isSpace(TCHAR c) { return _tcschr(Spaces, c) != NULL;  }

TSymbol GetSymbType(TCHAR c)
{
  if (isSogl(c)) return st_Sogl;
  if (isGlas(c)) return st_Glas;
  if (isSpecSign(c)) return st_Spec;
  return st_NoDefined;
}

bool isSlogMore(TSymbol *c, int start, int len)
{
  for(int i=start; i < len; i++)
  {
    if (c[i]==st_NoDefined) 
      return false;
    if (c[i]==st_Glas && (c[i+1]!=st_NoDefined || i!=start))
      return true;
  }
  return false;
}

// расставл€лка переносов
TCHAR* SetHyph(TCHAR* pc, int MaxSize)
{
  if (MaxSize < 1 || _tcslen (pc) == 0)
    return NULL;
  int i, 
    cur,  //Tекуща€ позици€ в разультирующем массиве
    cw,   //Ќомер буквы в слове
    lock, //счетчик блокировок
    len = MaxSize;
  TCHAR *HypBuff = new TCHAR[MaxSize+1];
  TSymbol *h = new TSymbol[len+1];

  //заполнение массива типов символов
  for (i=0; i < len; i++)
    h[i]=GetSymbType(pc[i]);

  //собственно расстановка переносов}
  cur = cw = lock = 0;
  for (i=0; i < len; i++)
  {
    HypBuff[cur]=pc[i]; cur++;
    if (i>=len-2) continue;
    if (h[i]==st_NoDefined)
    {
      cw=0;
      continue;
    }
    else
      cw++; 

    if (lock!=0) {
      lock--;
      continue;
    }
    if (cw<=1)
      continue;
    if (!isSlogMore(h,i+1,len)) 
      continue;

    if ( h[i]==st_Sogl && h[i-1]==st_Glas && h[i+1]==st_Sogl && h[i+2]!=st_Spec)
    {
      HypBuff[cur]=HypSymb; cur++; lock=1;
    }
    if ( h[i]==st_Glas && h[i-1]==st_Sogl && h[i+1]==st_Sogl && h[i+2]==st_Glas)
    {
      HypBuff[cur]=HypSymb; cur++; lock=1;
    }
    if ( h[i]==st_Glas && h[i-1]==st_Sogl && h[i+1]==st_Glas && h[i+2]==st_Sogl)
    {
      HypBuff[cur]=HypSymb; cur++; lock=1;
    }
  }
  delete h;
  HypBuff[cur] = 0;
  return HypBuff;
}


bool Red_GlasMore(TCHAR* p, int pos)
{
  while (p[pos])
  {
    if (isSpace(p[pos])) return false;
    if (isGlas(p[pos]) ) return true;
    pos++;
  }
  return false;
}

bool Red_SlogMore(TCHAR* p, int pos)
{
  bool BeSogl = false, BeGlas = false;
  while (p[pos])
  {
    if (isSpace(p[pos])) break;
    if (!BeGlas) BeGlas = isGlas(p[pos]);
    if (!BeSogl) BeSogl = isSogl(p[pos]);
    pos++;

  }
  return BeGlas && BeSogl;
}

bool MayBeHyph(TCHAR* p, int i)
{
  int len =_tcslen(p);
  return (
    len>3 &&
    i>2 &&
    i<len-2 &&
    (!isSpace(p[i]) && !isSpace(p[i+1]) && !isSpace(p[i-1])) &&
    ( (isSogl(p[i]) && isGlas(p[i-1]) && isSogl(p[i+1]) && Red_SlogMore(p,i+1)) ||
      (isGlas(p[i]) && isSogl(p[i-1]) && isSogl(p[i+1]) && isGlas(p[i+2])) ||
      (isGlas(p[i]) && isSogl(p[i-1]) && isGlas(p[i+1]) && Red_SlogMore(p,i+1)) ||
      (isSpecSign(p[i])) ) );
}


CString SetHyphString(CString s)
{
  CString Result;
  TCHAR* Res = new TCHAR[s.GetLength()*2];
  Res = SetHyph(Res, s.GetLength()*2);
  Result= Res;
  free(Res);
  return Result;
}


const TCHAR splitterSymbols[] = _T(".,; ");
const TCHAR splitterSymbolsRetry[] = _T("-+/\\");
const TCHAR splitterSymbolsExludeNum[] =  _T(",.");

bool isSplitSymbol(TCHAR c)
{
  return _tcschr(splitterSymbols, c) != NULL;
}

bool isSplitSymbolRetry(TCHAR c)
{
  return _tcschr(splitterSymbolsRetry, c) != NULL;
}

bool isSplitSymbolExludeNum(TCHAR c)
{
  return _tcschr(splitterSymbolsExludeNum, c) != NULL;
}


int splitTextFindSplitter(LPCTSTR str)
{
  CString s = str;
  int lenTmp = 0, iResult = -1, i = 0, len = s.GetLength();
  while (i < len)
  {
    i = s.FindOneOf(splitterSymbols);
    if (i > -1)
      iResult = i + lenTmp;
    else
      iResult = -1;
    if (i > -1 && i+1 < len &&
        isSplitSymbolExludeNum(s[i]) && _istdigit(s[i+1]))
    {
      lenTmp = i;
      s = s.Mid(i+1);
      i = 0; len = s.GetLength();
    }
    else
      break;
  }
  return iResult;
}

void rx_splitText(LPCTSTR str, double width, CStringArray &resArray, 
                  double height, LPCTSTR nameStyle, double scale, double obliquingAngle)
{
  if ( !str ) return;
  resArray.RemoveAll();

  double angle = 0.0;

  CString st, s = str;
  AcGePoint3d pt, pt1, pt2;
  int i, f;

  AcDbObjectId styleId;
  rx_getTextStyleId(nameStyle, styleId);
  if (styleId.isNull())
    WORKDWG->textstyle();

  s.TrimLeft();
  s.TrimRight();
  while (!s.IsEmpty() &&
         rx_textBox(&pt1, &pt2, s, styleId, height, scale, obliquingAngle) &&
         1.2*(pt2.x - pt1.x) > width )
  {
    i = int(s.GetLength() * ((double)width / (pt2.x - pt1.x)) + 1);
    f = -1;
    if (i >= 0)
    {
      if (i > 3) i -= 1;
      st = s.Mid(i);
      f = splitTextFindSplitter(st);
    }

    if (f > -1)
    {
      i += (f+1);
      while(s[i]==' ')i++;
      //if(isSplitSymbolRetry(st[f]))
      //  i -= 1;
    }
    else
      break; //!!! остаток возвращаем как есть!!!

    st = s.Left(i);
    st.TrimLeft();
    st.TrimRight();
    if (!st.IsEmpty())
      resArray.Add(st);

    s = s.Mid(i);
    //s.TrimLeft();
    //s.TrimRight();
  }

  if (!s.IsEmpty()) 
    resArray.Add(s);
}

int rx_getPickfirst(AcDbObjectIdArray& objIdAr)
{
  objIdAr.setLogicalLength(0);

  // Get the selection sets
  AcDbObjectId entId;
  ads_name ename;
  struct resbuf* prbGrip = NULL;
  struct resbuf* prbPick = NULL;
  int rt = acedSSGetFirst(&prbGrip, &prbPick);
  if (rt != RTNORM)
    return rt;

  Adesk::Int32 gripLen = 0, pickLen = 0; //long
  ads_name &pfSet = prbPick->resval.rlname;
  acedSSLength(pfSet, &pickLen);
  //acedSSLength(prbGrip->resval.rlname, &gripLen);
  for(long i = 0; i < pickLen; i++)
    if(acedSSName(pfSet, i, ename) == RTNORM)
      if(acdbGetObjectId(entId, ename) == Acad::eOk)
        objIdAr.append(entId);

  // Don't forget to free the selections sets.
  // Also don't forget to release the result buffers
  if (prbPick) {
    acedSSFree(prbPick->resval.rlname); 
    acutRelRb(prbPick);
  }
  if (prbGrip)
  {
    acedSSFree(prbGrip->resval.rlname);
    acutRelRb(prbGrip);
  }
  return rt;
}

int rx_setPickfirst(const AcDbObjectIdArray& objIdAr)
{
  rx_clearPickFirst();

  ads_name lname, ourset;
  ourset[0] = ourset[1] = 0;
  int rt = acedSSAdd(NULL, NULL, ourset);
  if ( rt != RTNORM) 
    return rt; 

  for (int i=0; i < objIdAr.length(); i++)
    if (acdbGetAdsName(lname, objIdAr[i]) == Acad::eOk)
      acedSSAdd(lname, ourset, ourset); 

  Adesk::Int32 count = 0; //long
  rt = acedSSLength(ourset, &count);
  if (rt == RTNORM && count == objIdAr.length())
    rt = acedSSSetFirst(ourset, NULL);
  else
    acedSSFree(ourset);

  return rt;
}

void rx_clearPickFirst()
{
  acedSSSetFirst(NULL, NULL);
}

//=========================================================================
//void _RedrawTr_Poly2d( AcDb2dPolyline* pPline2d )
//{
//  AcDbObjectIterator* pPolyIter = pPline2d->vertexIterator();
//  AcDb2dVertex *pVert;
//  for(pPolyIter->start(); !pPolyIter->done();	pPolyIter->step())
//  {
//    pVert = AcDb2dVertex::cast(pPolyIter->entity());
//    rx_redrawTr(pVert);
//  }
//}
//
//void _RedrawTr_AcDbWEntity(AcDbWEntity* pWEnt)
//{
//  AcDbObject* pObj;
//  for(int i=0; i < pWEnt->linkIds().length(); i++)
//    if (actrTransactionManager->getObject(pObj, pWEnt->linkIds().at(i), AcDb::kForWrite) == Acad::eOk)
//      rx_redrawTr(AcDbEntity::cast(pObj));
//}

void rx_redrawTr(AcDbEntity* pEntity)
{
  //if (pEntity == NULL) return;
  ASSERT(FALSE);
  //rx_refreshDisplay();
  //try
  //{
  //  if( pEntity->isWriteEnabled() )
  //  {
  //    pEntity->downgradeOpen();
  //    pEntity->upgradeOpen();
  //  }
  //  else
  //  {
  //    pEntity->upgradeOpen();
  //    pEntity->downgradeOpen();
  //  }

  //  //WORKDWG->modifiedGraphics(pEntity);

  //  if (pEntity->isKindOf(AcDb2dPolyline::desc()))
  //    _RedrawTr_Poly2d(AcDb2dPolyline::cast(pEntity));
  //  else
  //  if (pEntity->isKindOf(AcDbWEntity::desc()))
  //    _RedrawTr_AcDbWEntity(AcDbWEntity::cast(pEntity));
  //}
  //catch (...)
  //{
  //  ASSERT(FALSE);
  //  return;
  //}
}

void rx_redrawTr(const AcDbObjectId& objId)
{
  rx_refreshDisplay();
  //AcDbObject* pObj = NULL;
  //if (actrTransactionManager->getObject(pObj, objId, AcDb::kForWrite) == Acad::eOk)
  //  rx_redrawTr(AcDbEntity::cast(pObj));
}

void rx_redrawTr(const AcDbObjectIdArray& objIdAr)
{
  rx_refreshDisplay();
  //for (UINT i=0; i<objIdAr.length();i++)
  //  rx_redrawTr(objIdAr[i]);
}

void rx_refreshDisplay()
{
#if defined(_NCAD_BUILD_) && defined(_NCAD_USE_EXTNRX)
  rxncm_hostRegen(false);
#else
  actrTransactionManager->queueForGraphicsFlush();
  actrTransactionManager->flushGraphics();
  acedUpdateDisplay();
#endif
}

void rx_redraw(const AcDbObjectIdArray& objIdAr)
{
/*
  actrTransactionManager->startTransaction();

  AcDbObject* pObject;
  AcDbEntity* pEnt;
  Acad::ErrorStatus es;
  for (int i=0; i<objIdAr.length();i++)
  {
    es = actrTransactionManager->getObject(pObject, objIdAr[i], AcDb::kForWrite);
    if( es == Acad::eOk)
    {
      pEnt = AcDbEntity::cast(pObject);
      if (pEnt)
      {
        pEnt->setVisibility(AcDb::kVisible);
        pEnt->recordGraphicsModified(Adesk::kTrue);
        pEnt->draw();
      }
    }
  }
  actrTransactionManager->endTransaction();
  rx_refreshDisplay();
*/

#if defined(_NCAD_BUILD_) && defined(_NCAD_USE_EXTNRX)
  rxncm_hostRegen(true);
#else
  AcDbEntityPtr pEnt;
  for (int i=0; i<objIdAr.length();i++)
  {
    if(rxOpenObject(pEnt, objIdAr[i], AcDb::kForWrite) == Acad::eOk)
    {
      if (pEnt->visibility() == AcDb::kVisible)
        pEnt->setVisibility(AcDb::kVisible);
      //pEnt->draw();
      //pEnt->close();
    }
  }
  rxCloseObject(pEnt);
  rx_refreshDisplay();
  //acedUpdateDisplay();
#endif
}

void rx_redraw(resbuf* filter)
{
  int ret = RTNORM;
  ads_name ss;
  ret = acedSSGet(_T("_X"), NULL, NULL, filter, ss);
  if( ret != RTNORM )
    return;
  Adesk::Int32 len = 0; //long
  if (acedSSLength(ss, &len)  != RTNORM || len == 0)
    return;
  AcDbObjectId objId;
  ads_name ent;
  AcDbObjectIdArray objIds;
  objIds.setLogicalLength(len);
  for (long i=0L; i<len; i++) {
      ret = acedSSName(ss, i, ent);
      ASSERT(ret == RTNORM);
      if (acdbGetObjectId(objId, ent) == Acad::eOk)
          objIds.setAt(i, objId);
  }
  rx_redraw(objIds);
}

int rx_commandCmd(LPCTSTR cmd, bool bCmdEchoOff/* = true*/)
{
  int svCmdEcho = 0;
  if (bCmdEchoOff)
    bCmdEchoOff = getSysVar(AcadVar::cmdecho, svCmdEcho) == Acad::eOk;
  if (bCmdEchoOff)
    bCmdEchoOff = svCmdEcho != 0;
  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, 0);
  int rt = acedCommand(RTSTR, cmd, 0); //RTNONE
  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, svCmdEcho);
  return rt;
}

 // ≈сть возможность пр€мого вызова команд AutoCAD (без вс€ких acedCommand, sendStringToExecute и т.д. - через пр€мой вызов функции). ¬от небольшой примерчик:
int rx_commandFromStack(LPCTSTR cmd)
{
  AcEdCommandStack* pCmdStack = acedRegCmds;
  AcEdCommand* pCmd = pCmdStack->lookupCmd(cmd,true);
  if (pCmd && pCmd->functionAddr()) {
    AcRxFunctionPtr funcCommand = pCmd->functionAddr();
    funcCommand();
    return RTNORM;
  } else {
    PRINT_CARET
    acutPrintf(_T("\nrx_command:Not found command \"%s\"\n"), cmd);
    AcEdCommandIterator* pIter = pCmdStack->iterator();
    for(; !pIter->done(); pIter->next() )
      acutPrintf(_T("%s "), pIter->command()->globalName());
    return RTERROR;
  }
}

#ifdef ARXDBG_UTILS_H

void rx_clearUndoCmd()
{
  int undoCtl = 0;
  getSysVar(_T("UNDOCTL"), undoCtl);
  bool isOn = (undoCtl & 1) == 1;
  if (!isOn)
    return;

  bool bCmdEchoOff = true;
  int svCmdEcho = 0;
  if (bCmdEchoOff)
    bCmdEchoOff = getSysVar(AcadVar::cmdecho, svCmdEcho) == Acad::eOk;
  if (bCmdEchoOff)
    bCmdEchoOff = svCmdEcho != 0;
  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, 0);

  bool isOneCmd = (undoCtl & 2) == 2;
  bool isAuto = (undoCtl & 4) == 4;

  ArxDbgRbList appendList;
  resbuf* newNode;

  newNode = acutNewRb(5005);
  acutNewString(_T("_UNDO"),newNode->resval.rstring);
  appendList.addHead(newNode);

  newNode = acutNewRb(5005);
  acutNewString(_T("_Control"),newNode->resval.rstring);
  appendList.addTail(newNode);

  newNode = acutNewRb(5005);
  acutNewString(_T("_None"),newNode->resval.rstring);
  appendList.addTail(newNode);

  newNode = acutNewRb(5005);
  acutNewString(_T("_UNDO"),newNode->resval.rstring);
  appendList.addTail(newNode);

  if (isOneCmd)
  {
    newNode = acutNewRb(5005);
    acutNewString(_T("_One"),newNode->resval.rstring);
    appendList.addTail(newNode);
  }
  else
  {
    newNode = acutNewRb(5005);
    acutNewString(_T("_All"),newNode->resval.rstring);
    appendList.addTail(newNode);

    if (!isAuto)
    {
      newNode = acutNewRb(5005);
      acutNewString(_T("_UNDO"),newNode->resval.rstring);
      appendList.addTail(newNode);

      newNode = acutNewRb(5005);
      acutNewString(_T("_Auto"),newNode->resval.rstring);
      appendList.addTail(newNode);

      newNode = acutNewRb(5005);
      acutNewString(_T("_Off"),newNode->resval.rstring);
      appendList.addTail(newNode);
    }
  }

  acedCmd(appendList.data());

  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, svCmdEcho);
}

void rx_clearUndoCmdSend()
{
  int undoCtl = 0;
  getSysVar(_T("UNDOCTL"), undoCtl);
  bool isOn = (undoCtl & 1) == 1;
  if (!isOn)
    return;

  bool isOneCmd = (undoCtl & 2) == 2;
  bool isAuto = (undoCtl & 4) == 4;

  CString cmdString = "_UNDO _Control _None _UNDO ";

  if (isOneCmd)
    cmdString += "_One ";
  else
  {
    cmdString += "_All ";

    if (!isAuto)
      cmdString += "_UNDO _Auto _Off ";
  }

  cmdString.TrimRight();

  //rx_commandCmd(cmdString);
  ArxDbgUtils::stuffAcadCmdLine(cmdString, true, false, true);
}

#endif // define ARXDBG_UTILS_H

#if 0

const TCHAR* acedGetSetvarVarname(); // defined in command.cpp
// defined in rxmfcapi.cpp - (exported but not published)
DWORD       acedCommandActive();         // direct access to CMDACTIVE sysvar (avoided getvar for speed reasons)
BOOL        acedManualInputProvided(AcApDocument* pAcTargetDocument); // last token was provided by a carbon based life form
BOOL        acedMenuInputProvided();

int rx_getCurrentCommandStateFlags()
{
    //cmd active flags
    int flags = acedCommandActive();

    if (acedMenuInputProvided())
        flags |= CMDSTATE_MENU;

    if (acedManualInputProvided(curDoc()))
        flags |= CMDSTATE_MANUAL;

    return flags;
}

#else

static int iCurrentCommandStateFlagsAppend = 0;

int rx_getCurrentCommandStateFlags()
{
  int flags = 0;
  Acad::ErrorStatus es = getSysVar(_T("CMDACTIVE"), flags);
  ASSERT(es == Acad::eOk);

  flags |= iCurrentCommandStateFlagsAppend;

  //if (flags == 0)
  //  if (getSysVar(_T("CMDDIA"), flags) == Acad::eOk && flags > 0)
  //      flags |= CMDSTATE_DIALOG;
  return flags;
}

int rx_setCurrentCommandStateFlagsAppend(int flag)
{
  int old = iCurrentCommandStateFlagsAppend;
  iCurrentCommandStateFlagsAppend = flag;
  return old;
}

#endif

void rx_extentsArray(const AcDbObjectIdArray& objIdAr, AcDbExtents &extents, bool bBestFitBlk /*=false*/)
{
  AcDbExtents extentsTmp;
  AcDbEntity *pEnt;
  bool first = true;
  for ( int i=0; i < objIdAr.length(); i++ )
  {
    if ( acdbOpenObject(pEnt, objIdAr[i], AcDb::kForRead) == Acad::eOk )
    {
      if (bBestFitBlk && pEnt->isKindOf(AcDbBlockReference::desc()))
        ((AcDbBlockReference*)pEnt)->geomExtentsBestFit(extentsTmp);
      else
        pEnt->getGeomExtents(extentsTmp);
      pEnt->close();
      if (first)
      {
        first = false;
        extents = extentsTmp;
      }
      else
        extents.addExt(extentsTmp);
    }
  }
}

bool rx_extentsBlkRef (const AcDbObjectId &objId, AcDbExtents &extents, bool offAttr /*= true*/)
{
  bool ret = false;
  AcTransaction *pTr = actrTransactionManager->startTransaction();
  AcDbObject* pEnt;
  
  try {
    if ( offAttr )
    {
      AcDbObjectIdArray arAttId;
      ARXOK( pTr->getObject(pEnt, objId, AcDb::kForRead) );
      rx_collectAttributes( (AcDbBlockReference *)pEnt, arAttId);
      pEnt->close();

      for( int i = 0; i < arAttId.length(); i++)
      {
        ARXOK( pTr->getObject( pEnt, arAttId[i], AcDb::kForWrite) );
        //((AcDbAttribute*)pEnt)->setTextString("");
        pEnt->erase();
        pEnt->close();
      }
    }
    ARXOK( pTr->getObject(pEnt, objId, AcDb::kForRead) );
    //((AcDbBlockReference *)pEnt)->getGeomExtents(extents);
    ret = ((AcDbBlockReference *)pEnt)->geomExtentsBestFit(extents) == Acad::eOk ;
    if (!ret)
      ret = ((AcDbBlockReference *)pEnt)->getGeomExtents(extents) == Acad::eOk;
  }
  catch (...)
  {
    acdbFail(_T("исключение getRectBlk()"));
  }
  actrTransactionManager->abortTransaction();
  return ret;
}


void rx_ident_init(ads_matrix id) 
{ 
  int i, j; 
  for (i=0; i<=3; i++)  for (j=0; j<=3; j++)  id[i][j] = 0.0; 
  for (i=0; i<=3; i++)  id[i][i] = 1.0; 
} 

static ads_point ptDragTmp, ptDragTmpLast, ptDragVecTmp;
static bool sbUseDragVec, sbDragVecFirst;


int rx_getDragMoveMatrix(ads_point usrpt, ads_matrix matrix) 
{ 
  rx_ident_init(matrix); // Initialize to identity.
// Initialize translation vector.
  matrix[X][T] = usrpt[X] - ptDragTmp[X]; 
  matrix[Y][T] = usrpt[Y] - ptDragTmp[Y]; 
  matrix[Z][T] = usrpt[Z] - ptDragTmp[Z]; 

  //if (acedIsUpdateDisplayPaused())
  //{
  //  ;
  //}

  if (sbUseDragVec)
  {
    if (sbDragVecFirst)
      sbDragVecFirst = false;
    else
      acedGrDraw(ptDragTmp, ptDragVecTmp, -1, 1);

    copyAdsPt(ptDragVecTmp, usrpt);
    copyAdsPt(ptDragTmpLast, ptDragTmp);

    acedGrDraw(ptDragTmp, usrpt, -1, 1);
  }

  return RTNORM; // Matrix was modified.
} 

static void moveAfterDragGen(ads_name ourset, const AcGePoint3d& pt)
{
  ads_matrix matrix;
  rx_ident_init(matrix); 
  matrix[0][T] = ptDragTmp[X] - pt.x;
  matrix[1][T] = ptDragTmp[Y] - pt.y; 
  matrix[2][T] = ptDragTmp[Z] - pt.z; 
  acedXformSS(ourset, matrix); 
}

#if !defined(_NCAD_USE_DRAG)

int rx_dragGen(LPCTSTR prompt, ads_name sset, AcGePoint3d &pt, bool move /*= false*/, int cursor/* = 0*/, bool bUseDragVec /*= false*/)
{
  ptDragTmp[X] = pt[X]; 
  ptDragTmp[Y] = pt[Y];
  ptDragTmp[Z] = pt[Z];

  sbUseDragVec = bUseDragVec;
  sbDragVecFirst = true;

  int res = acedDragGen(sset, CARETCOLON(prompt), cursor, rx_getDragMoveMatrix, ptDragTmp);

  if (sbUseDragVec && !sbDragVecFirst)
    acedGrDraw(ptDragTmpLast, ptDragVecTmp, -1, 1);

  if (res == RTNORM) 
  {
    if (move) moveAfterDragGen(sset, pt);
    pt = asPnt3d(ptDragTmp);
  }
  rx_refreshDisplay();
  return res;
}

int rx_dragGen(LPCTSTR prompt, const AcDbObjectId& objId, AcGePoint3d &pt, bool move /*= false*/, int cursor/* = 0*/, bool bUseDragVec /*= false*/)
{
  ads_name lname, ourset;
  ourset[0] = ourset[1] = 0;
  int res = RTERROR;
  if (acdbGetAdsName(lname, objId) == Acad::eOk)
    if ( acedSSAdd(NULL, NULL, ourset) == RTNORM) 
    {
      if (acedSSAdd(lname, ourset, ourset) == RTNORM) 
        res = rx_dragGen(prompt, ourset, pt, move, cursor, bUseDragVec);
      acedSSFree(ourset);
    }
  return res;
}

int rx_dragGen(LPCTSTR  prompt, const AcDbObjectIdArray& objIdAr, AcGePoint3d &pt, bool move /*= false*/, int cursor/* = 0*/, bool bUseDragVec /*= false*/)
{
  ads_name lname, ourset;
  ourset[0] = ourset[1] = 0;
  int res = acedSSAdd(NULL, NULL, ourset);
  if ( res == RTNORM) 
  {
    for(int i=0; i < objIdAr.length(); i++ )
      if ( acdbGetAdsName(lname, objIdAr[i]) != Acad::eOk  ||
           acedSSAdd(lname, ourset, ourset) != RTNORM)
      {
        acedSSFree(ourset);
        return RTERROR;
      }
    res = rx_dragGen(prompt, ourset, pt, move, cursor, bUseDragVec);
    acedSSFree(ourset);
  }
  return res;
}
#endif //_NCAD_BUILD_

//bool rx_addBlockReference(const AcDbObjectId& blockId, double angle/*=0.0*/, const AcGePoint3d *insertPoint/*= NULL*/)
//{
//  AcGePoint3d basePoint;
//
//  AcDbBlockReference *pBlkRef = new AcDbBlockReference;
//  pBlkRef->setBlockTableRecord(blockId);
//
//  // Give it the current UCS normal.
//  //
//  struct resbuf to, from;
//  from.restype = RTSHORT;
//  from.resval.rint = 1; // UCS
//  to.restype = RTSHORT;
//  to.resval.rint = 0; // WCS
//  AcGeVector3d normal(0.0, 0.0, 1.0);
//  acedTrans(&(normal.x), &from, &to, Adesk::kTrue, &(normal.x));
//
//  pBlkRef->setPosition(basePoint);
//
//  // Indicate the LCS 0.0 angle, not necessarily the UCS 0.0 angle.
//  //
//  pBlkRef->setRotation(0.0);
//  pBlkRef->setNormal(normal);
//
//  AcDbBlockTable *pBlockTable;
//  acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);
//  AcDbBlockTableRecord *pBlockTableRecord;
//  pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
//  pBlockTable->close();
//  // Append the block reference to the model space block Table Record.
//  AcDbObjectId newEntId;
//  pBlockTableRecord->appendAcDbEntity(newEntId, pBlkRef);
//  pBlockTableRecord->close();
//
//  AcDbBlockTableRecord *pBlockDef;
//  acdbOpenObject(pBlockDef, blockId, AcDb::kForRead);
//  // Set up a block table record iterator to iterate over the attribute definitions.
//  AcDbBlockTableRecordIterator *pIterator;
//  pBlockDef->newIterator(pIterator);
//  AcDbEntity *pEnt;
//  AcDbAttributeDefinition *pAttdef;
//  for (pIterator->start(); !pIterator->done(); pIterator->step())
//  {
//    pIterator->getEntity(pEnt, AcDb::kForRead);
//    // Make sure the entity is an attribute definition and not a constant.
//    pAttdef = AcDbAttributeDefinition::cast(pEnt);
//    if (pAttdef != NULL && !pAttdef->isConstant()) 
//    {
//      // We have a non-constant attribute definition,
//      // so build an attribute entity.
//      //
//      AcDbAttribute *pAtt = new AcDbAttribute();
//      pAtt->setPropertiesFrom(pAttdef);
//      pAtt->setInvisible(pAttdef->isInvisible());
//      // Translate the attribute by block reference.
//      // To be really correct, the entire block
//      // reference transform should be applied here.
//      //
//      basePoint = pAttdef->position();
//      basePoint += pBlkRef->position().asVector();
//      pAtt->setPosition(basePoint);
//      pAtt->setHeight(pAttdef->height());
//      pAtt->setRotation(pAttdef->rotation());
//      pAtt->setTag(_T("Tag"));
//      pAtt->setFieldLength(25);
//
//      TCHAR *pStr = pAttdef->tag();
//      pAtt->setTag(pStr);
//      free(pStr);
//      pAtt->setFieldLength(pAttdef->fieldLength());
//
//      // The database column value should be displayed.
//      // INSERT prompts for this.
//      //
//      pStr = pAttdef->textString();
//      pAtt->setTextString(pStr);
//      free(pStr);
//
//      resbuf *xdata =pAttdef->xData();
//      pAtt->setXData(xdata);
//      acutRelRb(xdata);
//
//
//      AcDbObjectId attId;
//      pBlkRef->appendAttribute(attId, pAtt);
//      pAtt->close();
//    }
//    pEnt->close(); // use pEnt... pAttdef might be NULL
//  }
//  delete pIterator;
//  pBlockDef->close();
//  pBlkRef->close();
//
//  rx_refreshDisplay();
//
//  if ( angle != 0.0 || !insertPoint)
//  {
//    if ( !insertPoint )   basePoint.set(0.0, 0.0, 0.0);
//    else                  basePoint = *insertPoint;
//    angle = angle*(PI/180.0);
//    AcGePoint3d pt;
//    ads_name lname, ourset; 
//    if (acdbEntLast(lname) != RTNORM) 
//      return false; 
//    AcDbObjectId objId;
//    acdbGetObjectId(objId, lname);
//    if (newEntId != objId)
//      return false; 
//    acdbOpenObject(pBlkRef, objId, AcDb::kForWrite);
//    pBlkRef->setRotation(angle);
//
//    AcDbObjectIterator *attribIter = pBlkRef->attributeIterator();
//    pBlkRef->close();
//    AcDbAttribute *pAtt;
//    for (attribIter->start(); !attribIter->done(); attribIter->step())
//    {
//      AOK ( acdbOpenObject(pAtt, attribIter->objectId(), AcDb::kForWrite));
//      pt = pAtt->position().rotateBy(angle, normal);
//      pAtt->setPosition( pt + basePoint.asVector());
//      pAtt->close();
//    }
//    delete attribIter;
//
//    if ( !insertPoint )
//    {
//      ptDragTmp[X] = basePoint[X]; 
//      ptDragTmp[Y] = basePoint[Y];
//      ptDragTmp[Z] = basePoint[Z];
//
//      if (acedSSAdd(NULL, NULL, ourset) != RTNORM) { acedSSFree(ourset); return false; }
//      if (acedSSAdd(lname, ourset, ourset) != RTNORM) { acedSSFree(ourset); return false; }
//      if (acedDragGen(ourset, _T("\n”кажите точку вставки Ѕѕќ ‘√"), 0, rx_getDragMoveMatrix, ptDragTmp) != RTNORM)
//      { acedSSFree(ourset); return false; }
//      acedSSFree(ourset);
//      basePoint = asPnt3d(ptDragTmp);
//      acdbOpenObject(pBlkRef, objId, AcDb::kForWrite);
//      pBlkRef->setPosition(basePoint);
////      if (!insertPoint) 
////        pBlkRef->setVisibility(AcDb::kInvisible);
////      pBlkRef->setVisibility(AcDb::kVisible);
//      AcDbObjectIterator *attribIter = pBlkRef->attributeIterator();
//      pBlkRef->close();
//      AcDbAttribute *pAtt;
//      for (attribIter->start(); !attribIter->done(); attribIter->step())
//      {
//        AOK ( acdbOpenObject(pAtt, attribIter->objectId(), AcDb::kForWrite));
//        pAtt->setPosition( pAtt->position() + basePoint.asVector() );
//        pAtt->close();
//      }
//    }
//  }
//  return true;
//}

bool rx_exlode(AcDbEntity* pEnt, bool bErase, AcDbObjectIdArray* pObjIdAr /*= NULL*/, AcDbDatabase *pDb /*= NULL*/)
{
  AcDbEntity* pEntExp = NULL;
  try
  {
    AcDbVoidPtrArray entitySet;
    ARXOK(pEnt->explode(entitySet));
    for (int i = 0; i < entitySet.length(); i++)
    {
      pEntExp = AcDbEntity::cast((AcRxObject *)entitySet[i]);
      if (pEntExp == NULL)
        delete entitySet[i];
      else
      {
        AcDbObjectId objId;
        ARXOK( postToDb(pEntExp, objId, pDb) );
        if (pObjIdAr)
          pObjIdAr->append(objId);
      }
    }
    if (bErase) pEnt->erase();
    return true;
  }
  catch(...)
  {
    if (pEntExp) delete pEntExp;
    rxIfAcASSERT(Acad::eOutOfMemory, _T("rx_explode()")) ARXOK(Acad::eOutOfMemory);
    return false;
  }
}


//#ifndef _NCAD_BUILD_

bool rx_explodeCmd(ads_name en,
                   AcDbObjectIdArray* pObjIdAr /*= NULL*/,
                   bool eraseBlkRef /*= false*/, bool eraseBlkDef /*= false*/, bool bCmdEchoOff/*=false*/)
{
  bool bRet = false;

  bCmdEchoOff = true;

  int svCmdEcho = 0;
  if (bCmdEchoOff)
    bCmdEchoOff = getSysVar(AcadVar::cmdecho, svCmdEcho) == Acad::eOk;
  if (bCmdEchoOff)
    bCmdEchoOff = svCmdEcho != 0;
  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, 0);

  AcApDocument *pDoc = curDoc();
  ASSERT(pDoc != NULL);
  ASSERT(pDoc->lockMode() == AcAp::kNotLocked);
  //AcAp::DocLockMode lm = pDoc->lockMode();
  //ASSERT(lm == AcAp::kNotLocked);
  AcAp::DocLockMode lmm = pDoc->myLockMode();
  //ASSERT(lmm == AcAp::kNotLocked);
  AcAp::DocLockMode lmS = pDoc->lockMode(true);

  try
  {
    if (lmS != AcAp::kNotLocked )
    {
      //ARXOK(acDocManager->unlockDocument(pDoc));
      //ARXOK(acDocManager->lockDocument(pDoc, AcAp::kNotLocked));
    }

    AcDbObjectId lastId;
    if (pObjIdAr)
      rx_lastId(lastId);

    //rx_clearPickFirst();

    //ArxDbgSelSet ass;
    //ass.createEmptySet();
    //ass.add(en);
    //ASSERT(ass.length() == 1);
    //ads_name ss;
    //ass.asAdsName(ss);
    //ADSOK(acedSSSetFirst(ss, NULL));
    //ADSOK(acedCommand(RTSTR, _T("_explode"), RTSTR, _T(""),  0 ));

    //ads_queueexpr(_T("_explode _p"));

#ifdef _NCAD_BUILD_
    ADSOK( acedCommand(RTSTR, _T("_explode"), RTENAME, en, RTSTR, _T(""),  0 ) );
#else
    ADSOK( acedCommand(RTSTR, _T("_explode"), RTENAME, en, 0 ) );
    //ads_queueexpr(_T("(command \"_.explode\" \"_l\")"));
#endif

    AcDbObjectId objIdDef;
    if (eraseBlkRef || eraseBlkDef)
    {
      AcDbObjectId objId;
      ARXOK(acdbGetObjectId(objId, en));
      AcDbBlockReferencePtr blkRef;
      if(rxOpenObject(blkRef, objId, AcDb::kForWrite, true) == Acad::eOk)
      {
        objIdDef = blkRef->blockTableRecord();
        if (eraseBlkRef && !blkRef->isErased())
          blkRef->erase();
      }
    }

    if (eraseBlkDef && objIdDef.isValid())
    {
      AcDbBlockTableRecordPtr blkRec;
      if(rxOpenObject(blkRec, objIdDef, AcDb::kForWrite) == Acad::eOk)
      {
        AcDbObjectIdArray refIds;
        blkRec->getBlockReferenceIds(refIds);
        if (refIds.length() == 0)
          blkRec->erase();
      }
    }

    //actrTransactionManager->queueForGraphicsFlush();
    //actrTransactionManager->flushGraphics();

    if (pObjIdAr)
    {
      rx_collectEntites(*pObjIdAr, lastId, true);
      rx_redraw(*pObjIdAr);
    }
    else
    {
      AcDbObjectIdArray objIdAr;
      rx_collectEntites(objIdAr, lastId, true);
      rx_redraw(objIdAr);
    }

    bRet = true;
  }
  CATCH_BLOCKALL

 if (lmS != AcAp::kNotLocked)
 {
   //ARXOK(acDocManager->lockDocument(pDoc, lmS));
   //ARXOK(acDocManager->unlockDocument(pDoc));
 }

  if (bCmdEchoOff)
    setSysVar(AcadVar::cmdecho, svCmdEcho);

  return bRet;
}

bool rx_explodeCmd(const AcDbObjectId& objId,
                   AcDbObjectIdArray* pObjIdAr /*= NULL*/,
                   bool eraseBlkRef /*= false*/, bool eraseBlkDef /*= false*/, bool bCmdEchoOff/*=false*/)
{
  ads_name en;
  if (acdbGetAdsName(en, objId) == Acad::eOk)
    return rx_explodeCmd(en, pObjIdAr, eraseBlkRef, eraseBlkDef, bCmdEchoOff);
  return false;
}
//#endif

Acad::ErrorStatus rx_explodeBlkRef(const AcDbObjectId& blkRefId,
                                   AcDbObjectIdArray* pObjIdAr /*= NULL*/, 
                                   bool eraseBlkRef /*= false*/, bool eraseBlkDef /*= false*/)
{
  if (pObjIdAr) pObjIdAr->setLogicalLength(0);
  try
  {
    AcDbBlockReferencePtr pBlkRef;
    ARXOK(rxOpenObject(pBlkRef, blkRefId, AcDb::kForWrite));

#if defined(_ACAD2000) || defined(_NCAD_BUILD_)
    AcDbVoidPtrArray entitySet;
    ARXOK(pBlkRef->explode(entitySet));
    for (int i = 0; i < entitySet.length(); i++)
    {
      AcDbEntityPtr pEnt; pEnt.acquire((AcDbEntity*&)entitySet[i]);
      if (pEnt != NULL)
      {
        AcDbObjectId objId;
#ifdef _NCAD_BUILD_
        ARXOK( addToDb(pEnt, objId) );
#else
        ARXOK( postToDb(pEnt, objId) );
#endif
        if (pObjIdAr) pObjIdAr->append(objId);
      }
      //else
      //  delete entitySet[i];
    }
#else
    AcDbObjectId objIdLast;
    if (pObjIdAr)
      rx_lastId(objIdLast);
    ARXOK(pBlkRef->explodeToOwnerSpace());
    if (pObjIdAr)
      rx_collectEntites(*pObjIdAr, objIdLast, true);
#endif

    AcDbObjectId objId = pBlkRef->blockTableRecord();
    if (eraseBlkRef)
      pBlkRef->erase();
    rxCloseObject(pBlkRef);

    if (eraseBlkRef && eraseBlkDef)
    {
      AcDbBlockTableRecordPtr pBlkRec;
      ARXOK(rxOpenObject(pBlkRec, objId, AcDb::kForWrite));
      AcDbObjectIdArray ids;
      if (pBlkRec->getBlockReferenceIds(ids, true, true) == Acad::eOk && ids.length() == 0)
        pBlkRec->erase();
    }
  }
  catch(Acad::ErrorStatus es)
  {
    PRINT_STATUS_ERROR(es);
    return es;
  }
  return Acad::eOk;
}

// Set some viewport information.
void rx_setSomeViewport(AcDbDatabase* pDb /*= NULL*/)
{
  if (!pDb) pDb = acdbHostApplicationServices()->workingDatabase();
  if (!pDb) return;
  AcDbViewportTable* pViewportTable;
  if (pDb->getViewportTable(pViewportTable, AcDb::kForRead) == Acad::eOk)
  {
    // Find the first viewport and open it for write. 
    AcDbViewportTableRecord *pRecord;
    if (pViewportTable->getAt( _T("*ACTIVE"), pRecord, AcDb::kForWrite) == Acad::eOk)
    {
        pRecord->setCenterPoint(AcGePoint2d(0.5, 0.5));
        pRecord->setHeight(1.0); 
        pRecord->setWidth(1.0); 
        pRecord->close(); 
    } 
    pViewportTable->close();
  }
}

bool rx_getTransformBlkRef(const AcDbObjectId &blkRefId, AcGeMatrix3d& blockTransform)
{
  AcDbBlockReference *blkRef;
  if ( acdbOpenObject(blkRef, blkRefId, AcDb::kForRead) != Acad::eOk) 
    return false;
  blockTransform = blkRef->blockTransform();
  blkRef->close();
  return true;
}

//-------------------------------------------------------------------------------------------

bool rx_groupGenerateUniqieName(AcDbDatabase *pDb, LPCTSTR prefix, CString& name)
{
  name.Empty();
  AcDbDictionary *pGroupDict;
  if ( pDb->getGroupDictionary(pGroupDict, AcDb::kForRead) != Acad::eOk)
    return false;
  int idx = 1;
  do 
  {
    name.Format(_T("%s_w%d"), prefix, idx++);
  } while(pGroupDict->has(name));
  pGroupDict->close();
  return true;
}

Acad::ErrorStatus rx_groupMake(AcDbDatabase *pDb, const AcDbObjectIdArray& objIdAr,  LPCTSTR  name /*= NULL*/, AcDbObjectId* id /*= NULL*/)
{
  Acad::ErrorStatus es = Acad::eNullObjectId; 
  if ( id ) id->setNull();
  if (objIdAr.length() > 0)
  {
    AcDbGroup *pGroup = new AcDbGroup;

    AcDbDictionary *pGroupDict;
    pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
    AcDbObjectId groupId;
    es = pGroupDict->setAt(name?name:_T("W_ANONYMOUS"), pGroup, groupId);
    pGroupDict->close();

    if (es == Acad::eOk)
    {
      if (!name)
      {
        pGroup->close();

        ARXOK(acdbOpenObject(pGroup, groupId, AcDb::kForWrite));
        pGroup->setAnonymous();
      }
      es = pGroup->append(objIdAr);

      pGroup->close();
      if ( id ) *id = groupId;
    }
  }
  return es;
}

Acad::ErrorStatus rx_groupCollectEntites(const AcDbObjectId& groupId, AcDbObjectIdArray &ids)
{
  AcDbGroup *pGroup;
  ids.setLogicalLength(0);
  Acad::ErrorStatus es = acdbOpenObject(pGroup, groupId, AcDb::kForRead);
  if ( es == Acad::eOk)
  {
    pGroup->allEntityIds(ids);
    pGroup->close();
    //for (int i=0; i < ids.length(); i++)
    //  if ( !ids[i].isValid() || ids[i].isErased())
    //    ids.removeAt(i--);
  }
  return es;
}

bool rx_groupIsExists(AcDbDatabase *pDb, LPCTSTR name, AcDbObjectIdArray* ids /*= NULL*/, AcDbObjectId* id /*= NULL*/)
{
  ASSERT(name != NULL);
  bool has = false;
  AcDbDictionary *pGroupDict;
  if (pDb->getGroupDictionary(pGroupDict, AcDb::kForRead) == Acad::eOk)
  {
    has = pGroupDict->has(name);
    if (has && (ids || id))
    {
      AcDbObjectId groupId;
      if ( pGroupDict->getAt(name, groupId) == Acad::eOk )
      {
        if ( id ) *id = groupId;
        if ( ids ) rx_groupCollectEntites(groupId, *ids);
      }
    }
    pGroupDict->close();
  }
  return has;
}

Acad::ErrorStatus rx_groupDelete(AcDbDatabase *pDb, LPCTSTR name, bool removeWitchObjects /*= true*/)
{
  ASSERT(name != NULL);
  Acad::ErrorStatus es = Acad::eOk; 
  AcDbDictionary *pGroupDict;
  es = pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
  if ( es != Acad::eOk)
    return es;
   
  AcDbGroup *pGroup;
  AcDbObjectId groupId;
  es = pGroupDict->getAt(name, groupId);
  if ( es == Acad::eOk && acdbOpenObject(pGroup, groupId, AcDb::kForWrite) == Acad::eOk)
  {
    if ( removeWitchObjects)
    {
      AcDbObjectIdArray ids;
      if ( pGroup->allEntityIds(ids) )
      {
        AcDbEntity *ent;
        for (int i=0; i < ids.length(); i++)
        {
          if (acdbOpenObject(ent, ids[i], AcDb::kForWrite) == Acad::eOk)
          {
            ent->erase();
            ent->close();
          }
        }
      }
    }
    pGroup->clear();
    pGroup->erase();
    pGroup->close();
  }
  es = pGroupDict->remove(name);
  pGroupDict->close();
  return es;
}


Acad::ErrorStatus rx_groupFindEntity(AcDbDatabase *pDb, const AcDbObjectId& objId, AcDbObjectIdArray& resGroupIds, LPCTSTR  mask /*="*"*/)
{
  Acad::ErrorStatus es = Acad::eOk; 
  resGroupIds.setLogicalLength(0);

  AcDbDictionary *pGroupDict;
  es = pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
  if ( es != Acad::eOk)
    return es;
   
  AcDbObjectId groupId;
  AcDbObjectIdArray ids;

  AcDbObject* obj;
  AcDbDictionaryIterator* dictIter = pGroupDict->newIterator();
  ASSERT(dictIter != NULL);
  if (dictIter != NULL) 
  {
    for (; !dictIter->done(); dictIter->next()) 
    {
      if (strIsWild(dictIter->name(), mask))
      if (dictIter->getObject(obj, AcDb::kForRead) == Acad::eOk)
      {
        if (obj->isKindOf( AcDbGroup::desc() ) )
        {
          ids.setLogicalLength(0);
          if ( AcDbGroup::cast(obj)->allEntityIds(ids) &&
               ids.contains(objId) )
            resGroupIds.append(dictIter->objectId());
        }
        obj->close();
      }
    }
#ifndef _NCAD_BUILD_NODELETE
    delete dictIter;
#endif
    es = Acad::eOk;
  }

  pGroupDict->close();
  return es;
}


Acad::ErrorStatus rx_groupFindEntity(AcDbDatabase *pDb, const AcDbObjectId& objId, SdStrObjIdList& list, LPCTSTR  mask /*="*"*/)
{
  Acad::ErrorStatus es = Acad::eOk; 

  AcDbDictionary *pGroupDict;
  es = pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
  if ( es != Acad::eOk)
    return es;
   
  AcDbObjectId groupId;
  AcDbObjectIdArray ids;

  AcDbObject* obj;
  AcDbDictionaryIterator* dictIter = pGroupDict->newIterator();
  ASSERT(dictIter != NULL);
  if (dictIter != NULL) 
  {
    for (; !dictIter->done(); dictIter->next()) 
    {
      if (strIsWild(dictIter->name(), mask))
      if (dictIter->getObject(obj, AcDb::kForRead) == Acad::eOk)
      {
        if ( obj->isKindOf( AcDbGroup::desc() ) )
        {
          ids.setLogicalLength(0);
          if ( AcDbGroup::cast(obj)->allEntityIds(ids) &&
               ids.contains(objId) )
            list.AddAlpha(dictIter->name(), dictIter->objectId());
        }
        obj->close();
      }
    }
#ifndef _NCAD_BUILD_NODELETE
    delete dictIter;
#endif
    es = Acad::eOk;
  }

  pGroupDict->close();
  return es;
}

Acad::ErrorStatus rx_groupCollect(AcDbDatabase *pDb, SdStrObjIdList& list, LPCTSTR  mask /*="*"*/)
{
  Acad::ErrorStatus es = Acad::eOk; 

  AcDbDictionary *pGroupDict;
  es = pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
  if ( es != Acad::eOk)
    return es;
   
  AcDbObjectId groupId;
  AcDbObjectIdArray ids;

  AcDbObject* obj;
  AcDbDictionaryIterator* dictIter = pGroupDict->newIterator();
  ASSERT(dictIter != NULL);
  if (dictIter != NULL) 
  {
    for (; !dictIter->done(); dictIter->next()) 
    {
      if (strIsWild(dictIter->name(), mask))
      if (dictIter->getObject(obj, AcDb::kForRead) == Acad::eOk)
      {
        if ( obj->isKindOf( AcDbGroup::desc() ) )
          list.AddAlpha(dictIter->name(), dictIter->objectId());
        obj->close();
      }
    }
#ifndef _NCAD_BUILD_NODELETE
    delete dictIter;
#endif
    es = Acad::eOk;
  }

  pGroupDict->close();
  return es;
}

Acad::ErrorStatus rx_groupClearAllEmpty(AcDbDatabase *pDb)
{
  Acad::ErrorStatus es = Acad::eOk; 

  AcDbDictionary *pGroupDict;
  es = pDb->getGroupDictionary(pGroupDict, AcDb::kForWrite);
  if ( es != Acad::eOk)
    return es;
   
  AcDbObject* obj;
  AcDbDictionaryIterator* dictIter = pGroupDict->newIterator();
  ASSERT(dictIter != NULL);
  if (dictIter != NULL) 
  {
    for (; !dictIter->done(); dictIter->next()) 
    {
      if (dictIter->getObject(obj, AcDb::kForWrite) == Acad::eOk)
      {
        if ( obj->isKindOf( AcDbGroup::desc() ) &&  AcDbGroup::cast(obj)->numEntities() == 0) 
          obj->erase();
        obj->close();
      }
    }
#ifndef _NCAD_BUILD_NODELETE
    delete dictIter;
#endif
    es = Acad::eOk;
  }

  pGroupDict->close();
  return es;
}

Acad::ErrorStatus rx_getNOD (AcDbDictionary *&pNOD, const AcDb::OpenMode mode, bool bTransaction) 
{
  AcDbObjectId idO = WORKDWG->namedObjectsDictionaryId();
  if (bTransaction)
    return (actrTransactionManager->getObject ((AcDbObject *&)pNOD, idO, mode));
  else
    return (acdbOpenAcDbObject ((AcDbObject *&)pNOD, idO, mode));
}

//////////////////////////////////////////////////////////////////////////

bool rx_mslide(LPCTSTR fName /*= _T("")*/, bool bAsWMF /*=false*/,
               LPCTSTR sZoomOpt /*= "_e"*/, bool bExludeXREF /*= false*/, AcDbObjectIdArray* pObjIds /*= NULL*/,
               int *pWMFFOREGND /*=NULL*/, int *pWMFBKGND /*=NULL*/)
{
  AcApDocument* pDoc = curDoc();
  CString s = fName;
  if (s.IsEmpty())
    s = pDoc->fileName();
  if (s.IsEmpty())
    return false;
  FileChangeExt(s, bAsWMF ? _T(".wmf") : _T(".sld"));
  DeleteFile(s);

#ifdef _NCAD_BUILD_
  if (!bAsWMF)
  {
     //ArxDbgUtils::stubMsg(_T(" NC:rx_mslide()"));
     return false;
  }
#endif

  pDoc->pushDbmod();

  //SdSysVarStack svs;
  //svs.push(AcadVar::cmdecho, 0);
  //int cmdecho;//, lwdisplay;
  //getSysVar(AcadVar::cmdecho, cmdecho);
  //setSysVar(AcadVar::cmdecho, 0);
  ////getSysVar(AcadVar::lwdisplay, lwdisplay); //08.08.2011  Acad12 не поддерживаеи эту переременную
  ////setSysVar(AcadVar::lwdisplay, 0);
  if (bAsWMF)
  {
    ads_name ss;
    AcDbObjectIdArray objIds;
    if (pObjIds)
      objIds = *pObjIds;
    else
      rx_collectEntites(objIds, AcDbEntity::desc(), false);

    AcDbExtents ext, extDb;
    bool bExtFirst = true;

    if ( objIds.length() > 0 )
    {
      acedSSAdd(NULL,NULL,ss);
      for (int i=0; i < objIds.length(); i++)
      {
        //if (bExludeXREF)
        {
          AcDbEntityPtr pEnt;
          if (rxOpenObject(pEnt, objIds[i]) == Acad::eOk)
          {
            AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEnt);
            if (pBlkRef)
            {
              AcDbBlockTableRecordPtr pBTR;
              if (rxOpenObject(pBTR, pBlkRef->blockTableRecord()) == Acad::eOk)
                if (pBTR->isFromExternalReference())
                {
                  objIds.removeAt(i--);
                  continue;
                }
            }
            if (pEnt->getGeomExtents(ext) == Acad::eOk)
            {
              if (bExtFirst)
              {
                bExtFirst = false;
                extDb = ext;
              }
              else
                extDb.addExt(ext);
            }
          }
        }
        ads_name eName;
        acdbGetAdsName(eName,objIds[i]);
        acedSSAdd(eName,ss,ss);
      }
    }

    if (objIds.length() > 0)
    {
      SdSysVarStack svs;
      svs.push(AcadVar::cmdecho, 0);

#ifdef _NCAD_BUILD_

      //rxncm_wmfout(WORKDWG, objIds, extDb, s); /////// ???????????????????


      //acedCommand(RTSTR, _T("_.zoom"), RTSTR, _T("_Object"), RTPICKS, ss, RTSTR, _T(""), RTNONE);
      //acedCommand(RTSTR, _T("_.wmfout"), RTSTR, s, RTPICKS, ss, RTSTR, _T(""), RTNONE);
      //acedCommand(RTSTR, _T("_.zoom"), RTSTR, _T("_p"), RTNONE);

#else //_NCAD_BUILD_

      //int WMFFOREGND = 0;
      if (pWMFFOREGND && (*pWMFFOREGND == 0 || *pWMFFOREGND == 1))
      {
        //getSysVar(_T("WMFFOREGND"), WMFFOREGND);
        //setSysVar(_T("WMFFOREGND"), 1);
        svs.push(_T("WMFFOREGND"), *pWMFFOREGND);
      }
      //int WMFBACKGND = 0;
      if (pWMFBKGND && (*pWMFBKGND == 0 || *pWMFBKGND == 1))
      {
        //getSysVar(_T("WMFBKGND"), WMFBKGND);
        //setSysVar(_T("WMFBKGND"), 1);
        svs.push(_T("WMFBKGND"), *pWMFBKGND);
      }

#ifdef _ACAD2000
      acedCommand(RTSTR, _T("_.zoom"), RTSTR, sZoomOpt, RTNONE);
#else
      acedCommand(RTSTR, _T("_.zoom"), RTSTR, _T("_Object"), RTPICKS, ss, RTSTR, _T(""), RTNONE);
#endif
      //acedCommand(RTSTR, _T("_.wmfout"), RTSTR, s, RTSTR, _T("_all"), RTSTR, _T(""), RTNONE);
      acedCommand(RTSTR, _T("_.wmfout"), RTSTR, s, RTPICKS, ss, RTSTR, _T(""), RTNONE);

      acedCommand(RTSTR, _T("_.zoom"), RTSTR, _T("_p"), RTNONE);

      //setSysVar(_T("WMFFOREGND"), WMFFOREGND);
      //setSysVar(_T("WMFBACKGND"), WMFBACKGND);

#endif //!_NCAD_BUILD_
    }
    else
    {
      rxDPRINT(_T("„истый WMF"));
      HWND hWnd = adsw_acadDocWnd();
      HDC hdcRef = GetDC(hWnd);
      CRect rect;
      GetClientRect(hWnd, &rect); 
      HDC hdcMeta = CreateEnhMetaFile(hdcRef, s, &rect, _T("")); 
      if (hdcMeta) 
      {
        CDC* pDC = CDC::FromHandle(hdcMeta);
        if (pDC)
        {
  	      CBrush brush;
			    if(brush.CreateSolidBrush(RGB(0,0,0)))
          {
				    pDC->FillRect(&rect,&brush) ;
			      brush.DeleteObject();
          }
        }
        DeleteEnhMetaFile(CloseEnhMetaFile(hdcMeta)); 
      }
      ReleaseDC(hWnd, hdcRef);
    }

    acedSSFree(ss);
  }
  else
  {
    SdSysVarStack svs;
    svs.push(AcadVar::cmdecho, 0);

    acedCommand(RTSTR, _T("_zoom"), RTSTR, sZoomOpt, RTNONE);
    acedCommand(RTSTR, _T("_mslide"), RTSTR, s, RTNONE);
    acedCommand(RTSTR, _T("_zoom"), RTSTR, _T("_p"), RTNONE);
  }
  //setSysVar(AcadVar::cmdecho, cmdecho);
  ////setSysVar(AcadVar::lwdisplay, lwdisplay);
  pDoc->popDbmod();

  return FileExists(s);
}

//////////////////////////////////////////////////////////////////////////

void rxZoom(ads_point pt, double magnf)
{
  AcDbViewTableRecord view;
  struct resbuf rb;
  struct resbuf wcs, dcs, ccs; // acedTrans coord system flags
  ads_point vpDir;
  ads_point ptmax;
  AcGeVector3d viewDir;
  AcGePoint2d cenPt;
  ads_real lenslength, viewtwist, frontz, backz;
  ads_point target;
  int /*viewmode,*/tilemode, cvport; // Get window to zoom to:
  wcs.restype = RTSHORT; // WORLD coord system flag
  wcs.resval.rint = 0;
  ccs.restype = RTSHORT; // CURRENT coord system flag
  ccs.resval.rint = 1;
  dcs.restype = RTSHORT; // DEVICE coord system flag
  dcs.resval.rint = 2; // Get the VPOINT' direction vector

  acedGetVar(_T("VIEWDIR"), &rb);
  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, vpDir);
  viewDir.set(vpDir[X], vpDir[Y], vpDir[Z]);
  acedTrans(pt, &ccs, &dcs, 0, ptmax);

  // set center point of view
  cenPt[X] = ptmax[X];
  cenPt[Y] = ptmax[Y];
  view.setCenterPoint(cenPt);
  view.setHeight(magnf*1.2);
  view.setWidth(magnf*1.2);
  view.setViewDirection(viewDir);

  // get and set other properties
  acedGetVar(_T("LENSLENGTH"), &rb);
  lenslength = rb.resval.rreal;
  view.setLensLength(lenslength);

  acedGetVar(_T("VIEWTWIST"), &rb);
  viewtwist = rb.resval.rreal;
  view.setViewTwist(viewtwist);

  acedGetVar(_T("FRONTZ"), &rb);
  frontz = rb.resval.rreal;

  acedGetVar(_T("BACKZ"), &rb);
  backz = rb.resval.rreal;
  view.setPerspectiveEnabled(false);
  view.setFrontClipEnabled(false);
  view.setBackClipEnabled(false);
  view.setFrontClipAtEye(false);

  acedGetVar(_T("TILEMODE"), &rb);
  tilemode = rb.resval.rint;

  acedGetVar(_T("CVPORT"), &rb);
  cvport = rb.resval.rint;

  // Paperspace flag
  bool paperspace = ((tilemode == 0) && (cvport == 1)) ? Adesk::kTrue:Adesk::kFalse;
  view.setIsPaperspaceView(paperspace);
  if (Adesk::kFalse == paperspace) {
    view.setFrontClipDistance(frontz);
    view.setBackClipDistance(backz);
  }
  else{
    view.setFrontClipDistance(0.0);
    view.setBackClipDistance(0.0);
  }
  acedGetVar(_T("TARGET"), &rb);
  acedTrans(rb.resval.rpoint,&ccs, &wcs, 0, target);
  view.setTarget(AcGePoint3d(target[X], target[Y], target[Z]));
  // update view
  acedSetCurrentView(&view, NULL);
}

bool rx_extentsIsValid(const AcDbExtents& ext)
{
  AcDbExtents nExt;
  return (ext.minPoint() != nExt.minPoint()) && (ext.maxPoint() != nExt.maxPoint());
}

void rx_extentsByLim(AcDbExtents& ext, AcDbDatabase* pDb)
{
  if (!pDb) pDb = WORKDWG;
  AcGePoint2d pMin = pDb->limmin(), pMax = pDb->limmax();
  ext.set(AcGePoint3d(pMin.x, pMin.y, 0), AcGePoint3d(pMax.x, pMax.y, 0) );
}

bool rx_CurViewPortDataUpdate(AcDbViewTableRecord& cViewTableRecord, AcDbViewTableRecord *pViewTableRecord, bool bRead)
{
  if (!WORKDWG->tilemode())
  {
    acutPrintf(_T("\nThis command is only for Model space (i.e tilemode)\n"));
    return false;
  }

  Acad::ErrorStatus es;
  AcDbViewTableRecord& vtr = cViewTableRecord;

  if (bRead)
  {
    es = acedVports2VportTableRecords();
    if (es != Acad::eOk)
      return false;

#if defined(_ACAD2000) || defined(_ACAD2004)
    AcDbViewportTableRecordPointer ptrVp(acedGetCurViewportObjectId(), AcDb::kForRead);
#else
    AcDbViewportTableRecordPointer ptrVp(acedActiveViewportId(), AcDb::kForRead);
#endif
    if (!ptrVp)
      return false;

    // Copy data from the ViewportTableRecord
    vtr.setBackClipDistance(ptrVp->backClipDistance());  
    vtr.setBackClipEnabled(ptrVp->backClipEnabled());   
    vtr.setElevation(ptrVp->elevation());   
    vtr.setFrontClipAtEye(ptrVp->frontClipAtEye());   
    vtr.setFrontClipDistance(ptrVp->frontClipDistance());  
    vtr.setPerspectiveEnabled(ptrVp->perspectiveEnabled());  
#ifndef _ACAD2015
    vtr.setRenderMode(ptrVp->renderMode());  
#endif
    vtr.setUcs(ptrVp->ucsName());   
    vtr.setViewTwist(ptrVp->viewTwist());    
    vtr.setCenterPoint(ptrVp->centerPoint());  
    vtr.setLensLength(ptrVp->lensLength());   

    vtr.setWidth(ptrVp->width());
    vtr.setHeight(ptrVp->height());
    vtr.setTarget(ptrVp->target()); 
    vtr.setViewDirection(ptrVp->viewDirection());  
    //// The main settings
    //double aspectRatio = vtr.width() / vtr.height();
    //double newWidth, newHeight;
    //if (aspectRatio > 1)
    //{
    //  // The height of the green circle we want to include in the picture is 40 
    //  newHeight = 40; 
    //  newWidth = newHeight * aspectRatio;
    //}
    //else
    //{
    //  newWidth = 40;
    //  newHeight = newWidth / aspectRatio;
    //}
    //vtr.setWidth(newWidth);
    //vtr.setHeight(newHeight); 
    //vtr.setTarget(ptTarget); 
    //vtr.setViewDirection(ptCamera - ptTarget);  
  }
  else
  {
    es = acedSetCurrentView(&vtr, NULL);
  }
  return (es == Acad::eOk);
}

void rx_GetCurViewPortData(ads_point selPt,
                           AcGeVector3d& viewDir, AcGePoint2d& cenPt, ads_real& vScale,
                           ads_real& vHeight, ads_real& vWidth,
                           ads_point& target,
                           bool& paperspace,
                           ads_real& lenslength, ads_real& viewtwist, ads_real& frontz, ads_real& backz)
{
  struct resbuf rb;
  struct resbuf wcs, dcs, ccs; // acedTrans coord system flags
  ads_point vpDir;
  ads_point ptmax;
  int /*viewmode,*/tilemode, cvport; // Get window to zoom to:
  wcs.restype = RTSHORT; // WORLD coord system flag
  wcs.resval.rint = 0;
  ccs.restype = RTSHORT; // CURRENT coord system flag
  ccs.resval.rint = 1;
  dcs.restype = RTSHORT; // DEVICE coord system flag
  dcs.resval.rint = 2; // Get the VPOINT' direction vector

  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, selPt);

  acedGetVar(_T("VIEWDIR"), &rb);
  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, vpDir);

  viewDir.set(vpDir[X], vpDir[Y], vpDir[Z]);
  acedTrans(selPt, &ccs, &dcs, 0, ptmax);

  // set center point of view
  cenPt[X] = ptmax[X];
  cenPt[Y] = ptmax[Y];

  // get and set other properties
  acedGetVar(_T("LENSLENGTH"), &rb);
  lenslength = rb.resval.rreal;

  acedGetVar(_T("VIEWTWIST"), &rb);
  viewtwist = rb.resval.rreal;

  acedGetVar(_T("FRONTZ"), &rb);
  frontz = rb.resval.rreal;

  acedGetVar(_T("BACKZ"), &rb);
  backz = rb.resval.rreal;

  acedGetVar(_T("TILEMODE"), &rb);
  tilemode = rb.resval.rint;

  acedGetVar(_T("CVPORT"), &rb);
  cvport = rb.resval.rint;

  // Paperspace flag
  paperspace = ((tilemode == 0) && (cvport == 1)) ? Adesk::kTrue : Adesk::kFalse;

  acedGetVar(_T("TARGET"), &rb);
  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, target);

  acedGetVar(_T("viewsize"),&rb); 
  vHeight = rb.resval.rreal; 
  acedGetVar(_T("screensize"),&rb); 
  vWidth = rb.resval.rpoint[Y];
  vScale = vHeight/vWidth; 
  vWidth = rb.resval.rpoint[X]*vScale;
}


void rx_SetCurViewPortData(AcDbDatabase *pDb,
                           AcGeVector3d& viewDir, AcGePoint2d& cenPt, ads_real& vScale,
                           ads_real vHeight, ads_real vWidth,
                           ads_point target,
                           bool paperspace,
                           ads_real lenslength, ads_real viewtwist, ads_real frontz, ads_real backz)
{
  if (!pDb->tilemode())
  {
    acutPrintf(_T("\nThis command is only for Model space (i.e tilemode)\n"));
    return;
  }

  AcDbViewTableRecord view;

  view.setCenterPoint(cenPt);
  view.setHeight(vHeight);
  view.setWidth(vWidth);
  view.setViewDirection(viewDir);

  view.setLensLength(lenslength);
  view.setViewTwist(viewtwist);
  view.setPerspectiveEnabled(false);
  view.setFrontClipEnabled(false);
  view.setBackClipEnabled(false);
  view.setFrontClipAtEye(false);

  view.setIsPaperspaceView(paperspace);
  if (Adesk::kFalse == paperspace) {
    view.setFrontClipDistance(frontz);
    view.setBackClipDistance(backz);
  }
  else{
    view.setFrontClipDistance(0.0);
    view.setBackClipDistance(0.0);
  }

  view.setTarget(AcGePoint3d(target[X], target[Y], target[Z]));
  acedSetCurrentView(&view, NULL);
}

double rx_GetViewScale() 
{ 
  resbuf viewsize;   acedGetVar(_T("viewsize"),&viewsize); 
  resbuf screensize; acedGetVar(_T("screensize"),&screensize); 

  double height = viewsize.resval.rreal; 
  double screenheight = screensize.resval.rpoint[Y]; 

  return height/screenheight; 
}

void rx_zoomExtentsXY()
{
  AcDbViewTableRecord view; 
  AcGePoint3d max = acdbHostApplicationServices()->workingDatabase()->extmax(), 
              min = acdbHostApplicationServices()->workingDatabase()->extmin(); 

  AcGePoint2d max_2d (max.x, max.y); 
  AcGePoint2d min_2d (min.x, min.y); 
  // now set the view centre point 
  view.setCenterPoint (min_2d + (max_2d - min_2d) / 2.0); 
  // now height and width of view 
  view.setWidth (max_2d.x - min_2d.x); 
  view.setHeight(max_2d.y - min_2d.y); 
  // set the view 
  acedSetCurrentView (&view, NULL); 
  // updates the extents 
  WORKDWG->updateExt(TRUE);
}

void rx_zoomExtents(AcDbViewport *vp)
{
  AcDbDatabase *db = vp->database();
  // get the screen aspect ratio to calculate the height and width 
  double mScrRatio = vp->width()/vp->height();
  db->updateExt(Adesk::kTrue);
  AcGePoint3d mMaxExt = db->extmax();
  AcGePoint3d mMinExt = db->extmin();
  AcDbExtents mExtents(mMinExt, mMaxExt);
  // prepare Matrix for DCS to WCS transformation 

  struct resbuf rbFrom, rbTo;
  rbFrom.restype = RTSHORT;
  rbFrom.resval.rint = 2; // From DCS
  rbTo.restype = RTSHORT;
  rbTo.resval.rint = 0; // To WCS

  // Translate the X-axis of the DCS to WCS co-ordinates (as a displacement vector)

  //AcGeVector3d ptDir;
  //acedTrans( asDblArray( AcGeVector3d::kXAxis ), &rbFrom, &rbTo, 1, asDblArray( ptDir ));

  AcGeMatrix3d matWCS2DCS = AcGeMatrix3d::planeToWorld(vp->viewDirection());
  matWCS2DCS *= AcGeMatrix3d(vp->viewTarget() - AcGePoint3d::kOrigin);
  matWCS2DCS *= AcGeMatrix3d::rotation(-vp->twistAngle(), vp->viewDirection(), vp->viewTarget());
  matWCS2DCS = matWCS2DCS.inverse();
  // tranform the extents to the DCS defined by the viewdir 
  mExtents.transformBy(matWCS2DCS);
  // width of the extents in current view 
  double mWidth = mExtents.maxPoint().x - mExtents.minPoint().y;
  // height of the extents in current view 
  double mHeight = mExtents.maxPoint().y - mExtents.minPoint().y;
  //get the view center point 
  AcGePoint2d mCentPt( (mExtents.maxPoint().x + mExtents.minPoint().x) * 0.5,
    (mExtents.maxPoint().y + mExtents.minPoint().y) * 0.5);
  // check if the width 'fits' in current window, if not then get the new height as per the viewports aspect ratio 
  if (mWidth > (mHeight * mScrRatio))
    mHeight = mWidth / mScrRatio;
  // set the view height - adjusted by 1% 
  vp->setViewHeight(mHeight * 1.01);
  //set the view center 
  vp->setViewCenter(mCentPt);
  //vp->visible = True
  vp->setOn();
  vp->updateDisplay();
}


bool gswrActxUpdateView()
{
  //bGswrIsUpdate = true;
  CComPtr <IAcadApplication> pAcad = static_cast<IAcadApplication *>(acedGetAcadWinApp()->GetIDispatch(TRUE));
  if (pAcad == NULL) return false;
  HRESULT hr = pAcad->Update();
  //bGswrIsUpdate = false;
  if (SUCCEEDED(hr))
    return (true);
  else
    return (false);
}

bool gswrActxRegenModel()
{
  CComPtr <IAcadApplication> pAcad = static_cast<IAcadApplication *>(acedGetAcadWinApp()->GetIDispatch(TRUE));
  if (pAcad == NULL) return false;
  CComPtr <IAcadDocument> pDoc = NULL;
  pAcad->get_ActiveDocument(&pDoc);
  if (pDoc == NULL) return false;
  HRESULT hr = pDoc->Regen(acActiveViewport);
  if (SUCCEEDED(hr))
    return (true);
  else
    return (false);
}