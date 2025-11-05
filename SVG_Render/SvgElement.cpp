#include "SvgElement.h"

SvgElement::SvgElement() {
	fill = Color(0, 0, 255); // Blue by default
	stroke = Color(0, 0, 0);     // Black by default
	strokeWidth = 0.0;
	strokeOpacity = 1.0f;
	fillOpacity = 1.0f;
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
	strokeOpacity = clamp(opacity, 0.0f, 1.0f);
}
void SvgElement::setFillOpacity(float opacity) {
	fillOpacity = clamp(opacity, 0.0f, 1.0f);
}