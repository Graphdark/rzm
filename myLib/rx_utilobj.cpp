// UTILOBJ.CPP 
#include "stdafx.h"

#include "rx_util.h"

#include "adscodes.h"

#include "math.h"
#include "dbxutil.h"
#include "geassign.h"

#ifndef _LCT
#define _LCT _T
#endif


// Return integer part of number.  Truncates towards 0 
double 
aint(double x)
{
    // Note that this routine cannot handle the full floating point
    // number range.  This function should be in the machine-dependent
    // floating point library!
    long l = long(x);
    if ((int) (-0.5) != 0 && l < 0)
        l++;
    x = l;
    return x;
}


// Range reduce angle to 0 <= angle < 2 * PI 

double 
rx_fixangle(double angle)
{
    return angle - (2 * PI) * aint((angle / (2 * PI)) - (angle < 0.0));
}

void 
rx_fixindex(int& index, int maxIndex)
{
    if (index > maxIndex)
	index -= maxIndex;

    if (index < 0)
	index += maxIndex;
}

double rx_roundByStep ( double value, double step, double minToMin /*= 0.1*/ )
{
  step = fabs(step);
  double tt = fmod( fabs(value), step);
  tt = (tt > minToMin) ? step - tt : 0;
  if (value >= 0) value += tt;
  else value -= tt;
  return value;
}

AcGePoint3d rx_roundByStep ( const AcGePoint3d &pt, double step, double minToMin /*= 0.1*/ )
{
  AcGePoint3d ptN;
  ptN.x = rx_roundByStep(pt.x, step, minToMin);  
  ptN.y = rx_roundByStep(pt.y, step, minToMin);  
  ptN.z = rx_roundByStep(pt.z, step, minToMin);  
  return ptN;
}

double rx_round( double val, int digits /*= 2*/)
{
  if (digits < 0) digits = 0;
  if (digits > 8) digits = 8;
  double power = pow(10.0, digits);
  return floor((val * power)+0.5) / power;
}

AcGePoint3d rx_round(const AcGePoint3d &pt, int numdecimalplaces /*= 2*/)
{
  AcGePoint3d ptN;
  ptN.x = rx_round(pt.x, numdecimalplaces);  
  ptN.y = rx_round(pt.y, numdecimalplaces);  
  ptN.z = rx_round(pt.z, numdecimalplaces);  
  return ptN;
}

AcGeVector3d rx_round(const AcGeVector3d &v, int numdecimalplaces /*= 2*/)
{
  AcGeVector3d vN;
  vN.x = rx_round(v.x, numdecimalplaces);  
  vN.y = rx_round(v.y, numdecimalplaces);  
  vN.z = rx_round(v.z, numdecimalplaces);  
  return vN;
}

double rx_roundAngleByAxisXY(double ang)
{
  ang = rx_fixangle(ang);
  if (PI/4. < ang && ang <= 3*PI/4 )
    return PI/2;
  else
  if (3*PI/4. < ang && ang <= 5*PI/4 )
    return PI;
  else
  if (5*PI/4. < ang && ang <= 7*PI/4 )
    return 3*PI/2;
  else
    return 0;
}

AcGeVector3d rx_roundAxisXY(double ang)
{
  ang = rx_fixangle(ang);
  AcGeVector3d vec;
  if (PI/4. < ang && ang <= 3*PI/4 )
    vec = AcGeVector3d::kYAxis;
  else
  if (3*PI/4. < ang && ang <= 5*PI/4 )
    vec = -AcGeVector3d::kXAxis;
  else
  if (5*PI/4. < ang && ang <= 7*PI/4 )
    vec = -AcGeVector3d::kYAxis;
  else
    vec = AcGeVector3d::kXAxis;
  return vec;
}

AcGeVector3d rx_roundAxisXY(const AcGeVector3d &v)
{
  return rx_roundAxisXY(v.angleTo(AcGeVector3d::kXAxis, -AcGeVector3d::kZAxis));
}

//*************************************************************************
// Утилиты для работы с RB-структурами
//*************************************************************************

struct resbuf* rx_nthRb (int pos, struct resbuf* pRb, struct resbuf* &resRb)
{
  resRb = pRb;
  for (int i = 0; resRb != NULL && i < pos; i++, resRb = resRb->rbnext);
  return resRb;
}

struct resbuf* rx_assocRb(struct resbuf *rchain, int gcode) 
{ 
    while ((rchain != NULL) && (rchain->restype != gcode)) 
        rchain = rchain->rbnext; 
    return rchain; 
} 

