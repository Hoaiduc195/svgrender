#pragma once
#include "SvgElement.h"
#include <vector>
#include "framework.h"

struct Vector2 {
	float x;
	float y;
	Vector2() : x(0), y(0) {}
	Vector2(float _x, float _y) : x(_x), y(_y) {}
};

class SvgPolyline : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolyline();
	SvgPolyline(const SvgPolyline& other);
	SvgPolyline(vector<Vector2> pts, bool closed);
	~SvgPolyline();
	void draw(Graphics& g) override;
};