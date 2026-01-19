#include "stdafx.h"
#include "ac_svg_generator_msxml.h"

#include "XmlNodeWrapper.h"

class CSVG
{
  CXmlDocumentWrapper m_xmlDoc;

  AcDbObjectIdArray m_lrIds, m_ltIds;

  void saveLinetypes(CXmlNodeWrapper& np)
  {
    CXmlNodeWrapper nlt = np.AppendChild(_T("linetypes"));

    for (int i = 0; i < m_ltIds.length(); i++)
    {
      AcDbLinetypeTableRecordPointer p(m_ltIds[i], AcDb::kForRead);
      LPCTSTR cs = NULL;
      p->getName(cs);

      //if (nodeLayers.FindNodeAValue(_T("LineType"), _T("Name"), cs))
      //  continue;

      CXmlNodeWrapper n = nlt.AppendChild(_T("linetype"));

      n.SetAValue(_T("Index"), i);
      n.SetAValue(_T("Name"), cs);
      cs = NULL;  p->comments(cs);
      n.SetAValue(_T("Caption"), cs);
    }
  }

  void saveTextStyles(CXmlNodeWrapper& np)
  {
    CXmlNodeWrapper n = np.AppendChild(_T("style"));
    n.SetText(_T("\
      .small {\
        font: italic 13px sans-serif;\
      }\
      .heavy {\
        font: bold 30px sans-serif;\
      }\
      "));
  }

  void saveAcDbEntity(CXmlNodeWrapper& n, AcDbEntity* pEnt)
  {
    //n.SetAValue(_T("id"), ++m_lastId);
    n.SetAValue(_T("l"), pEnt->layer());

    int foundAt = 0; m_ltIds.find(pEnt->linetypeId(), foundAt);
    n.SetAValue(_T("lt"), foundAt);

    foundAt = int(pEnt->lineWeight());
    if (foundAt < 0) foundAt = 0;
    n.SetAValue(_T("lw"), foundAt / 100.);

    CString str;

    AcCmColor color = pEnt->color();
    Adesk::UInt16 ACIindex = color.colorIndex();
    unsigned rgb = 0;

    enum CP { cpPrimitive, cpLayer, cpblock, cpSystem };

    int cp = 0; //  

    AcCmEntityColor::ColorMethod cMethod = color.colorMethod();
    switch (cMethod)
    {
    case AcCmEntityColor::kByACI:
      cp = cpPrimitive;
      rgb = acedGetRGB(ACIindex);
      //rgb = RGB((rgb & 0xffL), (rgb & 0xff00L) >> 8, rgb >> 16);
      break;
    case AcCmEntityColor::kByColor:
      cp = cpSystem;
      // now convert back to the original values, first 8 bits blue
      rgb = RGB(color.red(), color.green(), color.blue());
      break;
    case AcCmEntityColor::kByLayer:
      cp = cpLayer;
      if (pEnt->layerId().isValid())
      {
        AcDbLayerTableRecordPointer l;
        if (rxOpenObject(l, pEnt->layerId()) == Acad::eOk)
        {
          ACIindex = l->color().colorIndex();
          rgb = acedGetRGB(ACIindex);
        }
        rxCloseObject(l);
      }
      break;
    case AcCmEntityColor::kByBlock:
      cp = cpblock;
    default:
      break;
    }

    n.SetAValue(_T("cp"), cp);
    n.SetAValue(_T("ac"), ACIindex);
    str.Format(_T("%u"), rgb);
    n.SetAValue(_T("c"), str);
  }

  void saveAcDbLine(CXmlNodeWrapper& np, AcDbLine* pEnt)
  {
    CXmlNodeWrapper n = np.AppendChild(_T("line"));
    saveAcDbEntity(n, pEnt);
    n.SetAValue(_T("x1"), pEnt->startPoint().x);
    n.SetAValue(_T("y1"), pEnt->startPoint().y);
    n.SetAValue(_T("x2"), pEnt->endPoint().x);
    n.SetAValue(_T("y2"), pEnt->endPoint().y);
  }

  void saveAcDbPolyline(CXmlNodeWrapper& np, AcDbPolyline* pPLine)
  {
    CXmlNodeWrapper n = np.AppendChild(_T("polyline"));
    saveAcDbEntity(n, pPLine);
    //n.SetAValue(_T("cs"), pPLine->isClosed() == Adesk::kTrue);
    CString s, sa;
    for (unsigned int i = 0; i < pPLine->numVerts(); i++)
    {
      AcGePoint3d pt;
      pPLine->getPointAt(i, pt);
      sa.Format(_T("%d"), pt.x);
      s += sa;
      sa.Format(_T("%d"), pt.y);
      s += sa;
    }
    n.SetAValue(_T("points"), s);
  }