//*************************************************************************
// Database related utility routines 
//*************************************************************************


Acad::ErrorStatus 
rx_scanPline(AcDb2dPolyline*        pline,
             AcGePoint3dArray&      points,
             AcGeDoubleArray&       bulges)
{
    Acad::ErrorStatus es = Acad::eOk;
    AcDb2dVertex*  vert   = NULL;
    AcDbObjectId   vId;
    AcDbObjectIterator *vIter;
    vIter = pline->vertexIterator();

    for (; !vIter->done(); vIter->step()) {
	  vId = vIter->objectId();
    pline->openVertex(vert, vId, AcDb::kForRead);
	  points.append(vert->position());
	  bulges.append(vert->bulge());
    }
    delete vIter;
    return es;
}

Acad::ErrorStatus 
rx_scanPline(AcDb3dPolyline* pline, AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcDb3dPolylineVertex*  vert   = NULL;
    AcDbObjectId           vId;

    AcDbObjectIterator *vIter;
    vIter = pline->vertexIterator();

    for (; !vIter->done(); vIter->step()) {
	vId = vIter->objectId();

	//AOK(pline->openVertex(vert, vId, AcDb::kForRead));
    pline->openVertex(vert, vId, AcDb::kForRead);

	points.append(vert->position());
    }

    delete vIter;
    
    return es;
}

Acad::ErrorStatus rx_setVertexPline(AcDbPolyline *pPLine, const AcGePoint3dArray &ptAr, bool bClosed)
{
  if (ptAr.length() < 2 || ptAr[0] == ptAr[1])
    return Acad::eInvalidInput;

  Acad::ErrorStatus es = Acad::eOk;

  UINT nVerts = pPLine->numVerts();
  if (nVerts > (UINT)ptAr.length())
  {
    while (nVerts > (UINT)ptAr.length())
    {
      es = pPLine->removeVertexAt(--nVerts);
      ASSERT(nVerts == pPLine->numVerts());
      if (nVerts != pPLine->numVerts())
        return es;
    }
  }

  for (UINT i = 0; i < (UINT)ptAr.length(); i++)
  {
    if (i < nVerts)
      es = pPLine->setPointAt(i, asPnt2d(asDblArray(ptAr[i])));
    else
      es = pPLine->addVertexAt(i, asPnt2d(asDblArray(ptAr[i])));
    if (es != Acad::eOk)
      break;
  }

#ifdef _DEBUG
  nVerts = pPLine->numVerts();
  ASSERT(pPLine->numVerts() == ptAr.length());
  ASSERT(nVerts > 1);
#endif
  if (intToBool(pPLine->isClosed()) != bClosed)
    pPLine->setClosed(bClosed);

  return es;
}


Acad::ErrorStatus 
rx_makeArc(const AcGePoint3d    pt1, 
           const AcGePoint3d    pt2, 
                 double         bulge,
           const AcGeVector3d   entNorm,
                 AcGeCircArc3d& arc)
{
    Acad::ErrorStatus es = Acad::eOk;

    // The points that are coming in are in ECS. These are actually
    // 2d points, may be with an elevation in the z coord.
    //
    // Therefore, let's create a 2d arc from these values and transform
    // the relevant data of the arc for creating a 3d arc.

    AcGeCircArc2d arc2d;
    AcGePoint2d p1, p2;

    RXASSERT(fabs(pt1[Z] - pt2[Z]) < 1.0e-10);

    p1.set(pt1[X], pt1[Y]); p2.set(pt2[X], pt2[Y]);
    arc2d.set(p1, p2, bulge);

    AcGePoint3d center((arc2d.center())[X], (arc2d.center())[Y], pt1[Z]);
    AcGePoint3d startPnt((arc2d.startPoint())[X], 
                         (arc2d.startPoint())[Y], pt1[Z]);
    AcGePoint3d endPnt((arc2d.endPoint())[X], (arc2d.endPoint())[Y], pt1[Z]);

    // If the arc is CW, flip the normal.

    AcGeVector3d norm;

    if (arc2d.startAng() > arc2d.endAng()) {
	norm.set(0, 0, -1);
    } else {
	norm.set(0, 0, 1);
    }

    double incAng = fabs(arc2d.endAng() - arc2d.startAng());

    // Transform all the data to WCS.

    acdbEcs2Wcs(asDblArray(center), asDblArray(center), asDblArray(entNorm), 
							    Adesk::kFalse);
    acdbEcs2Wcs(asDblArray(startPnt), asDblArray(startPnt), asDblArray(entNorm), 
							    Adesk::kFalse);
    acdbEcs2Wcs(asDblArray(endPnt), asDblArray(endPnt), asDblArray(entNorm), 
							    Adesk::kFalse);
    acdbEcs2Wcs(asDblArray(norm), asDblArray(norm), asDblArray(entNorm), 
							    Adesk::kTrue);

    arc.set(center, norm, norm.perpVector(),
	(startPnt - center).length(), 0, incAng);

    return es;
}


