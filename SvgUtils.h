#pragma once

namespace SvgUtils
{

extern int precision;
extern Adesk::UInt32 iBackgoundColor;

double rad2deg(double a);

string iToString(int i);
string fToChars(double d);
string fPairToString(double x, double y);
void addAttribute(string& s, const char* attr, string value);
void addAttribute(string& s, const char* attr, double value);

void pattern2dasharray(const AcDbObjectId& lineTypeId, double scaling, string& out);

const char* ColorEntToStrRGB(AcDbEntity* pEnt);

typedef AcGePoint2d XY;

class PathElement
{
  string path;

public:
  string toString()
  {
    return path;
  }

  void AddMoveAndArc(
    double startPointX, double startPointY, double endPointX, double endPointY, double r,
    bool largeArc, bool sweep)
  {
    AddMoveAndArc(startPointX, startPointY, endPointX, endPointY, 0, r, r, largeArc, sweep);
  }

  /// <summary>
  /// Adds a move clause to the start point and an elliptic arc clause to the d-Attribute
  /// of this path element and returns this element.
  /// </summary>
  /// <param name="startX">X of the start point of the arc.</param>
  /// <param name="startY">Y of the start point of the arc.</param>
  /// <param name="endX">X of the end point of the arc.</param>
  /// <param name="endY">Y of the end point of the arc.</param>
  /// <param name="rx">The length of the major axis of the ellipse.</param>
  /// <param name="ry">The length of the minor axis of the ellipse.</param>
  /// <param name="rot">The rotation of the major axis in degrees.</param>
  /// <param name="largeArc">Indicates if <c>true</c> that the large-arc flag ist set to 1; otherwise, 0.</param>
  /// <param name="sweep">Intcates if <c>true</c> that the sweep flag is set to 1; otherwise, 0.</param>
  /// <returns>This <see cref="PathElement"/>.</returns>
  void AddMoveAndArc(
    double startPointX, double startPointY, double endPointX, double endPointY, double rot,
    double rX, double rY,
    bool largeArc, bool sweep)
  {
    //using namespace SvgUtils;
    //"M mx, my A rx,ry x-axis-rotation large-arc-flag, sweep-flag x,y" />
    path += "M " + fToChars(startPointX) + " " + fToChars(startPointY) +
      " A " + fToChars(rX) + " " + fToChars(rY) + " " + fToChars(rot) + " " +
      (largeArc ? "1" : "0") + " " + (sweep ? "1" : "0") + " " + fToChars(endPointX) + " " + fToChars(endPointY);
  }

  /// <summary>
  /// Adds a line clause to the start point and an arc clause to the d-Attribute
  /// of this path element and returns this element.
  /// </summary>
  /// <param name="startX">X of the start point of the arc.</param>
  /// <param name="startY">Y of the start point of the arc.</param>
  /// <param name="endX">X of the end point of the arc.</param>
  /// <param name="endY">Y of the end point of the arc.</param>
  /// <param name="r">The arc radius.</param>
  /// <param name="largeArc">Indicates if <c>true</c> that the large-arc flag ist set to 1; otherwise, 0.</param>
  /// <param name="sweep">Intcates if <c>true</c> that the sweep flag is set to 1; otherwise, 0.</param>
  /// <returns>This <see cref="PathElement"/>.</returns>
  void AddLineAndArc(double startPointX, double startPointY, double endPointX, double endPointY, double r,
    bool largeArc, bool sweep)
  {
    //using namespace SvgUtils;
    path += "L " + fToChars(startPointX) + " " + fToChars(startPointY) +
      " A " + fToChars(endPointX) + " " + fToChars(endPointY) + " " +
      (largeArc ? "1" : "0") + " " + (sweep ? "1" : "0") + fToChars(endPointX) + " " + fToChars(endPointY);
  }

  /// Adds a line clause to the start point and an elliptic arc clause to the d-Attribute
    /// of this path element and returns this element.
    /// </summary>
    /// <param name="startX">X of the start point of the arc.</param>
    /// <param name="startY">Y of the start point of the arc.</param>
    /// <param name="endX">X of the end point of the arc.</param>
    /// <param name="endY">Y of the end point of the arc.</param>
    /// <param name="rx">The length of the major axis of the ellipse.</param>
    /// <param name="ry">The length of the minor axis of the ellipse.</param>
    /// <param name="rot">The rotation of the major axis in degrees.</param>
    /// <param name="largeArc">Indicates if <c>true</c> that the large-arc flag ist set to 1; otherwise, 0.</param>
    /// <param name="sweep">Intcates if <c>true</c> that the sweep flag is set to 1; otherwise, 0.</param>
    /// <returns>This <see cref="PathElement"/>.</returns>
  void AddLineAndArc(double startPointX, double startPointY, double endPointX, double endPointY, double r,
    double rX, double rY,
    bool largeArc, bool sweep)
  {
  }
};


class Utils {
public:
  static std::string& strReplace(std::string& s, std::string sSub1, std::string sSub2)
  {
    size_t  cur = 0;
    while (true) {
      cur = s.find(sSub1);
      if (cur == wstring::npos)  break;
      s.replace(cur, sSub2.size(), sSub2);
    }
  }

  static std::list<double> VerticesToArray(std::list<XY> list) {
    std::list<double> result;
    for (auto v : list) {
      result.push_back(v.x);
      result.push_back(v.y);
    }
    return result;
  }

