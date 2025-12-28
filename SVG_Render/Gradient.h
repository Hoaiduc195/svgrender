#pragma once
#include <vector>
#include <string>
#include "Color.h"

// Define types for casting later
enum class GradientType {
    Linear,
    Radial
};

struct GradientStop {
    float offset;
    Color color;
};

// Abstract Base Class
struct Gradient {
    std::string id;
    std::vector<GradientStop> stops;
    GradientType type;

    Gradient(GradientType t) : type(t) {}
    virtual ~Gradient() = default;
};

// Linear Gradient
struct LinearGradient : public Gradient {
    float x1, y1, x2, y2;

    LinearGradient() : Gradient(GradientType::Linear), x1(0), y1(0), x2(1), y2(0) {}
};

// Radial Gradient
struct RadialGradient : public Gradient {
    float cx, cy, r;
    float fx, fy; // Focal point

    RadialGradient() : Gradient(GradientType::Radial), cx(0.5f), cy(0.5f), r(0.5f), fx(0.5f), fy(0.5f) {}
};