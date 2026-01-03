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
	
	// Getter
	const vector<Vector2>& getPoints() const;

	void accept(Renderer& renderer) const override;
	RectF getBoundingBox() const override;
};