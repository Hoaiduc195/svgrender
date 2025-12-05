#include"SvgText.h"
#include <vector>
#include <string>
#include "Renderer.h"

SvgText::~SvgText() {}

SvgText::SvgText() {
	x = 0.0f;
	y = 0.0f;
	content = "";
	fontSize = 12.0f;
}

SvgText::SvgText(const SvgText& other) {
	*this = other;
}

SvgText::SvgText(float _x, float _y, float _fontSize, const string& _content) {
	x = _x;
	y = _y;
	fontSize = _fontSize;
	content = _content;
}

void SvgText::accept(Renderer& renderer) {
    renderer.render(*this);
}


void SvgText::setFontSize(float size) {
    fontSize = size;
}