  static std::list<double> VerticesToArray(std::list<AcGePoint3d> list) {
    std::list<double> result;
    for (auto v : list) {
      result.push_back(v.x);
      result.push_back(v.y);
    }
    return result;
  }

  static string CleanBlockName(string name) {
    return strReplace(strReplace(name, "_", "__"), " ", "_");
  }

  static string GetObjectType(AcDbEntity* entity) {
    CString cs = entity->desc()->name();
    return string(CT2CA(cs, CP_UTF8));
  }

  //static double GetInfinity(Entity entity) {
  //  return Math.Max(
  //    entity.Document.Header.ModelSpaceExtMax.X - entity.Document.Header.ModelSpaceExtMin.X,
  //    entity.Document.Header.ModelSpaceExtMax.Y - entity.Document.Header.ModelSpaceExtMin.Y);
  //}

  static unsigned long long ReverseBytes(unsigned long long value) {
    return (value & 0x00000000000000FFUL) << 56 | (value & 0x000000000000FF00UL) << 40 |
      (value & 0x0000000000FF0000UL) << 24 | (value & 0x00000000FF000000UL) << 8 |
      (value & 0x000000FF00000000UL) >> 8 | (value & 0x0000FF0000000000UL) >> 24 |
      (value & 0x00FF000000000000UL) >> 40 | (value & 0xFF00000000000000UL) >> 56;
  }


  static void ArcToPath(
    PathElement& path, bool move,
    XY arcCenter, double r,
    double startAngle, double endAngle, bool counterClockWise = true) {

    XY startPoint, endPoint;
    GetArcStartAndEnd(
      arcCenter, startAngle, endAngle, r, counterClockWise,
      startPoint, endPoint);

    bool largeArc = determineLargeArc(startAngle, endAngle);
    bool sweep = counterClockWise;

    if (move) {
      path.AddMoveAndArc(startPoint.x, startPoint.y, endPoint.x, endPoint.y, r, largeArc, sweep);
    }
    else {
      path.AddLineAndArc(startPoint.x, startPoint.y, endPoint.x, endPoint.y, r, largeArc, sweep);
    }
  }

  static void EllipseArcToPath(
    PathElement& path, bool move,
    XY arcCenter, XY majorAxisEndPoint, double minorToMajorRatio,
    double startAngle, double endAngle, bool counterClockWise = true) {

    ////  Major axis vector and length
    ////  Rotation of major axis in degrees
    double rx = majorAxisEndPoint.asVector().length();
    double ry = rx * minorToMajorRatio;
    double rot = atan2(majorAxisEndPoint.y, majorAxisEndPoint.x) * 180.0 / PI;

    XY startPoint, endPoint;
    GetEllipseArcStartAndEnd(arcCenter, majorAxisEndPoint, minorToMajorRatio,
      startAngle, endAngle, counterClockWise,
      startPoint, endPoint);

    double sweepAngle = endAngle - startAngle;
    bool largeArc = determineLargeArc(startAngle, endAngle);
    bool sweep = counterClockWise; // CCW = 1, CW = 0

    if (move) {
      path.AddMoveAndArc(startPoint.x, startPoint.y, endPoint.x, endPoint.y, rx, ry, rot, largeArc, sweep);
    }
    else {
      path.AddLineAndArc(startPoint.x, startPoint.y, endPoint.x, endPoint.y, rx, ry, rot, largeArc, sweep);
    }
  }

  static void GetArcStartAndEnd(
    XY center, double startAngle, double endAngle, double r, bool counterClockWise,
    XY& startPoint, XY& endPoint) {

    auto fa = counterClockWise ? 1 : -1;
    auto sa = fa * startAngle;
    auto ea = fa * endAngle;

    startPoint = XY(
      center.x + r * cos(sa),
      center.y + r * sin(sa));
    endPoint = XY(
      center.x + r * cos(ea),
      center.y + r * sin(ea));
  }

  static void GetEllipseArcStartAndEnd(
    XY arcCenter, XY majorAxisEndPoint, double minorToMajorRatio,
    double startAngle, double endAngle, bool counterClockWise,
    XY& startPoint, XY& endPoint) {

    //  Major-axis vector and length
    //  Evaluate minor-axis vector and length
    double rx = majorAxisEndPoint.asVector().length();
    double ry = rx * minorToMajorRatio;
    AcGeVector2d v_majorNorm = majorAxisEndPoint.asVector().normalize();
    XY majorNorm(v_majorNorm.x, v_majorNorm.y);
    XY minorAxisEndPoint = XY(-majorNorm.y, majorNorm.x) * ry;

    //  Invert minor-axis vector for CW
    if (!counterClockWise) {
      minorAxisEndPoint.asVector() = -minorAxisEndPoint.asVector();
    }

    startPoint = arcCenter +
      cos(startAngle) * majorAxisEndPoint.asVector() +
      sin(startAngle) * minorAxisEndPoint.asVector();

    endPoint = arcCenter +
      cos(endAngle) * majorAxisEndPoint.asVector() +
      sin(endAngle) * minorAxisEndPoint.asVector();
  }

  static bool determineLargeArc(double startAngle, double endAngle) {
    if (startAngle < 0) {
      startAngle += 2 * PI;
    }
    if (endAngle < 0) {
      endAngle += 2 * PI;
    }
    if (startAngle > endAngle) {
      return endAngle - startAngle - 2 * PI > PI;
    }
    else {
      return (endAngle - startAngle) > PI;
    }
  }
};

};
