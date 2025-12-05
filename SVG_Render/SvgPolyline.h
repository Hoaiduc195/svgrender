#pragma once
#include "SvgElement.h"
#include "framework.h"
#include "Vector2.h"

class Renderer;

class SvgPolyline : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolyline();
	SvgPolyline(const SvgPolyline& other);
	SvgPolyline(vector<Vector2> pts, bool closed);
	~SvgPolyline();
	void accept(Renderer& renderer) override;

	const vector<Vector2>& getPoints() const { return points; }
};