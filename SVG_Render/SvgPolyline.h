#pragma once

#include"framework.h"
#include"Resource.h"
#include<vector>
using namespace std;

class SvgPolyline : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolyline();
	~SvgPolyline();
	void draw();
};
