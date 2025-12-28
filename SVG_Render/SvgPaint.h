#pragma once
#include "Color.h
#include "framework.h"

struct SvgPaint {
    enum class Type { None, Solid, Gradient };

    Type type = Type::None;
    Color color = Color(0, 0, 0, 0); // For solid colors
    std::string gradientId;          // For url(#id)

    SvgPaint() : type(Type::None) {}
    SvgPaint(Color c) : type(Type::Solid), color(c) {}
    SvgPaint(std::string id) : type(Type::Gradient), gradientId(id) {}
};