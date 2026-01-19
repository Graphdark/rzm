#include "stdafx.h"

#include "SvgUtils.h"

#include <sstream>
#include <string>
#include <xstring>

#define sExMaxStr  _T("Превышение максимальной длины строки")

namespace SvgUtils
{
  int precision = 6;
  Adesk::UInt32 iBackgoundColor = -1;

  double rad2deg(double a) {
    return 180. / M_PI * a;
  }

  string iToString(int i)
  {
    static char buf[8] = { 0 };
    if (snprintf(buf, 7, "%d", i) > 7) throw sExMaxStr;
    return buf;
  }

  string fToChars(double d)
  {
    static char buf[32] = { 0 };
    //gcvt(d, precision, buf);
    if (snprintf(buf, 31, "%.*f", precision, d) > 31) throw sExMaxStr;
    char* s = strchr(buf, '.');
    if (s > 0) {
      // remove trailing zeros:
      int l = strlen(buf);
      while (s < buf + l && buf[l - 1] == '0') {
        buf[--l] = '\0';
      }
      // remove trailing .
      if (buf[l - 1] == '.') {
        buf[l - 1] = '\0';
      }
    }
    return buf;
  }

  string fPairToString(double x, double y)
  {
    return " " + fToChars(x) + "," + fToChars(y);
  }

  void addAttribute(string& s, const char* attr, string value)
  {
    s += " ";
    if (value.empty()) return;
    s += attr;
    s += "=\"";
    s += value;
    s += "\"";
  }

  void addAttribute(string& s, const char* attr, double value)
  {
    addAttribute(s, attr, fToChars(value));
  }

  enum LineWeightType {
    ByDIPs = -4,
    Default = -3,
    ByBlock = -2,
    ByLayer = -1,
    W0 = 0
  };

  struct ConversionContext
  {
    struct  {
      LineWeightType DefaultLineWeight = Default;
      double LineweightScaleFactor = 1;
    } ConversionOptions;
  };


  class LineUtils {
  public:

    static double GetLineWeight(LineWeightType lineWeightType, AcDbEntity* entity, ConversionContext ctx) {

      switch (lineWeightType) {
      case LineWeightType::ByDIPs:
      case LineWeightType::Default:
        return getLineWeightValue(ctx.ConversionOptions.DefaultLineWeight, entity, ctx);
      case LineWeightType::ByBlock:
        //BlockRecord block = entity.Owner as BlockRecord;
        //if (block == null) {
        //  return getLineWeightValue(ctx.ConversionOptions.DefaultLineWeight, entity, ctx);
        //}

        //double lineWeightByBlock = 0.1; // block.BlockEntity.LineWeight;
        //if (lineWeightByBlock == LineWeightType.ByLayer) {
        //  lineWeightByBlock = block.BlockEntity.Layer.LineWeight;
        //}
        //if (lineWeightByBlock == LineWeightType.Default || lineWeightByBlock == LineWeightType.ByDIPs) {
        //  return getLineWeightValue(ctx.ConversionOptions.DefaultLineWeight, entity, ctx);
        //}
        //return getLineWeightValue(lineWeightByBlock, entity, ctx);
      case LineWeightType::ByLayer:
        //auto lv = entity->layerId().->lineWeight();
        //if (lv == LineWeightType.Default) {
        //  return getLineWeightValue(ctx.ConversionOptions.DefaultLineWeight, entity, ctx);
        //}
        //return getLineWeightValue(entity->layer().LineWeight, entity, ctx);
      default:
        return getLineWeightValue(lineWeightType, entity, ctx);
      }
    }


    // Convert to millimeters/pixels
  private:

    LineWeightType layerLineWeight(const AcDbObjectId& lrId);

    static double getLineWeightValue(LineWeightType lineweightType, AcDbEntity* entity, ConversionContext ctx) {
      //CadHeader header = entity.Document.Header;
      double scaleFactor = ctx.ConversionOptions.LineweightScaleFactor;
      //if (scaleFactor <= 0) {
      //  scaleFactor = Math.Max(header.ModelSpaceExtMax.X - header.ModelSpaceExtMin.X, header.ModelSpaceExtMax.Y - header.ModelSpaceExtMin.Y) / 2500;
      //}
      return (int)lineweightType * 0.01 * scaleFactor;
    }


    //  public static string LineToDashArray(Entity entity, LineType lineType) {
    //    List<double> result = new List<double>();

    //    LineType lType = lineType;
    //    if (lType.Name == "ByLayer") {
    //      lType = entity.Layer.LineType;
    //    }

    //    if (lType.Segments.Count() <= 0) {
    //      return string.Empty;
    //    }

    //    foreach(LineType.Segment segment in lType.Segments) {
    //      if (segment.Length == 0) {
    //        result.Add(1);
    //      }
    //      else if (segment.Length > 0) {
    //        result.Add(segment.Length);
    //      }
    //      else {
    //        result.Add(Math.Abs(segment.Length));
    //      }
    //    }

    //    while (result.Count % 2 != 2 && result.Count < 4) {
    //      result.Add(result[result.Count - 2]);
    //    }

    //    if (result[result.Count - 1] == 0) {
    //      result.Add(result[result.Count - 2]);
    //    }

    //    StringBuilder sb = new StringBuilder();
    //    foreach(double item in result) {
    //      sb.Append(SvgElementBase.Cd(item)).Append(" ");
    //    }

