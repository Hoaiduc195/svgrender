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

    GraphicsPath path;

    std::vector<PointF> gdiPoints;
    gdiPoints.reserve(points.size());
    for (const auto& p : points) {
        gdiPoints.emplace_back(p.x, p.y);
    }

    if (isClosed) {
        path.AddPolygon(gdiPoints.data(), (INT)gdiPoints.size());
    }
    else {
        path.AddLines(gdiPoints.data(), (INT)gdiPoints.size());
    }

    Matrix matrix;
    SetGdiMatrix(getTransform(), matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}

void SvgPolyline::accept(Renderer& renderer) const {
    renderer.render(*this);
}