  void saveAcDbCircle(CXmlNodeWrapper& np, AcDbCircle* pCircle)
  {
    CXmlNodeWrapper n = CXmlNodeWrapper(np.AppendChild(_T("circle")));
    saveAcDbEntity(n, pCircle);

    n.SetAValue(_T("cx"), pCircle->center().x);
    n.SetAValue(_T("cy"), pCircle->center().y);
    n.SetAValue(_T("r"), pCircle->radius());
  }

  void saveAcDbArc(CXmlNodeWrapper& np, AcDbArc* pArc)
  {
    CXmlNodeWrapper n = CXmlNodeWrapper(np.AppendChild(_T("path")));
    saveAcDbEntity(n, pArc);

    AcGePoint3d ptStart, ptEnd;
    pArc->getStartPoint(ptStart);
    pArc->getEndPoint(ptEnd);

    CString s; s.Format(_T("M %d %d A %d %d %d %d %d %d %d"), ptStart.x, ptStart.y, pArc->radius(), pArc->radius(), 0, 0, 0, ptEnd.x, ptEnd.y);
    n.SetAValue(_T("d"), s);
    //n.SetAValue(_T("fill"), _T("\"#FF00FFA0\""));
    n.SetAValue(_T("stroke"), _T("\"black\""));
    n.SetAValue(_T("stroke-width"), _T("\"2\""));
  }

  //    <TvmEllipse id="13" l="0" c="16777215" cp="0" ac="7" lt="2" lw="1" hl="" lv="0" t="0" dl="false" vd="0;" x="4388.85" y="275.04" rw="301.06" rh="577.32" a="0" cg="144" ft="0" sc="true"/>
  void saveAcDbEllipse(CXmlNodeWrapper& np, AcDbEllipse* pEllipse)
  {
    CXmlNodeWrapper n = CXmlNodeWrapper(np.AppendChild(_T("ellipse")));
    saveAcDbEntity(n, pEllipse);

    AcGePoint3d center; AcGeVector3d unitNormal, majorAxis;
    double radiusRatio, startAngle, endAngle;
    Acad::ErrorStatus es = pEllipse->get(center, unitNormal, majorAxis, radiusRatio, startAngle, endAngle);

    n.SetAValue(_T("x"), center.x);
    n.SetAValue(_T("y"), center.y);
    //n.SetAValue(_T("t"), pEllipse->center().z);

    n.SetAValue(_T("a"), majorAxis.angleTo(AcGeVector3d::kXAxis));
    n.SetAValue(_T("rw"), majorAxis.length());
    n.SetAValue(_T("rh"), majorAxis.length() * radiusRatio);
  }

  LPCTSTR sHAlign(AcDb::TextHorzMode hmode)
  {
    switch (hmode) {
    case AcDb::kTextLeft:  return _T("tahLeft");
    case AcDb::kTextCenter:  return _T("tahCenter");
    case AcDb::kTextRight:  return _T("tahRight");
    case AcDb::kTextAlign:  return _T("tahEntered");
    case AcDb::kTextMid:  return _T("tahCenter");
    case AcDb::kTextFit:  return _T("tahJustify");
    }
    return _T("tahLeft");
  }

  LPCTSTR sVAlign(AcDb::TextVertMode vmode)
  {
    switch (vmode) {
    case AcDb::kTextBase:  return _T("tavCenter");
    case AcDb::kTextBottom:  return _T("tavBottom");
    case AcDb::kTextTop:  return _T("tavTop");
    case AcDb::kTextVertMid:  return _T("tavCenter");
    }
    return _T("tavBottom");
  }

