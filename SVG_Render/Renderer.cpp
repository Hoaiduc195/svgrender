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
float AngleFromVector(float ux, float uy) { return atan2(uy, ux); }

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
// Helper function to project a point onto a line segment (v1 to v2) and get the t value
float getProjectionT(PointF p, PointF v1, PointF v2) {
    float dx = v2.X - v1.X;
    float dy = v2.Y - v1.Y;
    float lenSq = dx * dx + dy * dy;
    if (lenSq < 0.0001f) return 0.0f;
    return ((p.X - v1.X) * dx + (p.Y - v1.Y) * dy) / lenSq;
}

Brush* createBrush(const SvgElement& element, const SvgDocument* doc, RectF* overrideBounds = nullptr) {
    if (element.getFillType() == FillType::None) return nullptr;

    // --- SOLID COLOR ---
    if (element.getFillType() == FillType::SolidColor) {
        if (element.getFillOpacity() <= 0.0f) return nullptr;
        Color c = element.getFill();
        return new SolidBrush(Color((BYTE)(element.getFillOpacity() * 255), c.GetR(), c.GetG(), c.GetB()));
    }

    if (doc) {
        const Gradient* gradBase = doc->getGradient(element.getGradientId());
        if (!gradBase) return new SolidBrush(Color::Black);

        RectF bounds;
        if (overrideBounds != nullptr) {
            bounds = *overrideBounds;
        }
        else {
            bounds = element.getBoundingBox();
        }
        Matrix fullMatrix;

        if (gradBase->units == GradientUnits::ObjectBoundingBox) {
            fullMatrix.Translate(bounds.X, bounds.Y);
            fullMatrix.Scale(bounds.Width, bounds.Height);
        }

        Matrix svgTransform;
        SetGdiMatrix(gradBase->transform, svgTransform);
        fullMatrix.Multiply(&svgTransform, MatrixOrderPrepend);

        // Helper to combine stop-opacity with element fill-opacity
        auto getWeightedColor = [&](const Color& c, float stopOpacity) -> Color {
            float currentAlpha = c.GetAlpha() / 255.0f;
            float finalAlpha = currentAlpha * element.getFillOpacity();
            return Color((BYTE)(clamp(finalAlpha, 0.0f, 1.0f) * 255), c.GetR(), c.GetG(), c.GetB());
            };

        // --- LINEAR GRADIENT (With "Pad" Simulation) ---
        if (gradBase->type == GradientType::Linear) {
            const auto* lGrad = static_cast<const LinearGradient*>(gradBase);
            PointF p1(lGrad->x1, lGrad->y1);
            PointF p2(lGrad->x2, lGrad->y2);
            fullMatrix.TransformPoints(&p1);
            fullMatrix.TransformPoints(&p2);

            // 1. Calculate the bounds of the shape in the local space
            PointF corners[4] = {
                {bounds.X, bounds.Y},
                {bounds.X + bounds.Width, bounds.Y},
                {bounds.X, bounds.Y + bounds.Height},
                {bounds.X + bounds.Width, bounds.Y + bounds.Height}
            };

            // 2. Project all corners onto the gradient vector to find the range [minT, maxT]
            float minT = 0.0f;
            float maxT = 1.0f;

            // Check projections to ensure "Pad" covers the whole shape
            for (int i = 0; i < 4; i++) {
                float t = getProjectionT(corners[i], p1, p2);
                if (t < minT) minT = t;
                if (t > maxT) maxT = t;
            }

            // 3. Expand the gradient vector if needed
            PointF start = p1;
            PointF end = p2;

            if (minT < 0.0f || maxT > 1.0f) {
                // Add a small buffer to avoid artifacts at the exact edge
                minT -= 0.01f;
                maxT += 0.01f;

                float dx = p2.X - p1.X;
                float dy = p2.Y - p1.Y;
                start.X = p1.X + dx * minT;
                start.Y = p1.Y + dy * minT;
                end.X = p1.X + dx * maxT;
                end.Y = p1.Y + dy * maxT;
            }

            // Validate length
            if (std::abs(start.X - end.X) < 0.1f && std::abs(start.Y - end.Y) < 0.1f) end.X += 0.1f;

            auto* brush = new LinearGradientBrush(start, end, Color::Black, Color::White);
            brush->SetWrapMode(WrapModeTileFlipXY); // Flip is safe now because we manually padded the colors
            brush->SetGammaCorrection(TRUE); // Optional: Helps with color blending accuracy

            // 4. Re-map the stops to the new expanded range
            std::vector<Color> colors;
            std::vector<REAL> positions;
            float range = maxT - minT;

            if (!gradBase->stops.empty()) {
                // Pad Start (fill from 0.0 to mapped start with first color)
                colors.push_back(getWeightedColor(gradBase->stops[0].color, 1.0f));
                positions.push_back(0.0f);

                for (const auto& stop : gradBase->stops) {
                    float newPos = (stop.offset - minT) / range;
                    newPos = clamp(newPos, 0.0f, 1.0f);
                    colors.push_back(getWeightedColor(stop.color, 1.0f));
                    positions.push_back(newPos);
                }

                // Pad End (fill from mapped end to 1.0 with last color)
                colors.push_back(getWeightedColor(gradBase->stops.back().color, 1.0f));
                positions.push_back(1.0f);

                brush->SetInterpolationColors(colors.data(), positions.data(), (INT)colors.size());
            }

            return brush;
        }

        // --- RADIAL GRADIENT (With "Pad" Simulation) ---
        else if (gradBase->type == GradientType::Radial) {
            const auto* rGrad = static_cast<const RadialGradient*>(gradBase);

            Matrix invGradientTransform;
            SetGdiMatrix(gradBase->transform, invGradientTransform);
            invGradientTransform.Invert();

            PointF unitCorners[4] = { {0,0}, {1,0}, {0,1}, {1,1} };
            if (gradBase->units == GradientUnits::UserSpaceOnUse) {
                unitCorners[0] = PointF(bounds.X, bounds.Y);
                unitCorners[1] = PointF(bounds.X + bounds.Width, bounds.Y);
                unitCorners[2] = PointF(bounds.X, bounds.Y + bounds.Height);
                unitCorners[3] = PointF(bounds.X + bounds.Width, bounds.Y + bounds.Height);
                invGradientTransform.TransformPoints(unitCorners, 4);
            }
            else {
                invGradientTransform.TransformPoints(unitCorners, 4);
            }

            float maxDist = 0;
            PointF center(rGrad->cx, rGrad->cy);
            auto distSq = [](PointF p1, PointF p2) { return pow(p1.X - p2.X, 2) + pow(p1.Y - p2.Y, 2); };

            for (auto& p : unitCorners) {
                float d = (float)sqrt(distSq(p, center));
                if (d > maxDist) maxDist = d;
            }

            float expansion = 1.0f;
            if (rGrad->r > 0.0001f && maxDist > rGrad->r) {
                expansion = maxDist / rGrad->r;
            }
            expansion *= 1.2f; // buffer
            float expRadius = rGrad->r * expansion;

            GraphicsPath path;
            path.AddEllipse(rGrad->cx - expRadius, rGrad->cy - expRadius, 2 * expRadius, 2 * expRadius);
            path.Transform(&fullMatrix);

            auto* brush = new PathGradientBrush(&path);
            brush->SetWrapMode(WrapModeClamp);

            PointF focus(rGrad->fx, rGrad->fy);
            fullMatrix.TransformPoints(&focus);
            brush->SetCenterPoint(focus);
            brush->SetGammaCorrection(TRUE);

            int count = (int)gradBase->stops.size();
            if (count > 0) {
                std::vector<Color> colors;
                std::vector<REAL> positions;

                // Pad Edge (GDI 0.0 = Edge)
                colors.push_back(getWeightedColor(gradBase->stops.back().color, 1.0f));
                positions.push_back(0.0f);

                for (int i = count - 1; i >= 0; --i) {
                    float svgOffset = gradBase->stops[i].offset;
                    float gdiPos = 1.0f - (svgOffset / expansion);
                    if (!positions.empty() && abs(positions.back() - gdiPos) < 0.001f) continue;

                    colors.push_back(getWeightedColor(gradBase->stops[i].color, 1.0f));
                    positions.push_back(gdiPos);
                }

                // Pad Center (GDI 1.0 = Center)
                if (positions.back() < 1.0f) {
                    colors.push_back(getWeightedColor(gradBase->stops[0].color, 1.0f));
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

Renderer::Renderer(Graphics& g, const SvgDocument* document) : g(g), doc(document) {}

void Renderer::render(const SvgRect& r) {
    GraphicsState state = g.Save();
    applyTransform(g, r.getTransform());
    Brush* brush = createBrush(r, doc);
    if (brush) { g.FillRectangle(brush, r.getX(), r.getY(), r.getWidth(), r.getHeight()); delete brush; }


    if (r.getStrokeOpacity() > 0 && r.getStrokeWidth() > 0) {
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


    if (c.getStrokeOpacity() > 0 && c.getStrokeWidth() > 0) {
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


    if (e.getStrokeOpacity() > 0 && e.getStrokeWidth() > 0) {
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

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
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

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
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

    if (t.getStrokeOpacity() > 0 && t.getStrokeWidth() > 0) {
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

            auto parseFlag = [&]() -> bool {
                skip();
                if (i < d.size() && isdigit(d[i])) {
                    bool val = (d[i] == '1');
                    i++;
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
    RectF tightBounds;
    GraphicsPath* clonePath = path.Clone();
    if (clonePath != nullptr) {
        // Flatten: Bien duong cong thanh cac doan thang nho de loai bo diem dieu khien thua
        clonePath->Flatten(NULL, 0.1f);
        // Lay khung bao chuan tu path da lam phang
        clonePath->GetBounds(&tightBounds);
        //xoa ban sao
        delete clonePath;
    }

    Brush* brush = createBrush(p, doc, &tightBounds);
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