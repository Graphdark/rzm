#include "stdafx.h"

#include "ac_svg_generator.h"

#include "SvgUtils.h"

//#define USE_WENTITY

#ifdef USE_WENTITY
#include "RzmCommon\RzmCommon_DBX\RzmZoneGrid.h"
#include "Wiring_1\Wiring_DBX\AcDbWEntity.h"
#endif

using namespace SvgUtils;

extern char* sRGB(int r, int g, int b);

string svg_gen::svgLineWidthEntToStr(AcDbEntity* pEnt)
{
  double w = 1.0,  scaleFactor = max(x_max - x_min, y_max - y_min) / 2500;
  AcDb::LineWeight lw = pEnt->lineWeight();
  switch (lw)
  {
  case AcDb::kLnWt000:
  case AcDb::kLnWtByLayer:
  case AcDb::kLnWtByBlock:
  case AcDb::kLnWtByLwDefault:
    w = 0.15;
    break;
  default:
    w = (int)lw * 0.01 * scaleFactor;
  }
  w = max(0.15, w * scaleFactor);
  return string(fToChars(w));
}

string objToEnameStr(const AcDbObject* obj)
{
  ASSERT(obj != NULL);
  if (obj->objectId().isNull())
    return "";

  ads_name ent;
  acdbGetAdsName(ent, obj->objectId());
  static char s[20];
  sprintf(s, "%llx", ent[0]);
  return s;
}

string objToClassStr(const AcRxObject* obj)
{
  ASSERT(obj != NULL);
  AcRxClass* rxClass = obj->isA();
  if (rxClass == NULL) {
    ASSERT(0);
    return "*Unknown*";
  }
  return string(CT2A(obj->isA()->name()));
}


string textStyleFontAttrs(const AcDbObjectId& styleId, double &textHeight)
{
  static string s(' ', 120);
  s = "";

  RXASSERT(AcDbObjectId::kNull != styleId);
  ACHAR* typeface = { 0 }; bool bold = false, italic = false; int charset = 0, pitchAndFamily = 0;

  AcDbObject* pObj;
  if (acdbOpenAcDbObject(pObj, styleId, AcDb::kForRead) == Acad::eOk)
  {
    AcDbTextStyleTableRecord* pStyle = AcDbTextStyleTableRecord::cast(pObj);
    if (pStyle == NULL)
    {
      pObj->close();
      return "";
    }

    if (textHeight < 0.001)
      textHeight = pStyle->textSize();

    //pStyle->getName(ps);
    //if (ps)
    //  styleName = CT2CA(ps);

    pStyle->font(typeface, bold, italic, charset, pitchAndFamily);

    if (!typeface || !*typeface)
    {
      pStyle->bigFontFileName(typeface);
      if (!typeface || !*typeface)
        pStyle->fileName(typeface);
    }

    pObj->close();

    if (typeface && *typeface && StrStrI(typeface, _T(".shx")))
      typeface = (ACHAR*)_T("Txt");  //Arial

    if (typeface && *typeface)
      addAttribute(s, "font-family", string(CT2A(typeface)));
    //  s += " font-size=\"" + charset;

    if (bold)
      addAttribute(s, "font-weight", "bold");
    if (italic)
      addAttribute(s, "font-style", "italic");
  }
  return s;
}

string HorizontalAlignmentToTextAnchor(AcDb::TextHorzMode horizontalAlignment) 
{
  switch (horizontalAlignment) {
  case AcDb::TextHorzMode::kTextCenter:
    return "middle";

  case AcDb::TextHorzMode::kTextLeft:
    break;

  case AcDb::TextHorzMode::kTextRight:
    return "end";
  }
  return "";
}

string HorizontalAlignmentToTextAnchor(AcDbMText::AttachmentPoint attachmentPoint) {
  switch (attachmentPoint) {
  case AcDbMText::AttachmentPoint::kTopLeft:
    return "";

  case AcDbMText::AttachmentPoint::kTopCenter:
    return "middle";

  case AcDbMText::AttachmentPoint::kTopRight:
    return "end";

  case AcDbMText::AttachmentPoint::kMiddleLeft:
    return "";

  case AcDbMText::AttachmentPoint::kMiddleCenter:
    return "middle";

  case AcDbMText::AttachmentPoint::kMiddleRight:
    return "end";

  case AcDbMText::AttachmentPoint::kBottomLeft:
    return "";

  case AcDbMText::AttachmentPoint::kBottomCenter:
    return "middle";

  case AcDbMText::AttachmentPoint::kBottomRight:
    return "end";

  default:
    return "";
  }
}

