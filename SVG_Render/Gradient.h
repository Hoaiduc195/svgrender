#pragma once
#include <vector>
#include <string>
#include "Color.h"
#include "Transform.h" // Add this include

enum class GradientType {
    Linear,
    Radial
};

enum class GradientUnits {
    ObjectBoundingBox, // Default: coordinates are 0..1 relative to the shape
    UserSpaceOnUse     // Coordinates are absolute pixels
};

struct GradientStop {
    float offset;
    Color color;
};

struct Gradient {
    std::string id;
    std::vector<GradientStop> stops;
    GradientType type;
    GradientUnits units;
    Transform transform;

    Gradient(GradientType t)
        : type(t), units(GradientUnits::ObjectBoundingBox) {
    } // Default per SVG spec
    virtual ~Gradient() = default;
};

struct LinearGradient : public Gradient {
    float x1, y1, x2, y2;

    LinearGradient() : Gradient(GradientType::Linear), x1(0), y1(0), x2(1), y2(0) {}
};

struct RadialGradient : public Gradient {
    float cx, cy, r;
    float fx, fy;

    RadialGradient() : Gradient(GradientType::Radial), cx(0.5f), cy(0.5f), r(0.5f), fx(0.5f), fy(0.5f) {}
};