void rx_collectRemoveNoReadyIds(AcDbObjectIdArray& objIdAr)
{
  AcDbEntity* pEnt;
  for (int i = 0; i < objIdAr.length(); i++)
  {
    if(acdbOpenObject(pEnt, objIdAr[i], AcDb::kForRead) == Acad::eOk)
      pEnt->close();
    else
      objIdAr.removeAt(i--);
  }
}

void rx_collectRemoveNoCastIds(AcDbObjectIdArray& objIdAr, AcRxClass* rxClass, bool bEqClass)
{
  AcDbEntity* pEnt;
  for (int i = 0; i < objIdAr.length(); i++)
  {
    ARXOK(acdbOpenObject(pEnt, objIdAr[i], AcDb::kForRead));
    if (bEqClass)
    {
      if (pEnt->desc() != rxClass)
        objIdAr.removeAt(i--);
    }
    else
    {
      if ( !pEnt->isKindOf(rxClass) )
        objIdAr.removeAt(i--);
    }
    pEnt->close();
  }
}


// Get an AcGiTextStyle from an AcDbTextStyleTableRecord.
// Try to map as many characteristics as we can.
//
Acad::ErrorStatus
rx_getTextStyle(AcGiTextStyle &newStyle, const AcDbObjectId& styleId)
{
    AcDbTextStyleTableRecord *oldStyle;
    Acad::ErrorStatus es = acdbOpenAcDbObject((AcDbObject *&)oldStyle,
                             styleId, AcDb::kForRead);
    if (es == Acad::eOk) {
        LPCTSTR tmpStr;
        if ((es = oldStyle->fileName(tmpStr)) != Acad::eOk) {
            oldStyle->close();
            return es;
        }
        newStyle.setFileName(tmpStr);

        if ((es = oldStyle->bigFontFileName(tmpStr)) != Acad::eOk) {
            oldStyle->close();
            return es;
        }
        newStyle.setBigFontFileName(tmpStr);

        newStyle.setTextSize(oldStyle->textSize());
        newStyle.setXScale(oldStyle->xScale());
        newStyle.setObliquingAngle(oldStyle->obliquingAngle());

        oldStyle->close();
        newStyle.loadStyleRec();
    }
    return es;
}

// Given the name of a text style, look up that name and
// return the db id of the TextStyleTableRecord.
//
Acad::ErrorStatus 
rx_getTextStyleId(LPCTSTR styleName, AcDbObjectId &styleId, AcDbDatabase *db /*NULL*/)
{
  	if(db == NULL) db = acdbHostApplicationServices()->workingDatabase();
	  if(db == NULL) acrx_abort(_T("!%s@%d: %s"), _T(__FILE__), __LINE__, _T("No database!"));

    AcDbTextStyleTable *pStyleTable;
    Acad::ErrorStatus es = db->getSymbolTable(pStyleTable, AcDb::kForRead);
    if (es == Acad::eOk) 
    {
        es = pStyleTable->getAt(styleName, styleId, Adesk::kFalse);
        pStyleTable->close();
    }
    return es;
}

Acad::ErrorStatus 
rx_getTextStyleName(const AcDbObjectId& styleId, LPCTSTR styleName)
{
	if(AcDbObjectId::kNull == styleId)
		return Acad::eNullObjectId;

  Acad::ErrorStatus es;
	AcDbObject *pObj;
  es = acdbOpenAcDbObject(pObj, styleId, AcDb::kForRead);
  if (es == Acad::eOk )
  {
	  AcDbTextStyleTableRecord *pStyle = AcDbTextStyleTableRecord::cast(pObj);
	  if(pStyle == NULL)
	  {
		  pObj->close();
		  return Acad::eWrongObjectType;
	  }
	  pStyle->getName(styleName);
	  pStyle->close();
  }
	return es;
}