double AlignmentToVerticalAdjustment(AcDbMText::AttachmentPoint attachmentPoint, double textHeight)
{
  switch (attachmentPoint) {
  case AcDbMText::AttachmentPoint::kTopLeft:
  case AcDbMText::AttachmentPoint::kTopCenter:
  case AcDbMText::AttachmentPoint::kTopRight:
    return textHeight * 0.8;

  case AcDbMText::AttachmentPoint::kMiddleLeft:
  case AcDbMText::AttachmentPoint::kMiddleCenter:
  case AcDbMText::AttachmentPoint::kMiddleRight:
    return textHeight * 0.4;

  case AcDbMText::AttachmentPoint::kBottomLeft:
  case AcDbMText::AttachmentPoint::kBottomCenter:
  case AcDbMText::AttachmentPoint::kBottomRight:
    return 0;

  default:
    return textHeight;
  }
}


typedef AcGePoint2d point;

char standard_header[] =
//"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\"  \
//viewBox=\"0.0 0.0 %f %f\" preserveAspectRatio=\"xMidYMid meet\" \
//version=\"1.1\">\n\t<link xmlns=\"http://www.w3.org/1999/xhtml\" rel=\"stylesheet\" \
//href=\"svg_style.css\" type=\"text/css\"/>\n  <desc>Produce by Rozmisel</desc>\n";

"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\""
"\n\tviewBox=\"0 0 %s %s\" version=\"1.1\" preserveAspectRatio=\"xMidYMid meet\""
"\n\tstroke-linecap=\"round\" stroke-linejoin=\"round\" fill-rule=\"evenodd\">"
"\n\t<link xmlns:xlink=\"http://www.w3.org/1999/xlink\" xml:space=\"preserve\"/>"
"\n\t<link xmlns=\"http://www.w3.org/1999/xhtml\" rel=\"stylesheet\" href=\"svg_style.css\" type=\"text/css\"/>"
"\n\t<desc>Produce by Rozmisel</desc>\n";

svg_gen::svg_gen()
{

}

string svg_gen::solve_quadratic_beizer(vector<point> pt)
{
  if (pt.size() >= 4)
  {
    string res;
    char pair[32] = { 0 };
    snprintf(pair, 31, "%.3f,%.3f ", pt[0].x, pt[0].y);
    res.append(pair);
    int i = 0;
  start:
    double Px = 2 * pt[i + 1].x - 0.5 * (pt[i].x + pt[i + 2].x);
    double Py = 2 * pt[i + 1].y - 0.5 * (pt[i].y + pt[i + 2].y);
    res = res + 'Q';
    memset(pair, 0, 32);
    snprintf(pair, 31, "%.3f,%.3f ", Px, Py);
    res.append(pair);
    memset(pair, 0, 32);
    snprintf(pair, 31, "%.3f,%.3f ", pt[i + 2].x, pt[i + 2].y);
    res.append(pair);
    if (pt.size() > i + 4) { i = i + 2; goto start; }
    else
    {
      i = i + 3;
      while (i < pt.size())
      {
        memset(pair, 0, 32);
        snprintf(pair, 31, "L%.3f,%.3f ", pt[i].x, pt[i].y);
        res.append(pair);
        ++i;
      }
    }
    return res;
  }
  else
  {
    string res = "";
    return res;
  }
}

//#define USE_STYLE

