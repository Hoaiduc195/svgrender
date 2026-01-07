#include"SvgText.h"
#include "framework.h"
#include "Renderer.h"

SvgText::~SvgText() {}

SvgText::SvgText() {
	x = 0.0f;
	y = 0.0f;
	content = "";
	fontSize = 12.0f;
	textAnchor = "start";
}

SvgText::SvgText(const SvgText& other) {
	*this = other;
}

SvgText::SvgText(float _x, float _y, float _fontSize, const string& _content) {
	x = _x;
	y = _y;
	fontSize = _fontSize;
	content = _content;
	textAnchor = "start";
}

SvgText::SvgText(float _x, float _y, float _fontSize, const string& _content, const string& _textAnchor) {
	x = _x;
	y = _y;
	fontSize = _fontSize;
	content = _content;
	textAnchor = _textAnchor;
}

void SvgText::accept(Renderer& renderer) const {
    renderer.render(*this);
}


void SvgText::setFontSize(float size) {
    fontSize = size;
}

void SvgText::setTextAnchor(const string& anchor) {
    textAnchor = anchor;
}

float SvgText::getX() const {
	return x; 
}
float SvgText::getY() const { 
	return y; 
}
float SvgText::getFontSize() const {
	return fontSize; 
}
const string& SvgText::getContent() const {
	return content; 
}
const string& SvgText::getTextAnchor() const {
	return textAnchor; 
}
RectF SvgText::getBoundingBox() const {
	if (content.empty()) return RectF(0, 0, 0, 0);
	GraphicsPath path;

	FontFamily fontFamily(L"Times New Roman");

	StringFormat format(StringFormat::GenericTypographic());
	if (textAnchor == "middle") {
		format.SetAlignment(StringAlignmentCenter);
	}
	else if (textAnchor == "end") {
		format.SetAlignment(StringAlignmentFar);
	}
	else {
		format.SetAlignment(StringAlignmentNear);
	}

	std::wstring wContent(content.begin(), content.end());

	path.AddString(
		wContent.c_str(),
		-1,
		&fontFamily,
		FontStyleRegular,
		fontSize,
		PointF(x, y - fontSize),
		&format
	);
	Matrix matrix;
	SetGdiMatrix(getTransform(), matrix);
	path.Transform(&matrix);

	RectF bounds;
	path.GetBounds(&bounds);
	return bounds;
}