Acad::ErrorStatus
rx_getLayerId(LPCTSTR pStr, AcDbObjectId& layerId, bool getErasedRecord /*= false*/, AcDbDatabase* db /*=NULL*/)
{
  if(db == NULL) db = acdbHostApplicationServices()->workingDatabase();
  if(db == NULL) acrx_abort(_T("!%s@%d: %s"), _T(__FILE__), __LINE__, _T("No database!"));
  
  layerId = db->layerZero();

  AcDbLayerTable* pLayerTable;
  Acad::ErrorStatus err = db->getLayerTable(pLayerTable, AcDb::kForRead);
  if (err == Acad::eOk)
  {
    // Get the ID of the layer with the name in 'pStr' (even if it's erased) and place it in 'id'.  
    err = pLayerTable->getAt(pStr, layerId, getErasedRecord);
    pLayerTable->close();
  }
  return err;
}


bool
rx_isAvailableLayer(LPCTSTR  pStr, bool mess /*=true*/, AcDbDatabase* db /*=NULL*/)
{
  AcDbObjectId layerId;
  if (rx_getLayerId(pStr, layerId, false, db) != Acad::eOk) 
  {
    if (mess)
      acutPrintf(_LCT("Слой %s не обнаружен."), pStr);
    return false;
  }
  return rx_isAvailableLayer(layerId, mess);
}

bool
rx_isAvailableLayer(const AcDbObjectId& layerId, bool mess /*=true*/)
{
  bool ret = false;

  AcDbObject *pObj;
  LPCTSTR pStr = NULL;
  Acad::ErrorStatus es = acdbOpenObject(pObj, layerId, AcDb::kForRead);
  if (es == Acad::eOk )
  {
    AcDbLayerTableRecord *pLayer = AcDbLayerTableRecord::cast(pObj);
	  if(pLayer == NULL)
	  {
		  pObj->close();
      acrx_abort(_T("!%s@%d: %s"), _T(__FILE__), __LINE__, _T("не AcDbLayerTableRecord!"));
		  return ret;
	  }
    pLayer->getName(pStr);
    ret = !(pLayer->isLocked() || pLayer->isFrozen());
		pLayer->close();
  }
  if ( !ret && mess && pStr )
    acutPrintf(_LCT("Слой %s недоступен."), pStr);

  return ret;
}

bool
rx_isOffLayer(const AcDbObjectId& layerId)
{
  bool ret = false;
  AcDbObject *pObj;
  if (acdbOpenAcDbObject(pObj, layerId, AcDb::kForRead) == Acad::eOk)
  {
    AcDbLayerTableRecord *pLayer = AcDbLayerTableRecord::cast(pObj);
    if(pLayer == NULL)
    {
      pObj->close();
      acrx_abort(_T("!%s@%d: %s"), _T(__FILE__), __LINE__, _T("не AcDbLayerTableRecord!"));
      return ret;
    }
    ret = pLayer->isOff();
    pLayer->close();
  }
  return ret;
}


bool
rx_isOffLayer(LPCTSTR  pStr, bool mess /*=true*/, AcDbDatabase* db /*=NULL*/)
{
  bool ret = false;
  AcDbObjectId layerId;
  if (rx_getLayerId(pStr, layerId, false, db) != Acad::eOk) 
    return ret;
  ret = rx_isOffLayer(layerId);
  if ( ret && mess )
    acutPrintf(_LCT("Слой %s выключен."), pStr);
  return ret;
}


Acad::ErrorStatus
rx_setOffLayer(const AcDbObjectId& layerId, bool bOff)
{
  AcDbObject *pObj;
  Acad::ErrorStatus es = acdbOpenAcDbObject(pObj, layerId, AcDb::kForWrite);
  if ( es == Acad::eOk)
  {
    AcDbLayerTableRecord *pLayer = AcDbLayerTableRecord::cast(pObj);
    if(pLayer == NULL)
      es = Acad::eWrongObjectType;
    else
      pLayer->setIsOff(bOff);
    pObj->close();
  }
  return es;
}

Acad::ErrorStatus
rx_setOffLayer(LPCTSTR  pStr, bool bOff, AcDbDatabase* db /*=NULL*/)
{
  AcDbObjectId layerId;
  Acad::ErrorStatus es = rx_getLayerId(pStr, layerId, false, db);
  if ( es == Acad::eOk) 
    es = rx_setOffLayer(layerId, bOff);
  return es;
}


