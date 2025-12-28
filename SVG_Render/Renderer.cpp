#include "Renderer.h"
#include "SvgDocument.h"
#include "Gradient.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"
#include "SvgPath.h"
#include "SvgGroup.h"
#include "Transform.h"
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

template <typename T> T clamp(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

void SetGdiMatrix(const Transform& transform, Matrix& matrix) {
    Matrix3x3 m = transform.getMatrix();
    matrix.SetElements(
        (REAL)m.matrix[0][0], (REAL)m.matrix[1][0],
        (REAL)m.matrix[0][1], (REAL)m.matrix[1][1],
        (REAL)m.matrix[0][2], (REAL)m.matrix[1][2]
    );
}

// --- ARC MATH HELPERS ---
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

    // Công thức tính điểm đã fix (chỉ xoay, không nhân thêm rx/ry)
    auto transformPoint = [&](float x, float y) {
        return PointF(
            cx + cosPhi * x - sinPhi * y,
            cy + sinPhi * x + cosPhi * y
        );
        };

    float x_cp1 = rx * (c1 - kappa * s1);
    float y_cp1 = ry * (s1 + kappa * c1);
    PointF cp1 = transformPoint(x_cp1, y_cp1);

    float x_cp2 = rx * (c2 + kappa * s2);
    float y_cp2 = ry * (s2 - kappa * c2);
    PointF cp2 = transformPoint(x_cp2, y_cp2);

    float x_end = rx * c2;
    float y_end = ry * s2;
    PointF end = transformPoint(x_end, y_end);

    PointF lastPoint;
    path.GetLastPoint(&lastPoint);
    path.AddBezier(lastPoint, cp1, cp2, end);
}

void TraceArc(GraphicsPath& path, float x1, float y1, float rx, float ry,
    float angle, bool largeArc, bool sweep, float x2, float y2) {

    if (rx == 0 || ry == 0 || (x1 == x2 && y1 == y2)) {
        path.AddLine(x1, y1, x2, y2);
        return;
    }

    rx = std::abs(rx);
    ry = std::abs(ry);
    float phi = angle * (float)M_PI / 180.0f;
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float dx = (x1 - x2) / 2.0f;
    float dy = (y1 - y2) / 2.0f;
    float x1p = cosPhi * dx + sinPhi * dy;
    float y1p = -sinPhi * dx + cosPhi * dy;

    float lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1.0f) {
        float sqLambda = sqrt(lambda);
        rx *= sqLambda;
        ry *= sqLambda;
    }

    float numerator = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;
    float denominator = rx * rx * y1p * y1p + ry * ry * x1p * x1p;
    if (numerator < 0) numerator = 0;

    float coef = sqrt(numerator / denominator);
    if (largeArc == sweep) coef = -coef;

    float cxp = coef * (rx * y1p / ry);
    float cyp = coef * -(ry * x1p / rx);

    float cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2.0f;
    float cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2.0f;

    float vx = (x1p - cxp) / rx;
    float vy = (y1p - cyp) / ry;
    float startAngle = AngleFromVector(vx, vy);

    float vx2 = (-x1p - cxp) / rx;
    float vy2 = (-y1p - cyp) / ry;

    float dot = vx * vx2 + vy * vy2;
    float cross = vx * vy2 - vy * vx2;
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
// --- END ARC MATH HELPERS ---