string svg_gen::svgStyleStr(AcDbEntity* pEnt, double* pTextHeigth)
{
  string s;
#ifdef USE_STYLE
  s = "style=\""; 
  //s += "fill:none; ";
  s += "stroke: #";
  s += ColorEntToStrRGB(pEnt);
  //s += "; stroke-width: ";
  //s += LineWidthEntToStr(pEnt);
  if (pEnt->isKindOf(AcDbText::desc()))
  {
    AcDbText* pText = (AcDbText*)pEnt;
    s += "; font-size: ";
    s += fToChars(pText->height()*0.93);
    CString cs = rx_getTextFontName(pText->textStyle());
    if (cs.GetLength()) {
      s += "; font-family: "; s += CT2CA(cs, CP_UTF8);
    }
    //s += "; text-anchor: ";
    switch (pText->horizontalMode())
    {
    case AcDb::kTextMid:
    case AcDb::kTextCenter:
      s += "; text-anchor: middle";break;
    case AcDb::kTextRight: 
      s += "; text-anchor: end"; break;
    case AcDb::kTextAlign:
    case AcDb::kTextFit:
    case AcDb::kTextLeft:
    default:
      ;// s += "start";
    }
    s += " fill=\"none\"";
  }
  s += "\" ";
#else
  addAttribute(s, "stroke", string("#") + ColorEntToStrRGB(pEnt));
  if (bUseLineWidth)
    addAttribute(s, "stroke-width", svgLineWidthEntToStr(pEnt));

  if (pEnt->isKindOf(AcDbText::desc()))
  {
    AcDbText* pText = (AcDbText*)pEnt;
    double h = pText->height();
    s += textStyleFontAttrs(pText->textStyle(), h);
    if (pTextHeigth)
      *pTextHeigth = h;
    addAttribute(s, "font-size", h);
    addAttribute(s, "text-anchor", HorizontalAlignmentToTextAnchor(pText->horizontalMode()));
    addAttribute(s, "fill", "none");
  }

  pattern2dasharray(pEnt->linetypeId(), 1, s);
#endif
  return s;
}

double svg_gen::LX(double x) const
{
  return bOriginExt ? x : x - x_min;
}

double svg_gen::LY(double y) const
{
  return bOriginExt ? y : y_max - y;
}

AcGePoint2d svg_gen::LXY(const AcGePoint2d& pt) const
{
  return AcGePoint2d(LX(pt.x), LY(pt.y));
}

string svg_gen::svgPathLineTo(const AcGePoint2d& point) const
{
  return "L" + fToChars(point.x) + "," + fToChars(point.y) + " ";
}

string svg_gen::svgPathMoveTo(const AcGePoint2d& point) const
{
  return "M" + fToChars(point.x) + "," + fToChars(point.y) + " ";
}

string svg_gen::svgPathArc(const AcGePoint2d& point, double radius_x, double radius_y, double x_axis_rotation, bool large_arc_flag, bool sweep_flag) const
{
  return "A" + fToChars(radius_x) + "," + fToChars(radius_y) + " " +
    fToChars(x_axis_rotation) + " " +
    (large_arc_flag ? "1" : "0") + "," +
    (sweep_flag ? "1" : "0") + " " +
    fToChars(point.x) + "," + fToChars(point.y) + " ";
}

string svg_gen::svgPathArc(AcDbArc* arc) const
{
  AcGePoint3d pt;
  arc->getEndPoint(pt);
  AcGePoint2d endpoint = LXY(pt.asPoint2d());
  double radius = arc->radius();
  double startangle = rad2deg(arc->startAngle());
  double endangle = rad2deg(arc->endAngle());

  if (endangle <= startangle)
    endangle += 360;

  bool large_arc_flag = ((endangle - startangle) > 180);
    //((endangle > startangle) && ((endangle - startangle) > 180)) ||
    //((endangle < startangle) && ((360 + endangle - startangle) >= 180));

  bool sweep_flag = false;
  //if (arc->isReversed())
  //{
  //  large_arc_flag = !large_arc_flag;
  //  sweep_flag = !sweep_flag;
  //}
  return svgPathArc(endpoint, radius, radius, 0.0, large_arc_flag, sweep_flag);
}

//void svg_gen::polyline(AcDbPolyline* pPolyline)
//{
//  string s;
//
//  AcGePoint3d pt;
//  //pPolyline->getStartPoint(pt);
//  string path; // = svgPathMoveTo(LXY(pt.asPoint2d()));
//
//  AcDbVoidPtrArray set;
//  pPolyline->explode(set);
//  for (int i = 0; i < set.length(); i++)
//  {
//    AcDbEntity* pEnt = AcDbEntity::cast((AcDbObject*)(set[i]));
//    if (pEnt->isKindOf(AcDbArc::desc()))
//    {
//      //if (i == 0)
//      {
//        ((AcDbArc*)pEnt)->getStartPoint(pt);
//        path += svgPathMoveTo(LXY(pt.asPoint2d()));
//      }
//      path += svgPathArc((AcDbArc*)pEnt);
//    }
//    else
//      if (pEnt->isKindOf(AcDbLine::desc()))
//      {
//        //if (i == 0)
//        {
//          ((AcDbLine*)pEnt)->getStartPoint(pt);
//          path += svgPathMoveTo(LXY(pt.asPoint2d()));
//        }
//        path += svgPathLineTo(LXY(((AcDbLine*)pEnt)->endPoint().asPoint2d()));
//      }
//  }
//
//  //if (pPolyline->isClosed()) 
//  //  path += " Z";
//
//  s = "\n\t<path";
//  addAttribute(s, "d", path);
//  s += svgStyleStr(pPolyline);
//  s += "/>";
//
//  resulting_svg += s;
//}