Acad::ErrorStatus
rx_setOffLayers(bool bOff, int layerCount, ...)
{
  Acad::ErrorStatus esT, es = Acad::eOk;
  va_list vl;
  int i;
  //  layerCount is the last argument specified; all
  //  others must be accessed using the variable-
  //  argument macros.
  va_start( vl, layerCount );
  // Step through the list.
  for( i = 0; i < layerCount; ++i )
  {
     LPCTSTR s = va_arg( vl, LPTSTR );
     esT = rx_setOffLayer(s, bOff);
     if (esT != Acad::eOk)
       es = esT;
  }
  va_end( vl );
  return es;
}


Acad::ErrorStatus
getUniformKnots(int numCtrlPts, int degree, int form, AcGeDoubleArray& knots)
{
    Acad::ErrorStatus ecode = Acad::eOk;
    int i, nspans;

    nspans = numCtrlPts - degree; 

    knots.setLogicalLength(nspans + (2*degree) + 1);

    double dt = 1.0; // (tn - t0) / n. Usually tn = n and t0 = 0

    for (i = 0; i < degree; i++) {
        if (form == 2) {
            knots[i] = -(degree - i) * dt;
        } else { 
            knots[i] = 0.0;
	}
    }

    knots[degree] = 0.0;
    for (i = degree + 1; i < nspans + degree + 1; i++) {
        knots[i] = knots[i - 1] + dt; 
    }

    for (i = 1; i <= degree; i++) {
        if (form == 2) {
            knots[nspans + degree + i] = 
		knots[nspans + degree] + i * dt;
        } else {
            knots[nspans + degree + i] = knots[nspans + degree]; 
	}
    }

    return ecode;
}


void rx_drawLine(AcGiGeometry& g, const AcGePoint3d& p1, const AcGePoint3d& p2)
{
  AcGePoint3d pts[2];
  pts[0] = p1;
  pts[1] = p2;
  g.polyline(2, pts);
}

void rx_drawRect(AcGiGeometry& g, const AcGePoint3d& p1, const AcGePoint3d& p2)
{
  AcGePoint3d pts[5];
  pts[0] = p1;  
  pts[1].set( p1[X], p2[Y], p1[Z]);
  pts[2] = p2;
  pts[3].set( p2[X], p1[Y], p1[Z]);
  pts[4] = p1;  
  g.polyline(5, pts);
}

void rx_drawX(AcGiGeometry& g, const AcGePoint3d& pt, double r, bool bAsPlus,  bool b3d)
{
  AcGePoint3d pts[2];
  if (bAsPlus)
  {
    pts[0] = pt; pts[0].x += r;
    pts[1] = pt; pts[1].x -= r;
    g.polyline(2, pts);
    pts[0] = pt; pts[0].y += r;
    pts[1] = pt; pts[1].y -= r;
    g.polyline(2, pts);
  }
  else
  {
    pts[0] = pt; pts[0].x += r; pts[0].y += r;
    pts[1] = pt; pts[1].x -= r; pts[1].y -= r;
    g.polyline(2, pts);
    pts[0] = pt; pts[0].x += r; pts[0].y -= r;
    pts[1] = pt; pts[1].x -= r; pts[1].y += r;
    g.polyline(2, pts);
  }
  if (!b3d) return;
  pts[0] = pt; pts[0].z += r;
  pts[1] = pt; pts[1].z -= r;
  g.polyline(2, pts);
}

void rx_drawBox(AcGiGeometry& g, const AcGePoint3d* pVerts, const AcGeVector3d* pViewDir /*=NULL*/)
{
      //   3 ------ 2
      //   |  7 ------ 6
      //   0--|-----1  |
      //      4 ------ 5  

  if (pVerts[0] == pVerts[1] || (pViewDir && pViewDir->isParallelTo(AcGeVector3d(pVerts[0]-pVerts[1]))))
  {
    AcGePoint3d ptAr[5];
    ptAr[0] = pVerts[0];
    ptAr[1] = pVerts[3];
    ptAr[2] = pVerts[7];
    ptAr[3] = pVerts[4];
    ptAr[4] = ptAr[0];
    g.polyline(5, ptAr);
  }
  else
  if (pVerts[0] == pVerts[3] || (pViewDir && pViewDir->isParallelTo(AcGeVector3d(pVerts[0]-pVerts[3]))))
  {
    AcGePoint3d ptAr[5];
    ptAr[0] = pVerts[0];
    ptAr[1] = pVerts[1];
    ptAr[2] = pVerts[5];
    ptAr[3] = pVerts[4];
    ptAr[4] = ptAr[0];
    g.polyline(5, ptAr);
  }
  else
  if (pVerts[0] == pVerts[4] || (pViewDir && pViewDir->isParallelTo(AcGeVector3d(pVerts[0]-pVerts[4]))))
  {
    AcGePoint3d ptAr[5];
    ptAr[0] = pVerts[0];
    ptAr[1] = pVerts[1];
    ptAr[2] = pVerts[2];
    ptAr[3] = pVerts[3];
    ptAr[4] = ptAr[0];
    g.polyline(5, ptAr);
  }
  else
  {
    for (int i=0; i<7; i++)
      if (pVerts[i] != pVerts[i+1])
        g.polyline(2, pVerts+i);
  }
}