Brush* createBrush(const SvgElement& element, const SvgDocument* doc) {
    if (element.getFillType() == FillType::None) return nullptr;
    if (element.getFillType() == FillType::SolidColor) {
        if (element.getFillOpacity() <= 0.0f) return nullptr;
        Color c = element.getFill();
        return new SolidBrush(Color((BYTE)(element.getFillOpacity() * 255), c.GetR(), c.GetG(), c.GetB()));
    }

    if (doc) {
        const Gradient* gradBase = doc->getGradient(element.getGradientId());
        if (!gradBase) return new SolidBrush(Color::Black);

        RectF bounds = element.getBoundingBox();
        Matrix fullMatrix;

        if (gradBase->units == GradientUnits::ObjectBoundingBox) {
            fullMatrix.Translate(bounds.X, bounds.Y);
            fullMatrix.Scale(bounds.Width, bounds.Height);
        }

        Matrix svgTransform;
        SetGdiMatrix(gradBase->transform, svgTransform);
        fullMatrix.Multiply(&svgTransform, MatrixOrderPrepend);

        if (gradBase->type == GradientType::Linear) {
            const auto* lGrad = static_cast<const LinearGradient*>(gradBase);
            PointF p1(lGrad->x1, lGrad->y1);
            PointF p2(lGrad->x2, lGrad->y2);
            fullMatrix.TransformPoints(&p1);
            fullMatrix.TransformPoints(&p2);

            if (std::abs(p1.X - p2.X) < 0.1f && std::abs(p1.Y - p2.Y) < 0.1f) p2.X += 0.1f;

            auto* brush = new LinearGradientBrush(p1, p2, Color::Black, Color::White);
            int count = (int)gradBase->stops.size();
            if (count > 0) {
                std::vector<Color> colors(count);
                std::vector<REAL> positions(count);
                for (int i = 0; i < count; ++i) {
                    colors[i] = gradBase->stops[i].color;
                    positions[i] = static_cast<REAL>(gradBase->stops[i].offset);
                }
                brush->SetInterpolationColors(colors.data(), positions.data(), count);
            }
            return brush;
        }
        else if (gradBase->type == GradientType::Radial) {
            const auto* rGrad = static_cast<const RadialGradient*>(gradBase);
            PointF center(rGrad->cx, rGrad->cy);
            PointF radiusVec(rGrad->cx + rGrad->r, rGrad->cy);

            fullMatrix.TransformPoints(&center);
            fullMatrix.TransformPoints(&radiusVec);
            float effectiveRadius = sqrt(pow(radiusVec.X - center.X, 2) + pow(radiusVec.Y - center.Y, 2));

            PointF corners[4] = {
                PointF(bounds.X, bounds.Y), PointF(bounds.X + bounds.Width, bounds.Y),
                PointF(bounds.X, bounds.Y + bounds.Height), PointF(bounds.X + bounds.Width, bounds.Y + bounds.Height)
            };
            if (gradBase->units == GradientUnits::ObjectBoundingBox) {
                corners[0] = PointF(0, 0); corners[1] = PointF(1, 0);
                corners[2] = PointF(0, 1); corners[3] = PointF(1, 1);
            }
            fullMatrix.TransformPoints(corners, 4);

            float maxDist = 0;
            for (int i = 0; i < 4; i++) {
                float d = sqrt(pow(corners[i].X - center.X, 2) + pow(corners[i].Y - center.Y, 2));
                if (d > maxDist) maxDist = d;
            }

            float scale = 1.0f;
            if (maxDist > effectiveRadius) {
                scale = maxDist / effectiveRadius;
                effectiveRadius = maxDist;
            }

            GraphicsPath path;
            path.AddEllipse(center.X - effectiveRadius, center.Y - effectiveRadius, effectiveRadius * 2, effectiveRadius * 2);
            auto* brush = new PathGradientBrush(&path);

            PointF focus(rGrad->fx, rGrad->fy);
            fullMatrix.TransformPoints(&focus);
            brush->SetCenterPoint(focus);

            int count = (int)gradBase->stops.size();
            if (count > 0) {
                std::vector<Color> colors;
                std::vector<REAL> positions;
                colors.push_back(gradBase->stops.back().color);
                positions.push_back(0.0f);

                for (int i = count - 1; i >= 0; --i) {
                    float svgOffset = gradBase->stops[i].offset;
                    float gdiPos = 1.0f - (svgOffset / scale);
                    if (!positions.empty() && abs(positions.back() - gdiPos) < 0.001f) continue;
                    colors.push_back(gradBase->stops[i].color);
                    positions.push_back(gdiPos);
                }
                if (positions.back() < 1.0f) {
                    colors.push_back(gradBase->stops[0].color);
                    positions.push_back(1.0f);
                }
                brush->SetInterpolationColors(colors.data(), positions.data(), (INT)colors.size());
            }
            return brush;
        }
    }
    return new SolidBrush(Color::Black);
}

static void applyTransform(Graphics& graphics, const Transform& transform) {
    Matrix gdipMatrix;
    SetGdiMatrix(transform, gdipMatrix);
    graphics.MultiplyTransform(&gdipMatrix);
}