void svg_gen::line(AcDbLine* pLine)
{
  string x1 = fToChars(LX(pLine->startPoint().x));
  string x2 = fToChars(LX(pLine->endPoint().x));
  string y1 = fToChars(LY(pLine->startPoint().y));
  string y2 = fToChars(LY(pLine->endPoint().y));
  string path = "M x1 y1 L x2 y2";
  path.replace(path.find("x1"), 2, x1);
  path.replace(path.find("x2"), 2, x2);
  path.replace(path.find("y1"), 2, y1);
  path.replace(path.find("y2"), 2, y2);
  resulting_svg += "\n\t<path ";
  resulting_svg += svgStyleStr(pLine);
  resulting_svg += " d=\"";
  resulting_svg += path;
  resulting_svg += "\"/>";
}

void svg_gen::arc(AcDbArc* pArc)
{
  //PathElement path;
  //Utils::ArcToPath(
  //  path, true,
  //  LXY(pArc->center().asPoint2d()), pArc->radius(), pArc->startAngle(), pArc->endAngle());

  string s = "\n\t<path ";
  s += svgStyleStr(pArc);
  s += " d=\"";
  //s += path.toString();
  AcGePoint3d pt;
  pArc->getStartPoint(pt);
  s += svgPathMoveTo(LXY(pt.asPoint2d()));
  s += svgPathArc(pArc);
  s += "\"/>";
  resulting_svg += s;
}

void svg_gen::circle(AcDbCircle* pCircle)
{
  string s;
  s += "\n\t<circle ";
  s += svgStyleStr(pCircle);
  addAttribute(s, "cx", LX(pCircle->center().x));
  addAttribute(s, "cy", LY(pCircle->center().y));
  addAttribute(s, "r", pCircle->radius());
  s += "/>";
  resulting_svg += s;
}

void svg_gen::ellipse(AcDbEllipse* pEllipse)
{
  double cx = pEllipse->center().x;
  double cy = pEllipse->center().y;
  double mx = pEllipse->majorAxis().x;
  double my = pEllipse->majorAxis().y;

  double rr = pEllipse->radiusRatio();
  double rx = sqrt(pow(mx, 2) + pow(my, 2));
  double ry = rx * rr;

  double angle1 = pEllipse->startAngle();
  double angle2 = pEllipse->endAngle();
  double amin = angle1 < angle2 ? angle1 : angle2;
  double amax = angle1 > angle2 ? angle1 : angle2;

  string s;

  if (amin == amax || fabs(amax - amin) >= 2*PI - 0.05)
  {
    double a = 180 - rad2deg(pEllipse->majorAxis().asVector2d().angle());
    double rot = atan2(pEllipse->majorAxis().y, pEllipse->majorAxis().x) * 180.0 / PI;
    assert(fabs(a - rot) < 0.001);
    s += "\n\t<ellipse ";
    s += svgStyleStr(pEllipse);
    addAttribute(s, "cx", LX(cx));
    addAttribute(s, "cy", LY(cy));
    addAttribute(s, "rx", rx); //pEllipse->majorAxis().length()
    addAttribute(s, "ry", ry); //pEllipse->minorAxis().length()
    //addAttribute(s, "transform", string("translate(") + fToChars(cx - x_min) + ", " + fToChars(y_max - cy) + ") " + "rotate(" + fToChars(a) + ")");
    if (fabs(a) > 0.001)
      addAttribute(s, "transform", string("rotate(") + fToChars(a) + " " + fToChars(LX(cx)) + " " + fToChars(LY(cy)) + ")");
    s += "/>";
  }
  else 
  {
    if (fabs(mx < 0.001) || fabs(my < 0.001))
    {
      if (mx < 0.001)
      {
        double t = mx;
        mx = my;
        my = t;
      }
      if (fabs(mx) < 0.001)
        return;
    }

    s = "\n\t<polyline ";
    s += svgStyleStr(pEllipse);
    s += " points=\"";

    double angle = atan(my / mx);
    const int count_vertex = 64;
    double alpha_array[count_vertex];
    double x[count_vertex];
    double y[count_vertex];

    for (int i = 0; i < count_vertex; i++)
    {
      if (fabs(angle) > 0.001)
      {
        if (amax < 6.28 || amin > 0.001)
          alpha_array[i] = (2 * M_PI) - (amin + i * (amax - amin) / count_vertex /* - angle*/);
        else
          alpha_array[i] = i * ((2 * M_PI) / count_vertex) - angle;

        double xr = rx * cos(alpha_array[i]);
        double yr = ry * sin(alpha_array[i]);
        x[i] = xr * cos(angle) - yr * sin(angle) + cx;
        y[i] = xr * sin(angle) + yr * cos(angle) + cy;
      }
      else //if (angle <= 0.001)
      {
        alpha_array[i] = i * (2 * M_PI / count_vertex);
        x[i] = rx * cos(alpha_array[i]) + cx;
        y[i] = ry * sin(alpha_array[i]) + cy;
      }
      s += fPairToString(LX(x[i]), LY(y[i]));
    }
    ///замкнуть на начало
    if (amin < 0.001 && amax > 6.28)
    {
      s += fPairToString(LX(x[0]), LY(y[0]));
      //res.append("Z");
    }
    s.append("\"/>");
  }
  resulting_svg += s;
}

