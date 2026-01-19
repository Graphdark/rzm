#include "stdafx.h"

#include "rx_util.h"

#ifdef RZMLIB
//#include "ArxDbgUtilsGe.h"
#include "..\Wiring_1\Wiring_DBX\AcDbWBox.h"
#include "..\Wiring_1\Wiring_DBX\AcDbWWire.h"
#endif

bool rx_pointNearEntities(const AcDbObjectIdArray& objIdAr, AcGePoint3d& pt)
{
  bool bFindNear = false;
  AcGePoint3d ptNearTmp, ptNear = pt;
  double d, dMin = -1;
  for (int i = 0; i < objIdAr.length(); i++)
  {
    AcDbEntity* pEnt;
    if (acdbOpenObject(pEnt, objIdAr[i], AcDb::kForRead) == Acad::eOk)
    {
      if (pEnt->isKindOf(AcDbCurve::desc()))
      {
        if (AcDbCurve::cast(pEnt)->getClosestPointTo(pt, ptNearTmp) == Acad::eOk)
        {
          d = pt.distanceTo(ptNearTmp);
          if (dMin < 0 || d < dMin)
          {
            dMin = d;
            ptNear = ptNearTmp;
            bFindNear = true;
          }
        }
      }
#ifdef RZMLIB
      else
        if (pEnt->isKindOf(AcDbWBox::desc()) && (((AcDbWBox*)pEnt)->showFlags() & AcDbWBox::kShowBox) > 0)
        {
          ptNearTmp = ((AcDbWBox*)pEnt)->closestPointTo(pt);
          d = pt.distanceTo(ptNearTmp);
          if (dMin < 0 || d < dMin)
          {
            dMin = d;
            ptNear = ptNearTmp;
            bFindNear = true;
          }
        }
        else
          if (pEnt->isKindOf(AcDbWWire::desc()))
          {
            ptNearTmp = ((AcDbWWire*)pEnt)->closestPointTo(pt);
            d = pt.distanceTo(ptNearTmp);
            if (dMin < 0 || d < dMin)
            {
              dMin = d;
              ptNear = ptNearTmp;
              bFindNear = true;
            }
          }
#endif
      pEnt->close();
    }
  }
  if (bFindNear)
    pt = ptNear;
  return bFindNear;
}

bool rx_pointNearEntities(const AcDbObjectId& objId, AcGePoint3d& pt)
{
  AcDbObjectIdArray objIdAr(1);
  objIdAr.setLogicalLength(1);
  objIdAr[0] = objId;
  return rx_pointNearEntities(objIdAr, pt);
}

Acad::ErrorStatus rx_getEntityCenter(const AcDbEntity* pEnt, AcGePoint3d& pt, double dMin)
{
  AcDbExtents ext;
  if (pEnt->getGeomExtents(ext) == Acad::eOk)
    if (ext.maxPoint() != ext.minPoint())
    {
      if (ext.minPoint().distanceTo(ext.maxPoint()) > dMin)
      {
        //pt = ArxDbgUtilsGe::midpoint(ext.minPoint(), ext.maxPoint());
        pt.x = (ext.minPoint().x + ext.maxPoint().x) / 2.0;
        pt.y = (ext.minPoint().y + ext.maxPoint().y) / 2.0;
        pt.z = (ext.minPoint().z + ext.maxPoint().z) / 2.0;
        return Acad::eOk;
      }
    }
  return Acad::eInvalidExtents;
}


bool rx_textPosByAlignmentPoint(AcDbText* pText)
{
  if (pText->verticalMode() == AcDb::kTextBase)
  {
    AcDb::TextHorzMode hMode = pText->horizontalMode();
    if (hMode == AcDb::kTextLeft || hMode == AcDb::kTextAlign || hMode == AcDb::kTextFit)
      return false;
  }
  return true;
}


