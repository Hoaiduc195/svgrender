#pragma once
#include"SvgElement.h"
#include "framework.h"
#include "Vector2.h"

class Renderer;

class SvgPolygon : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolygon();
	SvgPolygon(const SvgPolygon& other);
	SvgPolygon(vector<Vector2> pts, bool closed);
	~SvgPolygon();
	void accept(Renderer& renderer) const override;

	// Getter
	const vector<Vector2>& getPoints() const;
};

