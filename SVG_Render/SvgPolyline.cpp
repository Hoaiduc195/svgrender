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

	std::vector<Gdiplus::PointF> gdiPoints;
	for (const auto& v : points) {
		gdiPoints.emplace_back(v.x, v.y);
	}

	Pen pen(Color::MakeARGB(255, 0, 0, 255), 2.0f);

	for (size_t i = 0; i < gdiPoints.size() - 1; ++i) {
		PointF p1 = gdiPoints[i];
		PointF p2 = gdiPoints[i + 1];
		g.DrawLine(&pen, p1, p2);
	}

	if (isClosed == true) {
		PointF first = gdiPoints.front();
		PointF last = gdiPoints.back();
		g.DrawLine(&pen, first, last);
	}
}