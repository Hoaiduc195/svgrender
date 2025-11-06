#include"SvgPolygon.h"
#include <vector>

SvgPolygon::~SvgPolygon() {}

SvgPolygon::SvgPolygon() {
	isClosed = true;
}

SvgPolygon::SvgPolygon(const SvgPolygon& other) {
	points = other.points;
	isClosed = other.isClosed;
}

SvgPolygon::SvgPolygon(vector<Vector2> pts, bool closed) {
	points = pts;
	isClosed = closed;
}

void SvgPolygon::draw(Graphics& g)  {
	if (points.size() < 2) return;
	
	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetCompositingQuality(CompositingQualityHighQuality);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	g.SetPageUnit(UnitPixel);
	g.SetPageScale(1.0f);
    
	vector<Gdiplus::PointF> gdiPoints;
	for (const auto& v : points) {
		gdiPoints.emplace_back(v.x, v.y);
	}
	/*Pen pen(Color(static_cast<BYTE>(strokeOpacity * 255), stroke.GetR(), stroke.GetG(), stroke.GetB()), strokeWidth);
	SolidBrush brush(Color(static_cast<BYTE>(fillOpacity*255), fill.GetR(), fill.GetG(), fill.GetB()));

	g.FillPolygon(&brush, gdiPoints.data(), (int)gdiPoints.size(), FillModeAlternate);
	g.DrawPolygon(&pen, gdiPoints.data(), (int)gdiPoints.size());*/

	if (fillOpacity > 0) {
		SolidBrush brush(Color(static_cast<BYTE>(fillOpacity * 255),
			fill.GetR(), fill.GetG(), fill.GetB()));
		g.FillPolygon(&brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
	}

	if (strokeOpacity > 0 && strokeWidth > 0) {
		Pen pen(Color(static_cast<BYTE>(strokeOpacity * 255),
			stroke.GetR(), stroke.GetG(), stroke.GetB()), strokeWidth);
		g.DrawPolygon(&pen, gdiPoints.data(), (INT)gdiPoints.size());
	}
	
}