Acad::ErrorStatus rx_getTextPosition(const AcDbEntity* pEnt, AcGePoint3d& pt)
{
  if (pEnt->isKindOf(AcDbText::desc()))
  {
    //AcDbText* pText = (AcDbText*)pEnt;
    //if (!pText->isDefaultAlignment())
    //   pText->adjustAlignment(pEnt->database());
      //pt = ((AcDbText*)pEnt)->isDefaultAlignment() ? ((AcDbText*)pEnt)->position() : ((AcDbText*)pEnt)->alignmentPoint();
    pt = rx_textPosByAlignmentPoint((AcDbText*)pEnt) ? ((AcDbText*)pEnt)->alignmentPoint() : ((AcDbText*)pEnt)->position();
    return Acad::eOk;
  }
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
  {
#if defined( _ACAD2000 )
    pt = ((AcDbMText*)pEnt)->isDefaultAlignment() ? ((AcDbMText*)pEnt)->location() : ((AcDbText*)pEnt)->alignmentPoint();
#else
    pt = ((AcDbMText*)pEnt)->location();
#endif //_ACAD2000
    return Acad::eOk;
  }
  return Acad::eNotThatKindOfClass;
}


Acad::ErrorStatus rx_setTextPosition(AcDbEntity* pEnt, const AcGePoint3d& pt)
{
  Acad::ErrorStatus es = Acad::eNotThatKindOfClass;
  if (pEnt->isKindOf(AcDbText::desc()))
  {
    //return ((AcDbText*)pEnt)->isDefaultAlignment() ? ((AcDbText*)pEnt)->setPosition(pt) : ((AcDbText*)pEnt)->setAlignmentPoint(pt);
    //return rx_textPosByAlignmentPoint((AcDbText*)pEnt) ? ((AcDbText*)pEnt)->setAlignmentPoint(pt) : ((AcDbText*)pEnt)->setPosition(pt);
    AcDbText* pText = (AcDbText*)pEnt;
    if (pText->verticalMode() == AcDb::kTextBase)
    {
      AcDb::TextHorzMode hMode = pText->horizontalMode();
      if (hMode == AcDb::kTextLeft || hMode == AcDb::kTextAlign || hMode == AcDb::kTextFit)
      {
        if (hMode == AcDb::kTextAlign || hMode == AcDb::kTextFit)
        {
          AcGeVector3d v = pt - pText->position();
          pText->setAlignmentPoint(pText->alignmentPoint() + v);
        }
        es = pText->setPosition(pt);
        if (es == Acad::eOk && pText->database())
          pText->adjustAlignment(pText->database());
        return es;
      }
    }
    es = pText->setAlignmentPoint(pt);
    if (es == Acad::eOk && pText->database())
      pText->adjustAlignment(pText->database());
  }
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
  {
#if defined( _ACAD2000 )
    es = ((AcDbMText*)pEnt)->isDefaultAlignment() ? ((AcDbMText*)pEnt)->setLocation(pt) : ((AcDbMText*)pEnt)->setAlignmentPoint(pt);
#else
    es = ((AcDbMText*)pEnt)->setLocation(pt);
#endif
  }
  return es;
}