void svg_gen::ellipse2(AcDbEllipse* pEllipse)
{
  string s;
  double cx = pEllipse->center().x;
  double cy = pEllipse->center().y;
  double mx = pEllipse->majorAxis().x;
  double my = pEllipse->majorAxis().y;
  double rr = pEllipse->radiusRatio();
  //  ep defines the length and rotation of the major axis
  double rx = pEllipse->majorAxis().length();
  double ry = rx * rr;
  double rot = pEllipse->majorAxis().angleTo(AcGeVector3d::kXAxis);	//	in radians
  double sa = pEllipse->startAngle();
  double ea = pEllipse->endAngle();

  if (sa == ea || sa == ea - M_PI * 2) 
  {
    s = "\n\t<ellipse ";
    addAttribute(s, "cx", LX(cx));
    addAttribute(s, "cy", LY(cy));
    addAttribute(s, "rx", rx); //pEllipse->majorAxis().length()
    addAttribute(s, "ry", ry); //pEllipse->minorAxis().length()
    if (rot != 0 && ry != rx)
      addAttribute(s, "transform", string("rotate(") + fToChars(rot) + " " + fToChars(LX(cx)) + " " + fToChars(LY(cy)) + ")");
    s += svgStyleStr(pEllipse);
    //addAttribute(s, "fill", "none");
    //addAttribute(s, "stroke", ColorUtils::GetHtmlColor(_ellipse, _ellipse.Color));
    //addAttribute(s, "StrokeWidth", LineUtils.GetLineWeight(_ellipse.LineWeight, _ellipse, _ctx));
    addAttribute(s, "id", (unsigned int)pEllipse->objectId().handle());
    CT2CA sUtf8(pEllipse->desc()->name(), CP_UTF8);
    addAttribute(s, "class", string(sUtf8));
    s += "/>";
  }
  else 
  {
    //	Elliptic arc
    PathElement path;
    Utils::EllipseArcToPath(
      path, true,
      XY(LX(cx), LY(cy)), XY(mx, my), rr, sa, ea);

    s = "\n\t<path ";
    s += " d=\"";
    s += path.toString();
    s += "\" ";
    s += svgStyleStr(pEllipse);
    s += "/>";
  }
  resulting_svg += s;
}

class spline_koeff
{
public:
  double b1;
  double b2;
  double b3;
  int n = 0;
  void coeffs(double t, vector<point> pt, vector<double> k);
};

