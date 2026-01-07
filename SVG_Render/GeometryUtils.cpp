#include "GeometryUtils.h"

void SetGdiMatrix(const Transform& transform, Matrix& matrix) {
    Matrix3x3 m = transform.getMatrix();
    matrix.SetElements(
        (REAL)m.matrix[0][0], (REAL)m.matrix[1][0],
        (REAL)m.matrix[0][1], (REAL)m.matrix[1][1],
        (REAL)m.matrix[0][2], (REAL)m.matrix[1][2]
    );
}

float AngleFromVector(float ux, float uy) {
    return atan2(uy, ux);
}

void AddArcSegment(GraphicsPath& path, float cx, float cy, float rx, float ry,
    float phi, float theta1, float theta2) {
    float halfDiff = (theta2 - theta1) / 2.0f;
    float kappa = (4.0f / 3.0f) * (1.0f - cos(halfDiff)) / sin(halfDiff);
    float c1 = cos(theta1), s1 = sin(theta1);
    float c2 = cos(theta2), s2 = sin(theta2);
    float cosPhi = cos(phi), sinPhi = sin(phi);

    auto transformPoint = [&](float x, float y) {
        return PointF(cx + cosPhi * x - sinPhi * y, cy + sinPhi * x + cosPhi * y);
        };

    float x_cp1 = rx * (c1 - kappa * s1), y_cp1 = ry * (s1 + kappa * c1);
    PointF cp1 = transformPoint(x_cp1, y_cp1);
    float x_cp2 = rx * (c2 + kappa * s2), y_cp2 = ry * (s2 - kappa * c2);
    PointF cp2 = transformPoint(x_cp2, y_cp2);
    float x_end = rx * c2, y_end = ry * s2;
    PointF end = transformPoint(x_end, y_end);
    float x_start = rx * c1, y_start = ry * s1;
    PointF start = transformPoint(x_start, y_start);
    path.AddBezier(start, cp1, cp2, end);
}

void TraceArc(GraphicsPath& path, float x1, float y1, float rx, float ry,
    float angle, bool largeArc, bool sweep, float x2, float y2) {
    if (rx == 0 || ry == 0 || (x1 == x2 && y1 == y2)) { path.AddLine(x1, y1, x2, y2); return; }
    rx = std::abs(rx); ry = std::abs(ry);
    float phi = angle * (float)M_PI / 180.0f;
    float cosPhi = cos(phi), sinPhi = sin(phi);
    float dx = (x1 - x2) / 2.0f, dy = (y1 - y2) / 2.0f;
    float x1p = cosPhi * dx + sinPhi * dy, y1p = -sinPhi * dx + cosPhi * dy;
    float lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1.0f) { float sqLambda = sqrt(lambda); rx *= sqLambda; ry *= sqLambda; }
    float num = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;
    float den = rx * rx * y1p * y1p + ry * ry * x1p * x1p;
    if (num < 0) num = 0;
    float coef = sqrt(num / den);
    if (largeArc == sweep) coef = -coef;
    float cxp = coef * (rx * y1p / ry), cyp = coef * -(ry * x1p / rx);
    float cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2.0f;
    float cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2.0f;
    float vx = (x1p - cxp) / rx, vy = (y1p - cyp) / ry;
    float startAngle = AngleFromVector(vx, vy);
    float vx2 = (-x1p - cxp) / rx, vy2 = (-y1p - cyp) / ry;
    float dot = vx * vx2 + vy * vy2, cross = vx * vy2 - vy * vx2;
    float sweepAngle = atan2(cross, dot);
    if (sweep && sweepAngle < 0) sweepAngle += 2 * (float)M_PI;
    else if (!sweep && sweepAngle > 0) sweepAngle -= 2 * (float)M_PI;
    int segments = (int)ceil(std::abs(sweepAngle) / (M_PI / 2.0));
    float delta = sweepAngle / segments;
    float currentAngle = startAngle;
    for (int i = 0; i < segments; i++) {
        AddArcSegment(path, cx, cy, rx, ry, phi, currentAngle, currentAngle + delta);
        currentAngle += delta;
    }
}

float getProjectionT(PointF p, PointF v1, PointF v2) {
    float dx = v2.X - v1.X;
    float dy = v2.Y - v1.Y;
    float lenSq = dx * dx + dy * dy;
    if (lenSq < 0.0001f) return 0.0f;
    return ((p.X - v1.X) * dx + (p.Y - v1.Y) * dy) / lenSq;
}