Acad::ErrorStatus rx_getTextRotation(AcDbEntity* pEnt, double& rotation)
{
  if (pEnt->isKindOf(AcDbText::desc()))
    rotation = ((AcDbText*)pEnt)->rotation();
  else
    if (pEnt->isKindOf(AcDbMText::desc()))
      rotation = ((AcDbMText*)pEnt)->rotation();
    else
      return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_setTextRotation(AcDbEntity* pEnt, double rotation)
{
  if (pEnt->isKindOf(AcDbText::desc()))
    ((AcDbText*)pEnt)->setRotation(rotation);
  else
    if (pEnt->isKindOf(AcDbMText::desc()))
      ((AcDbMText*)pEnt)->setRotation(rotation);
    else
      return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_getTextHeight(AcDbEntity* pEnt, double& height)
{
  if (pEnt->isKindOf(AcDbText::desc()))
    height = ((AcDbText*)pEnt)->height();
  else
    if (pEnt->isKindOf(AcDbMText::desc()))
      height = ((AcDbMText*)pEnt)->textHeight();
    else
      return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_setTextHeight(AcDbEntity* pEnt, double height)
{
  if (pEnt->isKindOf(AcDbText::desc()))
    ((AcDbText*)pEnt)->setHeight(height);
  else
    if (pEnt->isKindOf(AcDbMText::desc()))
      ((AcDbMText*)pEnt)->setTextHeight(height);
    else
      return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_getEntityPosition(const AcDbEntity *pEnt, AcGePoint3d& pt)
{
  if (pEnt->isKindOf(AcDbCircle::desc()))
    pt = ((AcDbCircle*)pEnt)->center();
#ifdef RZMLIB
  else
  if (pEnt->isKindOf(AcDbWBox::desc()))
    pt = ((AcDbWBox*)pEnt)->posGrip();
#endif
  else
  if (pEnt->isKindOf(AcDbBlockReference::desc()))
    pt = ((AcDbBlockReference*)pEnt)->position();
  else
    return rx_getTextPosition(pEnt, pt);
  return Acad::eOk;
}

Acad::ErrorStatus rx_setEntityPosition(AcDbEntity* pEnt, const AcGePoint3d& pt)
{
  Acad::ErrorStatus es = Acad::eNotThatKindOfClass;
  if (pEnt->isKindOf(AcDbCircle::desc()))
    es = ((AcDbCircle*)pEnt)->setCenter(pt);
#ifdef RZMLIB
  else
  if (pEnt->isKindOf(AcDbWBox::desc()))
    es = ((AcDbWBox*)pEnt)->moveGrip(pt);
#endif
  else
  if (pEnt->isKindOf(AcDbBlockReference::desc()))
    es = ((AcDbBlockReference*)pEnt)->setPosition(pt);
  else
    es = rx_setTextPosition(pEnt, pt);
  return es;
}

Acad::ErrorStatus rx_getEntityRotation(const AcDbEntity* pEnt, double& ang)
{
#ifdef RZMLIB
  if (pEnt->isKindOf(AcDbWBox::desc()))
    ang = ((AcDbWBox*)pEnt)->rotation();
  else
#endif
  if (pEnt->isKindOf(AcDbBlockReference::desc()))
    ang = ((AcDbBlockReference*)pEnt)->rotation();
  else
  if (pEnt->isKindOf(AcDbText::desc()))
    ang = ((AcDbText*)pEnt)->rotation();
  else
  if (pEnt->isKindOf(AcDbMText::desc()))
    ang = ((AcDbMText*)pEnt)->rotation();
  else
    return  Acad::eNotThatKindOfClass;
  return Acad::eOk;
}

Acad::ErrorStatus rx_setEntityRotation(AcDbEntity *pEnt, const double& ang)
{
  Acad::ErrorStatus es = Acad::eOk;
  AcGePoint3d ptOld, ptNew;
  double angOld = 0.;
  es = rx_getEntityPosition(pEnt, ptOld);
  if (es != Acad::eOk) 
    return es;
  es = rx_getEntityRotation(pEnt, angOld);
  if (es == Acad::eOk)
  {
    //if (ArxDbgUtilsGe::fuzzyEqual(angOld, ang))
    if (fabs(angOld - ang) <= AcGeContext::gTol.equalPoint())
      return es;

    ptNew == ptOld;

#ifdef RZMLIB
    if (pEnt->isKindOf(AcDbWBox::desc()))
      es = ((AcDbWBox*)pEnt)->setRotation(ang);
    else
#endif
    if (pEnt->isKindOf(AcDbBlockReference::desc()))
      es = ((AcDbBlockReference*)pEnt)->setRotation(ang);
    else
    if (pEnt->isKindOf(AcDbText::desc()))
      es = ((AcDbText*)pEnt)->setRotation(ang);
    else
    if (pEnt->isKindOf(AcDbMText::desc()))
      es = ((AcDbMText*)pEnt)->setRotation(ang);
    else
      es = Acad::eNotThatKindOfClass;

    if (es == Acad::eOk && rx_getEntityPosition(pEnt, ptNew) == Acad::eOk && ptNew != ptOld)
      rx_setEntityPosition(pEnt, ptOld);
  }

  return es;
}

