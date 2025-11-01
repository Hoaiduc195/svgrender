#pragma once
#include"SvgElement.h"
#include<vector>
#include<string>
#include<gdiplus.h>
using namespace std;
using namespace Gdiplus;

class SvgText : public SvgElement {
private:
	float x, y;
	float rotationAngle;
	string content;
	string textAnchor;
public:
	SvgText();
	SvgText(const SvgText& other);
	SvgText(float _x, float _y, float angle, const string& txt, const string& anchor);
	~SvgText();
	void draw(Graphics& g) override;
};