AcGePoint3d rx_pt2Dto3D (const AcGePoint2d& p)
{
  AcGePoint3d pt;
  pt[0] = p.x;   pt[1] = p.y;   pt[2] = 0.0;
  return pt;
}

AcGePoint2d rx_pt3Dto2D (const AcGePoint3d& p)
{
  AcGePoint2d pt;
  pt[0] = p.x;   pt[1] = p.y;
  return pt;
}
//-----------------------------------------------------------------------------
// Turn the given vector into a unit vector with the same direction.
//
bool rx_univec(AcGeVector3d& ap, AcGeVector3d& bp)
{
    double d;

    if ((d = (bp[X] * bp[X] + bp[Y] * bp[Y] + bp[Z] * bp[Z])) <= (ZERO * ZERO))
        return FALSE;
    d = 1.0 / sqrt(d);
    ap[X] = bp[X] * d;
    ap[Y] = bp[Y] * d;
    ap[Z] = bp[Z] * d;
    return TRUE;
}


bool rx_textBox ( LPCTSTR  pStr,
                  const AcGiTextStyle &style, 
                  AcGePoint3d& ptMin,
                  AcGePoint3d& ptMax,
                  AcGiWorldDraw* ctxt /*= NULL*/)
{
  AcGePoint2d ext = style.extents(pStr, Adesk::kFalse, (const int)_tcslen(pStr), Adesk::kFalse, ctxt);
  ptMin.set(0.0, 0.0, 0.0);
  ptMax.set(ext.x, ext.y, 0.0);
  return true;
}

bool rx_textBox ( AcDbText* pText,
                  AcGePoint2d& ext,
                  AcGiWorldDraw* ctxt /*= NULL*/)
{
  LPTSTR pStr = (LPTSTR)pText->textString();
  AcGiTextStyle style;
  if (fromAcDbTextStyle(style, pText->textStyle()) == Acad::eOk)
  {
    ext = style.extents(pStr, Adesk::kFalse, (const int)_tcslen(pStr), Adesk::kFalse, ctxt);
    ext.x *= pText->widthFactor();

    free(pStr);
    return true;
  }
  free(pStr);
  return false;
}

void rx_boxPoints(const AcGePoint3d& pt1, const AcGePoint3d& pt2, 
                  AcGePoint3dArray& ptArray, double dH, double dL)
{
  ptArray.setLogicalLength(4);
  double len = pt1.distanceTo(pt2);
  ptArray[0].set(pt1.x - dL, pt1.y - dH, pt1.z);
  ptArray[1].set(pt1.x - dL, pt1.y + dH, pt1.z);
  ptArray[2].set(pt1.x + len + dL, pt1.y + dH, pt1.z);
  ptArray[3].set(pt1.x + len + dL, pt1.y - dH, pt1.z);
  if (len > 1.0)
  {
    double ang = (pt2 - pt1).angleTo(AcGeVector3d::kXAxis);
    for (int i = 0; i < 4; i++ )
      ptArray[i].rotateBy ( (pt1.y < pt2.y) ? ang : -ang , AcGeVector3d::kZAxis, pt1 );
  }
}


// calculate size of a marker by using PDSIZE variable
// which is how AcDbPoint entities draw in AutoCAD.
double rx_pdSize(AcDbDatabase* pDb, double percentageOfViewsize /*0.1*/)
{
  if (!pDb)
    pDb = WORKDWG;
  double pdsize = pDb->pdsize();
  if (pdsize <= 0.0) {			// values greater than 0.0 are already in absolute size
    double viewsize = 100.0;	// arbitrary hard default
    //viewsize = getSysVar("viewsize", viewsize);
    viewsize /= 2.0;     // ?? doesn't match AutoCAD docs, but is correct value
    if (pdsize < 0.0)    // percentage of viewsize
      pdsize  = viewsize * (fabs(pdsize) / 100.0);
    else
      pdsize = viewsize * 0.1; // 10% of viewsize
  }
  return pdsize;
}
