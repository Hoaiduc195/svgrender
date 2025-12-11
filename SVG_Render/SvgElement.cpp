#include "SvgElement.h"

SvgElement::SvgElement() {
	fill = Color(0, 0, 0, 0);
	stroke = Color(255, 255, 255, 255);
	strokeWidth = 1.0;
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
void SvgElement::setTransform(const Transform& t) {
	transform = t;
}

const Color& SvgElement::getFill() const {
	return fill; 
}
const Color& SvgElement::getStroke() const {
	return stroke; 
}
float SvgElement::getStrokeWidth() const {
	return strokeWidth; 
}
float SvgElement::getStrokeOpacity() const {
	return strokeOpacity; 
}
float SvgElement::getFillOpacity() const {
	return fillOpacity; 
}
Transform SvgElement::getTransform() const {
	return transform; 
}