  void saveAcDbText(CXmlNodeWrapper& np, AcDbText* pText)
  {
    //<text x="20" y="35" class="small">My</text>
    CXmlNodeWrapper n = CXmlNodeWrapper(np.AppendChild(_T("text")));
    saveAcDbEntity(n, pText);

    AcGePoint3d pt = pText->position(); //rx_getTextPosition(pText, pt);
    double h = pText->height(); //rx_getTextHeight(pText, h);
    double a = pText->rotation(); //rx_getTextRotation(pText, a);
    //CString st; rx_getTextStyleName(pText, st);
    AcDb::TextHorzMode hmode = pText->horizontalMode();
    AcDb::TextVertMode vmode = pText->verticalMode();

    if (vmode == AcDb::kTextBase)  // У текста E21 отсутствует понятие базоваой линии!!! 
    {
      if (hmode == AcDb::kTextLeft || hmode == AcDb::kTextAlign || hmode == AcDb::kTextFit) // use pText->position();
      {
        AcGeVector3d v = AcGeVector3d::kYAxis;
        v.rotateBy(a, AcGeVector3d::kZAxis);
        v *= h / 3;
        pt += v;  //Заменил AcDb::kTextBase на AcDb::kTextCenter + смещение
      }
    }

    n.SetAValue(_T("x"), pt.x);
    n.SetAValue(_T("y"), pt.y);
    n.SetAValue(_T("sz"), h);
    //n.SetAValue(_T("st"), st);
    n.SetAValue(_T("a"), a);

    n.SetAValue(_T("ha"), sHAlign(pText->horizontalMode()));
    n.SetAValue(_T("va"), sVAlign(pText->verticalMode()));

    if (fabs(pText->widthFactor() - 1) > -0.001)
    {
      n.SetAValue(_T("cm"), pText->widthFactor());
    }

    switch (pText->horizontalMode()) {
    case AcDb::kTextAlign:
    case AcDb::kTextFit:
      //n.SetAValue(_T("w"), pText->position().distanceTo(pText->alignmentPoint()));
      double w = pText->position().distanceTo(pText->alignmentPoint());
      n.SetAValue(_T("fw"), w);
    }

    CString text = pText->textString(); text.Replace(_T("\n"), _T("#13#10"));
    n.SetText(text);
  }

  void saveAcDbMText(CXmlNodeWrapper& np, AcDbMText* pMText)
  {
    CXmlNodeWrapper n = CXmlNodeWrapper(np.AppendChild(_T("text")));
    saveAcDbEntity(n, pMText);

    AcGePoint3d pt = pMText->location();   //rx_getTextPosition(pMText, pt);
    double h; pMText->textHeight(); //rx_getTextHeight(pMText, h);
    double a; pMText->rotation();  //rx_getTextRotation(pMText, a);
    //CString st; rx_getTextStyleName(pMText, st);

    //AcDbMText::AttachmentPoint at; //rx_getTextAttachment(pMText, at);
    AcDb::TextHorzMode hmode; AcDb::TextVertMode vmode;
    //rx_alignByAttachment(at, hmode, vmode);

    n.SetAValue(_T("x"), pt.x);
    n.SetAValue(_T("y"), pt.y);
    n.SetAValue(_T("sz"), h);
    //n.SetAValue(_T("st"), st);
    n.SetAValue(_T("ha"), sHAlign(hmode));
    n.SetAValue(_T("va"), sVAlign(vmode));
    n.SetAValue(_T("a"), a);

#ifdef _NCAD_BUILD
    switch (pMText->horizontalMode()) {
    case AcDb::kTextAlign:
    case AcDb::kTextFit:
      n.SetAValue(_T("w"), pMText->actualWidth());
    }
#endif

    CString text = pMText->contents(); //rx_getTextString(pMText, text); 
    text.Replace(_T("\n"), _T("#13#10"));
    n.SetText(text);
  }


  void saveEntity(CXmlNodeWrapper& np, AcDbEntity* pEnt, const AcGeMatrix3d& ecsMatrix = AcGeMatrix3d::kIdentity)
  {
    if (pEnt->isKindOf(AcDbLine::desc()))
      saveAcDbLine(np, AcDbLine::cast(pEnt));
    else
      if (pEnt->isKindOf(AcDbPolyline::desc()))
        saveAcDbPolyline(np, AcDbPolyline::cast(pEnt));
      else
        if (pEnt->isKindOf(AcDbCircle::desc()))
          saveAcDbCircle(np, AcDbCircle::cast(pEnt));
        else
          if (pEnt->isKindOf(AcDbArc::desc()))
            saveAcDbArc(np, AcDbArc::cast(pEnt));
    //else
    //if (pEnt->isKindOf(AcDbEllipse::desc()))
    //  saveAcDbEllipse(np, AcDbEllipse::cast(pEnt));
          else
            if (pEnt->isKindOf(AcDbText::desc()))
              saveAcDbText(np, AcDbText::cast(pEnt));
            else
              if (pEnt->isKindOf(AcDbMText::desc()))
                saveAcDbMText(np, AcDbMText::cast(pEnt));
              else
                if (pEnt->isKindOf(AcDbBlockReference::desc()))
                {
                  AcDbBlockReference* pBlkRef = AcDbBlockReference::cast(pEnt);

                  AcGeMatrix3d mat = pBlkRef->blockTransform();

                  AcDbVoidPtrArray pEnts;
                  pBlkRef->explode(pEnts);
                  for (int i = 0; i < pEnts.length(); i++)
                  {
                    AcDbEntity* pEnt = (AcDbEntity*)pEnts[i];

                    if (!ecsMatrix.isEqualTo(AcGeMatrix3d::kIdentity))
                      pEnt->transformBy(ecsMatrix);

                    saveEntity(np, pEnt, mat);
                    delete pEnt;
                  }
                }
    //else
    //  m_iCount--;
  }

