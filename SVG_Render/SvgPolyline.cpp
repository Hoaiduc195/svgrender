#include "SvgPolyline.h"
#include <vector>

SvgPolyline::~SvgPolyline() {}

SvgPolyline::SvgPolyline() {
	isClosed = false;
}

SvgPolyline::SvgPolyline(const SvgPolyline& other) {
	points = other.points;
	isClosed = other.isClosed;
}

SvgPolyline::SvgPolyline(vector<Vector2> pts, bool closed) {
	points = pts;
	isClosed = closed;
}

void SvgPolyline::draw(Graphics& g) {
	if (points.size() < 2) return;

	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetCompositingQuality(CompositingQualityHighQuality);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	g.SetPageUnit(UnitPixel);
	g.SetPageScale(1.0f);

	std::vector<PointF> gdiPoints;
	for (const auto& v : points) {
		gdiPoints.emplace_back(v.x, v.y);
	}

	Pen pen(Color(static_cast<BYTE>(strokeOpacity*255), stroke.GetR(), stroke.GetG(), stroke.GetB()), strokeWidth);
	
	if (isClosed) {
		g.DrawPolygon(&pen, gdiPoints.data(), static_cast<INT>(gdiPoints.size()));
	} else {
		g.DrawLines(&pen, gdiPoints.data(), static_cast<INT>(gdiPoints.size()));
	}
}