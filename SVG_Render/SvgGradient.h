#pragma once
#include "framework.h"

struct GradientStop {
    float offset;
    Color color;
};

struct LinearGradient {
    std::string id;
    float x1, y1, x2, y2;
    std::vector<GradientStop> stops;
};