    //    return sb.ToString().Trim();
    //  }
  };

void pattern2dasharray(const AcDbObjectId& lineTypeId, double scaling, string& out)
{
  AcDbLinetypeTableRecordPointer pLT(lineTypeId, AcDb::kForRead);
  if (pLT.openStatus() == Acad::eOk && pLT->patternLength() > 0) {
    char temp[50];
    out += " stroke-dasharray=\"";
    for (size_t i = 0; i < pLT->numDashes() - 1; i++) {
      out += gcvt(scaling * sqrt(pow(pLT->dashLengthAt(i), 2)), precision, temp);
      out += ",";
    }
    out += gcvt(scaling * sqrt(pow(pLT->dashLengthAt(pLT->numDashes() - 1), 2)), precision, temp);
    out += "\" ";
  }
}

void arc2path(AcDbArc* a, double scaling, string& out) {
  // So far this appears to be the only way to convert arcs into something recognized by SVG
  char temp[20];

  AcGePoint3d startPoint, endPoint;
  a->getStartPoint(startPoint);
  a->getEndPoint(endPoint);

  double ret_x = a->center().x, ret_y = a->center().y;
  double ret_radius = a->radius();
  double ret_srt_ang_rads = a->startAngle(), ret_end_ang_rads = a->endAngle();
  double ret_srt_ang = a->startAngle() * PI / 180, ret_end_ang = a->endAngle() * PI / 180;

  out += "<\n\tpath d=\"M";
  // Calculate the starting point from the center and the start angle.  As far as I can tell the rotation is CCW in the dxf notation and it in degrees
  out += gcvt(scaling * (ret_x + ret_radius * cos(ret_srt_ang_rads)), precision, temp);
  out += " ";
  out += gcvt(-1 * scaling * (ret_y + ret_radius * sin(ret_srt_ang_rads)), precision, temp);
  out += " A ";
  // For arcs there is only one radius
  out += gcvt(scaling * ret_radius, precision, temp);
  out += ",";
  out += gcvt(scaling * ret_radius, precision, temp);

  out += " 0"; // For arc assume no x-axis rotation.  That seems to apply to elipse elements only
  // Determine if it is a large arc
  if ((ret_end_ang > ret_srt_ang) && ((ret_end_ang - ret_srt_ang) > 180)) {
    out += " 1,0 "; // Large arc flag...Always use a zero sweep flag
  }
  else if ((ret_end_ang < ret_srt_ang) && ((360 + ret_end_ang - ret_srt_ang) >= 180)) {
    out += " 1,0 "; // Large arc flag...Always use a zero sweep flag
  }
  else {
    out += " 0,0 "; // Small arc flag...Always use a zero sweep flag
  }

  //The final point
  out += gcvt(scaling * (ret_x + ret_radius * cos(ret_end_ang_rads)), precision, temp);
  out += ",";
  out += gcvt(-1 * scaling * (ret_y + ret_radius * sin(ret_end_ang_rads)), precision, temp);
  //out += "\" fill=\"none\" stroke=\"black\" stroke-width=\"1\" ";
  //ltype linfo = plot_info.ret_ltype(a.ret_ltype_name(), a.ret_layer_name());

  pattern2dasharray(a->linetypeId(), scaling, out);  // Add the linetype information

  out += "/>";
}


static char* sRGB(int r, int g, int b)
{
  static char out[6];
  string output;
  stringstream oss;

  //if (r == 255 && g == 255 && b == 255)
  //  r = g = b = 0;

  if (r < 16)
    oss << 0;
  oss << hex << r;

  if (g < 16)
    oss << 0;
  oss << hex << g;

  if (b < 16)
    oss << 0;
  oss << hex << b;

  output = oss.str();
  for (int i = 0; i < 6; i++)
    out[i] = output[i];

  return out;
}

const char* ColorEntToStrRGB(AcDbEntity* pEnt)
{
  AcCmColor color = pEnt->color();
  Adesk::UInt16 ACIindex = color.colorIndex();
  unsigned rgb = 0;

  enum { cpPrimitive, cpLayer, cpblock, cpSystem };

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
    {
      AcDbObjectPointer<AcDbBlockReference> pBlkRef;
      if (rxOpenObject(pBlkRef, pEnt->ownerId()) == Acad::eOk)
      {
        ACIindex = pBlkRef->color().colorIndex();
        rxCloseObject(pBlkRef);
        rgb = acedGetRGB(ACIindex);
      }
    }
  default:
    break;
  }

  if (iBackgoundColor == Adesk::UInt32(-1))
  {
    if (rgb == 0xFFFFFF)
      rgb = 0x0;
  }
  else
  {
    unsigned m = 0x2a;
    if ((iBackgoundColor & 0xffL) <= m && ((iBackgoundColor & 0xff00L) >> 8) <= m && ((iBackgoundColor & 0xff0000L) >> 16) <= m)
    {
      if ((rgb & 0xffL) <= m && ((rgb & 0xff00L) >> 8) <= m && ((rgb & 0xff0000L) >> 16) <= m)
        rgb = 0xFFFFFF;
    }
    else
    {
      m = 0xFFFFFF - 0x2a;
      if ((((rgb & 0xffL) >= m && ((rgb & 0xff00L) >> 8) >= m && ((rgb & 0xff0000L) >> 16) >= m)))
        rgb = 0x0;
    }
  }

  return sRGB((rgb & 0xffL), (rgb & 0xff00L) >> 8, (rgb & 0xff0000L) >> 16);
}


}; // namespace