void spline_koeff::coeffs(double t, vector<AcGePoint2d> pt, vector<double> k)
{
#define k2 k[n+1]
#define k3 k[n+2]
#define k4 k[n+3]
#define k5 k[n+4]
  if (k2 - k4 != 0 && k3 - k4 != 0 && k3 - k5 != 0 && n < (int)pt.size())
  {
    b1 = pow(k4 - t, 2) / (-k4 + k2) / (-k4 + k3);
    b2 = -(t * t * k2 + k2 * k3 * k5 + k2 * k3 * k4 - k4 * k2 * k5 - 2 * t * k3 * k2 - k4 * t * t + 2 * t * k4 * k5 - t * t * k5 + t * t * k3 - k3 * k4 * k5) / (-k4 + k2) / (-k4 + k3) / (-k5 + k3);
    b3 = pow(-t + k3, 2) / (-k4 + k3) / (-k5 + k3);
  }
  else
  {
    b1 = 0; b2 = 0; b3 = 0;
  }
}

void svg_gen::spline_points_and_knots_degree2(vector<point> pt, vector<double> k)
{
  AcGePoint2d pt1, pt2;
  string res = "\n\t<path d=\"M";
  double t = 0;
  vector<AcGePoint2d> pair;
  spline_koeff b123;
  for (int j = 3; j < (int)k.size() - 2; ++j)
  {
    double ink = (k[j] - k[j - 1]) / 10;
    b123.n = j - 3;
    t = k[j - 1];
    int i = 0;
    while (i < 10)
    {
      b123.coeffs(t, pt, k);
#define b1 b123.b1
#define b2 b123.b2
#define b3 b123.b3
#define n b123.n
      pt1.x = b1 * pt[n].x + b2 * pt[n + 1].x + b3 * pt[n + 2].x;
      pt1.y = b1 * pt[n].y + b2 * pt[n + 1].y + b3 * pt[n + 2].y;
      t = t + ink;
      b123.coeffs(t, pt, k);
      pt2.x = b1 * pt[n].x + b2 * pt[n + 1].x + b3 * pt[n + 2].x;
      pt2.y = b1 * pt[n].y + b2 * pt[n + 1].y + b3 * pt[n + 2].y;
      t = t - ink;
      char buf[72] = { 0 };
      snprintf(buf, 71, "%.3f,%.3f %.3f,%.3f ", LX(pt1.x), LY(pt1.y), LX(pt2.x), LY(pt2.y));
      AcGePoint2d pt = { LX(pt1.x), LY(pt1.y) };
      pair.emplace_back(pt);
      if (i == 9)
      {
        AcGePoint2d pt = { LX(pt2.x), LY(pt2.y) };
        pair.emplace_back(pt);
      }
      //res.append(buf);
      t = t + ink;
      ++i;
    }
  }
  res = res + solve_quadratic_beizer(pair);
  res.append("\"/>");
  this->resulting_svg = this->resulting_svg + res;
#undef b1
#undef b2
#undef b3
#undef n
}

//#define MAX(x,y) (x>y?x:y)
//#define MIN(x,y) (x<y?x:y)

void svg_gen::spline(AcDbSpline* pSpline)
{
  int degree; NRX::Boolean rational, closed, periodic;
  NcGePoint3dArray controlPoints;
  NcGeDoubleArray knots, weights;
  double controlPtTol, knotTol;
  pSpline->getNurbsData(degree, rational, closed, periodic, controlPoints, knots, weights, controlPtTol, knotTol);

  //reverse(pSpline->.begin(), spline_knots[i].end());
  //reverse(spline_points[i].begin(), spline_points[i].end());

  vector<point> vPoints;
  vector<double> vKnots;
  int i = pSpline->numControlPoints() - 1;
  for (; i >= 0; --i)
  {
    AcGePoint3d pt;
    pSpline->getControlPointAt(i, pt);
    vPoints.push_back(pt.asPoint2d());
    vKnots.push_back(pSpline->weightAt(i));
  }

  if (degree == 2)
    spline_points_and_knots_degree2(vPoints, vKnots);
  else
  {
    //i = pSpline->numControlPoints() - 1;
    //for (; i >= 0; --i)
    {
      int j = controlPoints.length() - 1;
      string res = "\n\t<polyline points=\"";
      for (; j >= 0; --j)
      {
        point pt = vPoints[i]; //spline_points[i][j];
        char buf[32] = { 0 };
        snprintf(buf, 31, "%.3f,%.3f ", LX(pt.x), LY(pt.y));
        res.append(buf);
      }
      res.append("\"/>");
      resulting_svg += res;
    }
  }
}