  void saveObjIds(CXmlNodeWrapper& np, const AcDbObjectIdArray& objIds, const AcGeMatrix3d& ecsMatrix = AcGeMatrix3d::kIdentity)
  {
    for (int i = 0; i < objIds.length(); i++)
    {
      AcDbEntityPtr pEnt;
      if (rxOpenObject(pEnt, objIds[i]) == Acad::eOk)
        saveEntity(np, pEnt, ecsMatrix);
    }
  }

  //  void saveAcDbBlockEntity(CXmlNodeWrapper &n, AcDbEntity *pEnt)
  //  {
  //    n = n.AppendChild(_T("Block"));
  //
  //    saveAcDbEntity(n, pEnt);
  //
  //    n = n.Parent();
  //    n = n.AppendChild(tagEntities);
  //    AcDbEntity *pEnt = NULL;
  //
  //    AcDbWBox *pBox = AcDbWBox::cast(pWEnt);
  //    if (pBox && pBox->extBlockId().isValid())
  //    {
  //      AcGeMatrix3d ecs;
  //      pBox->getEcs(ecs);
  //
  //      AcDbBlockTableRecord *pBlockDef;
  //      ARXOK(acdbOpenObject(pBlockDef, pBox->extBlockId(), AcDb::kForRead));
  //      // Set up a block table record iterator to iterate over the attribute definitions.
  //      AcDbBlockTableRecordIterator *pIterator;
  //      ARXOK(pBlockDef->newIterator(pIterator));
  //      AcDbEntity *pItEnt, *pEnt;
  //      for (pIterator->start(); !pIterator->done(); pIterator->step())
  //      {
  //        ARXOK(pIterator->getEntity(pItEnt, AcDb::kForRead));
  //
  //        pEnt = (AcDbEntity *)pItEnt->clone();
  //        pEnt->transformBy(ecs);
  //
  //        if (pEnt->isKindOf(AcDbLine::desc()))
  //          saveAcDbLine(CXmlNodeWrapper(n.AppendChild(tagLine)), (AcDbLine *)pEnt);
  //        else
  //          if (pEnt->isKindOf(AcDbPolyline::desc()))
  //            saveAcDbPolyline(n, (AcDbPolyline *)pEnt);
  //          else
  //            if (pEnt->isKindOf(AcDbArc::desc()))
  //              saveAcDbArc(CXmlNodeWrapper(n.AppendChild(tagArc)), (AcDbArc *)pEnt);
  //            else
  //              if (pEnt->isKindOf(AcDbCircle::desc()))
  //                saveAcDbCircle(CXmlNodeWrapper(n.AppendChild(tagCircle)), (AcDbCircle *)pEnt);
  //              else
  //                if (pEnt->isKindOf(AcDbEllipse::desc()))
  //                  saveAcDbEllipse(CXmlNodeWrapper(n.AppendChild(tagEllipse)), (AcDbEllipse *)pEnt);
  //                else
  //        delete pEnt;
  //
  //        pItEnt->close();
  //      }
  //
  //#ifndef _NCAD_BUILD_NODELETE
  //      delete pIterator;
  //#endif
  //    }
  //
  //    AcDbObjectIdArray objIdAr = pWEnt->linkIds();
  //    try
  //    {
  //      for (int i = 0; i < objIdAr.length(); i++)
  //      {
  //        if (acdbOpenObject(pEnt, objIdAr[i], AcDb::kForRead) == Acad::eOk)
  //        {
  //          if (pEnt->isKindOf(AcDbLine::desc()))
  //            saveAcDbLine(n, (AcDbLine *)pEnt);
  //          else
  //            if (pEnt->isKindOf(AcDbPolyline::desc()))
  //              saveAcDbPolyline(CXmlNodeWrapper(n.AppendChild(tagPolyline)), (AcDbPolyline *)pEnt);
  //            else
  //              if (pEnt->isKindOf(AcDbArc::desc()))
  //                saveAcDbArc(CXmlNodeWrapper(n.AppendChild(tagArc)), (AcDbArc *)pEnt);
  //              else
  //                if (pEnt->isKindOf(AcDbCircle::desc()))
  //                  saveAcDbCircle(CXmlNodeWrapper(n.AppendChild(tagCircle)), (AcDbCircle *)pEnt);
  //                else
  //                  if (pEnt->isKindOf(AcDbEllipse::desc()))
  //                    saveAcDbEllipse(CXmlNodeWrapper(n.AppendChild(tagEllipse)), (AcDbEllipse *)pEnt);
  //                  else
  //                    if (pEnt->isKindOf(AcDbHatch::desc()))
  //                    {
  //                      //saveAcDbHatch(CXmlNodeWrapper(n.AppendChild(tagHatch)),       (AcDbHatch*)pEnt);
  //                      CXmlNodeWrapper ne = n.AppendChild(tagHatch);
  //                      saveAcDbHatch(ne, (AcDbHatch *)pEnt);
  //                      if (ne.NumNodes() == 0)
  //                        n.RemoveNode(ne.Detach());
  //                    }
  //                    else
  //#ifdef DBWIPE_ON
  //                      if (pEnt->isKindOf(AcDbWipeout::desc()))
  //                        saveAcDbWipeout(CXmlNodeWrapper(n.AppendChild(tagWipeout)), (AcDbWipeout *)pEnt);
  //                      else
  //#endif
  //                        if (pEnt->isKindOf(AcDbText::desc()))
  //                        {
  //                          CWAttribute wAttr;
  //                          if (wAttr.updateXdata(pEnt) == Acad::eOk)
  //                          {
  //                            wAttr.multi = false;
  //                            CString s;
  //                            if (tagAttrByOwnCode(wAttr, s))
  //                              saveSlot(CXmlNodeWrapper(n.AppendChild(s)), (AcDbText *)pEnt, wAttr);
  //                          }
  //                          else
  //                            saveAcDbText(CXmlNodeWrapper(n.AppendChild(tagText)), (AcDbText *)pEnt);
  //                        }
  //                        else
  //                          if (pEnt->isKindOf(AcDbMText::desc()))
  //                          {
  //                            CWAttribute wAttr;
  //                            if (wAttr.updateXdata(pEnt) == Acad::eOk) // clone() не принадлежат базе !!!
  //                            {
  //                              wAttr.multi = true;
  //                              CString s;
  //                              if (tagAttrByOwnCode(wAttr, s))
  //                                saveMSlot(CXmlNodeWrapper(n.AppendChild(s)), (AcDbMText *)pEnt, wAttr);
  //                            }
  //                            else
  //                              saveAcDbMText(CXmlNodeWrapper(n.AppendChild(tagMText)), (AcDbMText *)pEnt);
  //                          }
  //                          else
  //                            ;
  //          pEnt->close();
  //        }
  //        else
  //        {
  //          //RXASSERT(FALSE);
  //        }
  //        pEnt = NULL;
  //      }
  //    }
  //    catch (Acad::ErrorStatus es)
  //    {
  //      PRINT_STATUS_ERROR(es);
  //    }
  //    catch (...)
  //    {
  //      ASSERT(FALSE);
  //    }
  //    if (pEnt) pEnt->close();
  //
  //    m_bLinked = false;
  //    n = n.Parent();
  //    n = n.FindNode(tagEntity);
  //  }

public:

