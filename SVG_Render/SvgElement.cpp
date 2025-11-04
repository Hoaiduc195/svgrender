#include "SvgElement.h"

SvgElement::SvgElement() {
	fill = Color(0, 255, 0); // Blue by default
	stroke = Color(0, 0, 0);     // Black by default
	strokeWidth = 0.0;
	strokeOpacity = 255;
	fillOpacity = 255;
}

void SvgElement::setFill(const Color& fillColor) {
	fill = fillColor;
}
void SvgElement::setStroke(const Color& strokeColor) {
	stroke = strokeColor;
}
void SvgElement::setStrokeWidth(float width) {
	strokeWidth = width;
}
void SvgElement::setStrokeOpacity(float opacity) {
	strokeOpacity = opacity;
}
void SvgElement::setFillOpacity(float opacity) {
	fillOpacity = opacity;
}