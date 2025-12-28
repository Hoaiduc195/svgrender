#include "SvgPolyline.h"
#include "Renderer.h"
#include "framework.h"

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

const vector<Vector2>& SvgPolyline::getPoints() const {
    return points; 
}

RectF SvgPolyline::getBoundingBox() const {
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

void SvgPolyline::accept(Renderer& renderer) const {
    renderer.render(*this);
}