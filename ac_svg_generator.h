#pragma once

class svg_gen
{
  bool bUseBackground = true;
  bool bUseLineWidth = false;
  bool bOriginExt = false;
  bool bReverseY = false;
  double x_min, x_max, y_min, y_max;
  string resulting_svg;

public:
  svg_gen();
  void svg_generate(LPCTSTR fileName, const AcDbObjectIdArray& objIds);

private:
  double LX(double x) const;
  double LY(double y) const;
  AcGePoint2d LXY(const AcGePoint2d& pt) const;

  string svgStyleStr(AcDbEntity* pEnt, double* pTextHeigth = nullptr);
  string svgLineWidthEntToStr(AcDbEntity* pEnt);

  string svgPathLineTo(const AcGePoint2d& point) const;
  string svgPathMoveTo(const AcGePoint2d& point) const;
  string svgPathArc(const AcGePoint2d& point, double radius_x, double radius_y, double x_axis_rotation, bool large_arc_flag, bool sweep_flag) const;
  string svgPathArc(AcDbArc* arc) const;


  void svg_add(const AcDbObjectIdArray& objIds);
  void svg_add(const AcDbObjectId& objId);
  void svg_add(AcDbEntity* ent);

  //void polyline(AcDbPolyline* pPolyline);
  void line(AcDbLine* pLine);
  void arc(AcDbArc* pArc);
  void circle(AcDbCircle* pCircle);
  void ellipse(AcDbEllipse* pEllipse);
  void ellipse2(AcDbEllipse* pEllipse);
  void spline(AcDbSpline* pSpline);
  void text(AcDbText* pText);
  //void mtext(AcDbMText* pMText);

  void spline_points_and_knots_degree2(vector<AcGePoint2d> pt, vector<double> k);
  string solve_quadratic_beizer(vector<AcGePoint2d> pt);
};


