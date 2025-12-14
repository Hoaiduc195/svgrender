#pragma once
#include "framework.h"
#include <map>
#include <algorithm>

struct GradientStop {
    float offset;
    Color color;
};

struct LinearGradient {
    std::string id;
    float x1, y1, x2, y2;
    std::vector<GradientStop> stops;
};

Color getColorByName(const string& name);