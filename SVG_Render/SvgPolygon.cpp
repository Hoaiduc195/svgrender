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

	Color strokeColor = getStroke();
	Pen pen(strokeColor, 2.0f);

	Color fillColor = getFill();
	SolidBrush brush(fillColor);

	g.DrawPolygon(&pen, gdiPoints.data(), (int)gdiPoints.size());
	g.FillPolygon(&brush, gdiPoints.data(), (int)gdiPoints.size());
}