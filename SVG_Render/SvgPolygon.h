#pragma once
#include"SvgElement.h"
#include<vector>
#include "framework.h"
#include "Vector2.h"

class SvgPolygon : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolygon();
	SvgPolygon(const SvgPolygon& other);
	SvgPolygon(vector<Vector2> pts, bool closed);
	~SvgPolygon();
	void draw(Graphics& g) override;
	// dtdnnta
};