void svg_gen::text(AcDbText* pText)
{
  double x, y, h, dy = 0;

  string s;
  s = "\n\t<text ";
  s += svgStyleStr(pText, &h);

  switch (pText->horizontalMode())
  {
    case AcDb::kTextLeft:
    case AcDb::kTextAlign:
    case AcDb::kTextFit:
      x = pText->position().x;
      y = pText->position().y;
      break;
    default:
      x = pText->alignmentPoint().x;
      y = pText->alignmentPoint().y;
  }
  addAttribute(s, "x", LX(x));
  addAttribute(s, "y", LY(y));

  switch (pText->verticalMode())
  {
  case AcDb::kTextTop:      dy = h * 0.8;  break;
  case AcDb::kTextVertMid:  dy = h * 0.4;  break;
  case AcDb::kTextBase:     dy = 0;        break;
  case AcDb::kTextBottom:   dy = h * -0.1; break;
  }
  if (dy > 0)
    addAttribute(s, "dy", dy);

  if (fabs(pText->rotation()) > 0.001) {
    s += " transform=\"rotate(";
    s += fToChars(rad2deg(-pText->rotation()));
    s += " " + fToChars(LX(x)) + " " + fToChars(LY(y));
    s += ")\"";
  }
  s += " >";

  s += "<tspan>";
  s += CT2CA(pText->textString(), CP_UTF8);
  s += "</tspan>";
  s += "</text>";

  resulting_svg += s;
}

//void svg_gen::mtext(AcDbMText* pMText)
//{
//  AcGePoint3d pt = pMText->location();
//  double textHeight = pMText->textHeight();
//  double rot = pMText->rotation() * 180 / PI;
//  string textAnchor = HorizontalAlignmentToTextAnchor(pMText->attachment());
//  double dY = AlignmentToVerticalAdjustment(pMText->attachment(), textHeight);
//  string value(CT2CA(pMText->contents(), CP_UTF8));
//  size_t end;
//  while ((end = value.find("\n")) != string::npos)
//    value = value.replace(end, strlen("\n"), "\\P");
//}

#ifdef USE_WENTITY
string svg_hrefWEnt(AcDbWEntity* pWEnt)
{
  string uname; //"/Objects/Devices||Cables|Documents/?"
  switch (pWEnt->recOwnerType())
  {
  case 1100:
  case 1103: //emtGround
    uname = "/Objects/Devices/?";
    break;
  case 1200: //emtLinkLine
  case 1201:
  case 1202:
  case 1206:
    uname = "/Objects/Lines/?";
    break;
  case 1210: //emtWireLine
  case 1216: //emtWireProtect
  case 1219: //emtConductor
  case 1211: //emtWireJumper
  case 1212: //emtWireScreen
    uname = "/Objects/Wires/?";;
    break;
  case 1214: //emtBusWireLine
    uname = "/Objects/wirebus?";
    break;
  case 1500: //emtWireCable
    uname = "/Objects/Cables/?";
    break;
  case 1800: //emtBus
    uname = "/Objects/Bus/?";
    break;
  default:
    uname = "/Objects/Unknown/?";
  }
  uname += CT2CA(pWEnt->name(), CP_UTF8);
  return uname;
}
#endif // USE_WENTITY