  void DwgToSWG(const AcDbObjectIdArray& objIds, LPCTSTR fileName)
  {
    AcDbDatabase* pDb = NULL;
    if (objIds.length() != 0)
      pDb = objIds[0].database();

    CString s;
    if (pDb)
      s.Format(_T("<svg xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\" width = \"%d\" height = \"%d\"> </svg>"),
        pDb->extmax().x - pDb->extmin().x, pDb->extmax().y - pDb->extmin().y);
    else
      s = _T("<svg xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\" width = \"100\" height = \"100\"> </svg>");

    m_xmlDoc.LoadXML(s);

    CXmlNodeWrapper nodeDoc(m_xmlDoc.AsNode());
    //nodeDoc.SetAValue(_T("version"), _T("1.1"));

    m_lrIds.setLogicalLength(0);
    ArxDbgUtils::collectSymbolIds(AcDbLayerTableRecord::desc(), m_lrIds, pDb);
    m_ltIds.setLogicalLength(0);
    ArxDbgUtils::collectSymbolIds(AcDbLinetypeTableRecord::desc(), m_ltIds, pDb);

    saveObjIds(nodeDoc, objIds);
    saveLinetypes(nodeDoc);
    saveTextStyles(nodeDoc);

    m_xmlDoc.SaveToFileAsFmt(fileName);
  }
};


