#include "SvgPolyline.h"
#include "Renderer.h"
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

const vector<Vector2>& SvgPolyline::getPoints() const {
    return points; 
}

void SvgPolyline::accept(Renderer& renderer) const {
    renderer.render(*this);
}
