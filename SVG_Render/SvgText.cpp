#include"SvgText.h"
#include <vector>
#include <string>

SvgText::~SvgText() {}

SvgText::SvgText() {
	x = 0.0f;
	y = 0.0f;
	rotationAngle = 0.0f;
	content = "";
	textAnchor = "start";
}

SvgText::SvgText(const SvgText& other) {
	x = other.x;
	y = other.y;
	rotationAngle = other.rotationAngle;
	content = other.content;
	textAnchor = other.textAnchor;
}

SvgText::SvgText(float _x, float _y, float angle, const string& txt, const string& anchor) {
	x = _x;
	y = _y;
	rotationAngle = angle;
	content = txt;
	textAnchor = anchor;
}

void SvgText::draw(Graphics& g) {
	g.SetTextContrast(0);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetCompositingQuality(CompositingQualityHighQuality);
	g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	g.SetPageUnit(UnitPixel);
	
	StringFormat format;
	if (textAnchor == "middle") {
		format.SetAlignment(StringAlignmentCenter);
	}
	else if (textAnchor == "end") {
		format.SetAlignment(StringAlignmentFar);
	}
	else {
		format.SetAlignment(StringAlignmentNear);
	}

	FontFamily fontFamily(L"Arial");
	Font font(&fontFamily, 16, FontStyleRegular, UnitPixel);
	SolidBrush brush(fill);
	g.DrawString(
		std::wstring(content.begin(), content.end()).c_str(),
		-1,
		&font,
		PointF(x, y),
		&format,
		&brush
	);
}