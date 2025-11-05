#pragma once
#include "SvgElement.h"
#include <vector>
#include "framework.h"
#include "Vector2.h"

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