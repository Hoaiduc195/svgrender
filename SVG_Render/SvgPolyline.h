#pragma once
#include "SvgElement.h"
#include <vector>

using namespace tinyxml2;
using namespace std;

class SvgPolyline : public SvgElement {
private:
	vector<Vector2> points;
	bool isClosed;
public:
	SvgPolyline();
	~SvgPolyline();
};