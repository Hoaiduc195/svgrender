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

    float minX = points[0].x;
    float maxX = points[0].x;
    float minY = points[0].y;
    float maxY = points[0].y;

    for (const auto& p : points) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    return RectF(minX, minY, maxX - minX, maxY - minY);
}

void SvgPolygon::accept(Renderer& renderer) const {
	renderer.render(*this);
}