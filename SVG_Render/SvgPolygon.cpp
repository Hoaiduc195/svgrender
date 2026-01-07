#include"SvgPolygon.h"
#include "Renderer.h"
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

const vector<Vector2>& SvgPolygon::getPoints() const {
	return points; 
}

RectF SvgPolygon::getBoundingBox() const {
    if (points.empty()) return RectF(0, 0, 0, 0);

    GraphicsPath path;
    std::vector<PointF> gdiPoints;
    for (const auto& pt : points) {
        gdiPoints.emplace_back(pt.x, pt.y);
    }
    path.AddPolygon(gdiPoints.data(), (INT)gdiPoints.size());

    Matrix matrix;
    SetGdiMatrix(getTransform(), matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}

void SvgPolygon::accept(Renderer& renderer) const {
	renderer.render(*this);
}