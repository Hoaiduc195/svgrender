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

void SvgPolygon::accept(Renderer& renderer) const {
	renderer.render(*this);
}