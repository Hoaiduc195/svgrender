#include "SvgElement.h"

SvgElement::SvgElement() {
    fillType = FillType::SolidColor;
    fill = Color(0, 0, 0); // Default black
    fillOpacity = 1.0f;

    stroke = Color(0, 0, 0);
    strokeWidth = 0.0f;
    strokeOpacity = 1.0f;
}

void SvgElement::setFill(const Color& fillColor) {
    fill = fillColor;
    fillType = FillType::SolidColor;
}

void SvgElement::setFillGradient(const std::string& id) {
    gradientId = id;
    fillType = FillType::Gradient;
}

void SvgElement::setStroke(const Color& strokeColor) {
    stroke = strokeColor;
}

void SvgElement::setStrokeWidth(float width) {
    strokeWidth = width;
}

// Clamp helper just for this file if needed, or use std::clamp
template <typename T>
T clamp_val(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

void SvgElement::setStrokeOpacity(float opacity) {
    strokeOpacity = clamp_val(opacity, 0.0f, 1.0f);
}

void SvgElement::setFillOpacity(float opacity) {
    fillOpacity = clamp_val(opacity, 0.0f, 1.0f);
}

void SvgElement::setTransform(const Transform& t) {
    transform = t;
}

const Color& SvgElement::getFill() const {
    return fill;
}

const std::string& SvgElement::getGradientId() const {
    return gradientId;
}

float SvgElement::getFillOpacity() const {
    return fillOpacity;
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

Transform SvgElement::getTransform() const {
    return transform;
}