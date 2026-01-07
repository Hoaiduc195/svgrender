#pragma once
#pragma once
#include "framework.h" // Ch?a Gdiplus
#include "Transform.h" // Ch?a class Transform c?a b?n
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

template <typename T>
T clamp(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

void SetGdiMatrix(const Transform& transform, Matrix& matrix);
float AngleFromVector(float ux, float uy);
void AddArcSegment(GraphicsPath& path, float cx, float cy, float rx, float ry,
    float phi, float theta1, float theta2);
void TraceArc(GraphicsPath& path, float x1, float y1, float rx, float ry,
    float angle, bool largeArc, bool sweep, float x2, float y2);
float getProjectionT(PointF p, PointF v1, PointF v2);