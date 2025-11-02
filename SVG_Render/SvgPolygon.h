#pragma once
#include"SvgElement.h"
#include<vector>
using namespace std;

struct Vector2 {
	float x;
	float y;
	Vector2() : x(0), y(0) {}
	Vector2(float _x, float _y) : x(_x), y(_y) {}
};

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

};