void svg_gen::svg_add(AcDbEntity* pEnt)
{
  if (pEnt->visibility() == AcDb::kInvisible)
    return;

  if (pEnt->isKindOf(AcDbLine::desc()))
    line(AcDbLine::cast(pEnt));
  else
    //if (pEnt->isKindOf(AcDbPolyline::desc()))
    //  polyline(AcDbPolyline::cast(pEnt));
    //else
      if (pEnt->isKindOf(AcDbCircle::desc()))
        circle(AcDbCircle::cast(pEnt));
      else
        if (pEnt->isKindOf(AcDbEllipse::desc()))
          //ellipse(AcDbEllipse::cast(pEnt));
          ellipse2(AcDbEllipse::cast(pEnt));
        else
          if (pEnt->isKindOf(AcDbArc::desc()))
            arc(AcDbArc::cast(pEnt));
          else
            if (pEnt->isKindOf(AcDbSpline::desc()))
              spline(AcDbSpline::cast(pEnt));
            else
              if (pEnt->isKindOf(AcDbText::desc()))
                text(AcDbText::cast(pEnt));
              else
                //if (pEnt->isKindOf(AcDbMText::desc()))
                //  mtext((AcDbMText*)pEnt);
                //else
                if (pEnt->isKindOf(AcDbBlockReference::desc()) 
#ifdef USE_WENTITY
                 || pEnt->isKindOf(AcDbWEntity::desc()) 
                 || pEnt->isKindOf(RzmZoneGrid::desc())
#endif
                 || pEnt->isKindOf(AcDbPolyline::desc())
                 || pEnt->isKindOf(AcDbMText::desc())
                   )
                {
                  if (pEnt->isKindOf(AcDbBlockReference::desc()))
                  {
                    AcDbObjectIdArray objAttrIds;
                    rx_collectAttributes((AcDbBlockReference*)pEnt, objAttrIds);
                    svg_add(objAttrIds);
                  }

                  string s;
                  addAttribute(s, "id", objToEnameStr(pEnt));
                  addAttribute(s, "class", objToClassStr(pEnt));

#ifdef USE_WENTITY
                  bool bIsA = pEnt->isKindOf(AcDbWEntity::desc());
                  if (bIsA)
                  {
                    resulting_svg += "\n<a" + s;
                    addAttribute(resulting_svg, "href", svg_hrefWEnt((AcDbWEntity*)pEnt));
                  }
                  else
#endif
                  {
                    resulting_svg += "\n<g" + s;
                  }
                  resulting_svg += ">";

                  AcDbVoidPtrArray pEnts;
                  pEnt->explode(pEnts);
                  for (int i = 0; i < pEnts.length(); i++)
                    svg_add((AcDbEntity*)pEnts[i]);

#ifdef USE_WENTITY
                  if (bIsA)
                    resulting_svg += "\n</a>";
                  else
#endif
                    resulting_svg += "\n</g>";
                }
}

void svg_gen::svg_add(const AcDbObjectId& objId)
{
  AcDbEntityPtr pEnt;
  if (rxOpenObject(pEnt, objId) == Acad::eOk)
    svg_add(pEnt);
}

void svg_gen::svg_add(const AcDbObjectIdArray& objIds)
{
  for (int i = 0; i < objIds.length(); i++)
    svg_add(objIds[i]);
}

void svg_gen::svg_generate(LPCTSTR fileName, const AcDbObjectIdArray& objIds)
{
  AcDbExtents ext;
  rx_extentsArray(objIds, ext);
  x_min = ext.minPoint().x;
  y_min = ext.minPoint().y;
  x_max = ext.maxPoint().x;
  y_max = ext.maxPoint().y;

  //printf("sizeof=%d\n", sizeof(standard_header));
  char result[sizeof(standard_header) + 64];
  sprintf(result, standard_header, fToChars(x_max - x_min), fToChars(y_max - y_min));
  resulting_svg += result;
  resulting_svg += "\n<g fill=\"none\" stroke=\"black\" stroke-width=\"0.35\"";
  if (bOriginExt || bReverseY)
  {
    string s;
    if (bOriginExt)
    {
      sprintf(result, "translate(0 %s)", fToChars(y_max - y_min).c_str());
      s += result;
      x_min = y_min = x_max = y_max = 0;
    }
    if (bReverseY)
    {
      if (bOriginExt) s += " ";
      s +="scale(1 -1)";
    }
    addAttribute(resulting_svg, "transform", s);
  }
  resulting_svg += ">";

  if (bUseBackground)
  {
    iBackgoundColor = acedGetRGB(0);

    char sbk[8];
    sprintf_s(sbk, 8, "%x", iBackgoundColor);
    //<rect
    //   style="fill:#000000;stroke:none;stroke-opacity:1"
    //   id="rect2"
    //   width="66.189362"
    //   height="46.044777"
    //   x="94.247894"
    //   y="668.36865"
    //   ry="0" />
    string s = "<rect";
    //addAttribute(s, "style", "fill:#000000;stroke:none;stroke-opacity:1");
    addAttribute(s, "fill", string("#") + sbk);
    addAttribute(s, "stroke", "none");
    addAttribute(s, "stroke-opacity", "1");

    addAttribute(s, "width", "100%");
    addAttribute(s, "height", "100%");
    addAttribute(s, "ry", "0");
    s += "/>";
    resulting_svg += s;
  }
  else
    iBackgoundColor = Adesk::UInt32(-1);
 
  svg_add(objIds);
  
  resulting_svg += "\n</g>";
  resulting_svg += "\n</svg>";

  ofstream test_svg;
  test_svg.open(fileName);
  test_svg.write(resulting_svg.c_str(), resulting_svg.size());
  test_svg.close();
}