// ... [Render methods: Rect, Circle, Ellipse, Line, Polygon, Polyline, Text - GIỮ NGUYÊN] ...
void Renderer::render(const SvgRect& r) {
    GraphicsState state = g.Save();
    applyTransform(g, r.getTransform());
    Brush* brush = createBrush(r, doc);
    if (brush) { g.FillRectangle(brush, r.getX(), r.getY(), r.getWidth(), r.getHeight()); delete brush; }
    if (r.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(r.getStrokeOpacity() * 255), r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()), r.getStrokeWidth());
        g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }
    g.Restore(state);
}
void Renderer::render(const SvgCircle& c) {
    GraphicsState state = g.Save();
    applyTransform(g, c.getTransform());
    Brush* brush = createBrush(c, doc);
    if (brush) { g.FillEllipse(brush, c.getCx() - c.getR(), c.getCy() - c.getR(), c.getR() * 2, c.getR() * 2); delete brush; }
    if (c.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(c.getStrokeOpacity() * 255), c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()), c.getStrokeWidth());
        g.DrawEllipse(&pen, c.getCx() - c.getR(), c.getCy() - c.getR(), c.getR() * 2, c.getR() * 2);
    }
    g.Restore(state);
}
void Renderer::render(const SvgEllipse& e) {
    GraphicsState state = g.Save();
    applyTransform(g, e.getTransform());
    Brush* brush = createBrush(e, doc);
    if (brush) { g.FillEllipse(brush, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2); delete brush; }
    if (e.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(e.getStrokeOpacity() * 255), e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()), e.getStrokeWidth());
        g.DrawEllipse(&pen, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    }
    g.Restore(state);
}
void Renderer::render(const SvgLine& l) {
    GraphicsState state = g.Save();
    applyTransform(g, l.getTransform());
    if (l.getStrokeOpacity() > 0 && l.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(l.getStrokeOpacity() * 255), l.getStroke().GetR(), l.getStroke().GetG(), l.getStroke().GetB()), l.getStrokeWidth());
        g.DrawLine(&pen, l.getX1(), l.getY1(), l.getX2(), l.getY2());
    }
    g.Restore(state);
}
void Renderer::render(const SvgPolygon& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());
    const auto& pts = p.getPoints();
    if (pts.size() < 2) { g.Restore(state); return; }
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);
    Brush* brush = createBrush(p, doc);
    if (brush) { g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size()); delete brush; }
    if (p.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
        g.DrawPolygon(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    }
    g.Restore(state);
}
void Renderer::render(const SvgPolyline& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());
    const auto& pts = p.getPoints();
    if (pts.size() < 2) { g.Restore(state); return; }
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);
    Brush* brush = createBrush(p, doc);
    if (brush) { g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size()); delete brush; }
    if (p.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
        g.DrawLines(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    }
    g.Restore(state);
}
void Renderer::render(const SvgText& t) {
    GraphicsState state = g.Save();
    applyTransform(g, t.getTransform());
    FontFamily fontFamily(L"Times New Roman");
    Font font(&fontFamily, t.getFontSize(), FontStyleRegular, UnitPixel);
    std::wstring wContent(t.getContent().begin(), t.getContent().end());
    GraphicsPath path;
    path.AddString(wContent.c_str(), -1, &fontFamily, FontStyleRegular, t.getFontSize(), PointF(t.getX(), t.getY() - t.getFontSize()), StringFormat::GenericDefault());
    Brush* brush = createBrush(t, doc);
    if (brush) { g.FillPath(brush, &path); delete brush; }
    if (t.getStrokeOpacity() > 0) {
        Pen pen(Color((BYTE)(t.getStrokeOpacity() * 255), t.getStroke().GetR(), t.getStroke().GetG(), t.getStroke().GetB()), t.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }
    g.Restore(state);
}

void Renderer::render(const SvgPath& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());

    const string& d = p.getPathData();
    if (d.empty()) { g.Restore(state); return; }

    GraphicsPath path(FillModeWinding);

    PointF cur(0, 0);
    PointF start(0, 0);
    PointF lastCubicControl(0, 0);
    bool lastWasCubic = false;
    bool figureStarted = false;
    size_t i = 0;
    char cmd = 0;

    auto skip = [&]() { while (i < d.size() && (isspace(d[i]) || d[i] == ',')) i++; };
    auto parseNum = [&]() -> float {
        skip();
        bool neg = false;
        if (i < d.size() && d[i] == '-') { neg = true; i++; }
        float v = 0;
        while (i < d.size() && isdigit(d[i])) { v = v * 10 + (d[i] - '0'); i++; }
        if (i < d.size() && d[i] == '.') {
            i++; float mul = 0.1f;
            while (i < d.size() && isdigit(d[i])) { v += (d[i] - '0') * mul; mul *= 0.1f; i++; }
        }
        return neg ? -v : v;
        };

    while (i < d.size()) {
        skip();
        if (i >= d.size()) break;
        if (isalpha(d[i])) { cmd = d[i++]; skip(); }
        else if (cmd == 0) { i++; continue; }
        bool isRelative = islower(cmd);
        char upperCmd = toupper(cmd);

        switch (upperCmd) {
        case 'M': {
            float x = parseNum(); float y = parseNum();
            if (isRelative) { x += cur.X; y += cur.Y; }
            path.StartFigure();
            cur = PointF(x, y); start = cur; figureStarted = true; lastWasCubic = false;
            cmd = isRelative ? 'l' : 'L';
            break;
        }
        case 'L': {
            float x = parseNum(); float y = parseNum();
            if (isRelative) { x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddLine(cur, PointF(x, y));
            cur = PointF(x, y); lastWasCubic = false;
            break;
        }
        case 'H': {
            float x = parseNum();
            if (isRelative) x += cur.X;
            if (figureStarted) path.AddLine(cur, PointF(x, cur.Y));
            cur.X = x; lastWasCubic = false;
            break;
        }
        case 'V': {
            float y = parseNum();
            if (isRelative) y += cur.Y;
            if (figureStarted) path.AddLine(cur, PointF(cur.X, y));
            cur.Y = y; lastWasCubic = false;
            break;
        }
        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRelative) { x1 += cur.X; y1 += cur.Y; x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddBezier(cur, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            cur = PointF(x, y); lastCubicControl = PointF(x2, y2); lastWasCubic = true;
            break;
        }
        case 'S': {
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRelative) { x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }
            PointF ctrl1 = cur;
            if (lastWasCubic) { ctrl1.X = 2 * cur.X - lastCubicControl.X; ctrl1.Y = 2 * cur.Y - lastCubicControl.Y; }
            if (figureStarted) path.AddBezier(cur, ctrl1, PointF(x2, y2), PointF(x, y));
            cur = PointF(x, y); lastCubicControl = PointF(x2, y2); lastWasCubic = true;
            break;
        }
        case 'A': {
            float rx = parseNum();
            float ry = parseNum();
            float angle = parseNum();

            // FIX: Hàm parse riêng cho Flag để tránh đọc lố sang toạ độ x
            // Flag trong SVG chỉ là 1 ký tự 0 hoặc 1, không cần khoảng trắng tách biệt
            auto parseFlag = [&]() -> bool {
                skip();
                if (i < d.size() && isdigit(d[i])) {
                    bool val = (d[i] == '1');
                    i++; // Chỉ ăn đúng 1 ký tự
                    return val;
                }
                return false;
                };

            bool largeArcFlag = parseFlag();
            bool sweepFlag = parseFlag();

            float x = parseNum();
            float y = parseNum();

            if (isRelative) { x += cur.X; y += cur.Y; }

            if (figureStarted) {
                TraceArc(path, cur.X, cur.Y, rx, ry, angle, largeArcFlag, sweepFlag, x, y);
            }
            cur = PointF(x, y);
            lastWasCubic = false;
            break;
        }
        case 'Z': {
            if (figureStarted) path.CloseFigure();
            cur = start; lastWasCubic = false;
            break;
        }
        default: i++; break;
        }
    }

    Brush* brush = createBrush(p, doc);
    if (brush) {
        g.FillPath(brush, &path);
        delete brush;
    }
    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        Color color((BYTE)(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB());
        Pen pen(color, p.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }
    g.Restore(state);
}

void Renderer::render(const SvgGroup& grp) {
    GraphicsState state = g.Save();
    applyTransform(g, grp.getTransform());
    for (const auto& element : grp.getElements()) {
        if (element) element->accept(*this);
    }
    g.Restore(state);
}