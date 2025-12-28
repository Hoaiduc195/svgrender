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

// Helper to safely clamp values
template <typename T>
T clamp(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// FIX: Helper to Populate GDI+ Matrix (Cannot return by value due to private copy constructor)
void SetGdiMatrix(const Transform& transform, Matrix& matrix) {
    Matrix3x3 m = transform.getMatrix();
    // Map User Transform to GDI+ Matrix
    // GDI+ expects: m11, m12, m21, m22, dx, dy
    matrix.SetElements(
        (REAL)m.matrix[0][0], (REAL)m.matrix[1][0],
        (REAL)m.matrix[0][1], (REAL)m.matrix[1][1],
        (REAL)m.matrix[0][2], (REAL)m.matrix[1][2]
    );
}

float distSq(PointF p1, PointF p2) {
    return (p1.X - p2.X) * (p1.X - p2.X) + (p1.Y - p2.Y) * (p1.Y - p2.Y);
}

Brush* createBrush(const SvgElement& element, const SvgDocument* doc) {
    if (element.getFillType() == FillType::None) return nullptr;
    if (element.getFillType() == FillType::SolidColor) {
        if (element.getFillOpacity() <= 0.0f) return nullptr;
        Color c = element.getFill();
        return new SolidBrush(Color((BYTE)(element.getFillOpacity() * 255), c.GetR(), c.GetG(), c.GetB()));
    }

    if (element.getFillType() == FillType::Gradient && doc != nullptr) {
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

        // --- LINEAR GRADIENT ---
        if (gradBase->type == GradientType::Linear) {
            const auto* lGrad = static_cast<const LinearGradient*>(gradBase);
            PointF p1(lGrad->x1, lGrad->y1);
            PointF p2(lGrad->x2, lGrad->y2);

            fullMatrix.TransformPoints(&p1);
            fullMatrix.TransformPoints(&p2);

            if (std::abs(p1.X - p2.X) < 0.1f && std::abs(p1.Y - p2.Y) < 0.1f) p2.X += 0.1f;

            auto* brush = new LinearGradientBrush(p1, p2, Color::Black, Color::White);

            // Standard Linear Gradient setup
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
        // --- RADIAL GRADIENT FIX ---
        else if (gradBase->type == GradientType::Radial) {
            const auto* rGrad = static_cast<const RadialGradient*>(gradBase);

            PointF center(rGrad->cx, rGrad->cy);
            float radius = rGrad->r;

            // Transform center and calculate transformed radius vector
            PointF radiusVec(rGrad->cx + rGrad->r, rGrad->cy);

            // Apply transform to center and radius point
            fullMatrix.TransformPoints(&center);
            fullMatrix.TransformPoints(&radiusVec);

            // Calculate effective radius in screen space (approximation)
            float effectiveRadius = sqrt(pow(radiusVec.X - center.X, 2) + pow(radiusVec.Y - center.Y, 2));

            // LOGIC FIX: Expand radius to cover the object bounding box (simulates "pad")
            // We need the gradient to cover the element bounds in screen space.
            // Since we can't easily get screen bounds here, we check against the 
            // "transformed" element bounds (approximated).

            // Simplified approach: Calculate distance to furthest corner of bounding box
            // Note: This assumes `fullMatrix` maps to screen space for `UserSpace` or from `0..1` to `BBox`.
            // We transform the BBox corners to find the max distance required.

            PointF corners[4] = {
                PointF(bounds.X, bounds.Y),
                PointF(bounds.X + bounds.Width, bounds.Y),
                PointF(bounds.X, bounds.Y + bounds.Height),
                PointF(bounds.X + bounds.Width, bounds.Y + bounds.Height)
            };

            // If ObjectBoundingBox, corners are 0,0 to 1,1 before fullMatrix
            if (gradBase->units == GradientUnits::ObjectBoundingBox) {
                corners[0] = PointF(0, 0); corners[1] = PointF(1, 0);
                corners[2] = PointF(0, 1); corners[3] = PointF(1, 1);
            }
            // If UserSpaceOnUse, corners are already bounds (but we need to apply element transform? 
            // No, createBrush happens before element transform in Render method).
            // Actually, for UserSpaceOnUse, 'fullMatrix' includes gradientTransform. 
            // The brush coordinates align with element coordinates.

            // To ensure coverage, we transform corners using the INVERSE of the gradient matrix?
            // No, simpler: Transform corners using fullMatrix to compare with center.
            fullMatrix.TransformPoints(corners, 4);

            float maxDist = 0;
            for (int i = 0; i < 4; i++) {
                float d = sqrt(pow(corners[i].X - center.X, 2) + pow(corners[i].Y - center.Y, 2));
                if (d > maxDist) maxDist = d;
            }

            // Scaling factor if we need to expand
            float scale = 1.0f;
            if (maxDist > effectiveRadius) {
                scale = maxDist / effectiveRadius;
                effectiveRadius = maxDist; // Expand the brush radius
            }

            GraphicsPath path;
            path.AddEllipse(
                center.X - effectiveRadius,
                center.Y - effectiveRadius,
                effectiveRadius * 2,
                effectiveRadius * 2
            );

            auto* brush = new PathGradientBrush(&path);

            // Transform focal point
            PointF focus(rGrad->fx, rGrad->fy);
            fullMatrix.TransformPoints(&focus);
            brush->SetCenterPoint(focus);

            // Stops Setup (remapped for expansion)
            int count = (int)gradBase->stops.size();
            if (count > 0) {
                std::vector<Color> colors;
                std::vector<REAL> positions;

                // Add Original Stops (scaled)
                // GDI+: 0=Boundary, 1=Center. SVG: 0=Center, 1=Boundary.
                // We map SVG 0..1 to GDI 1..0
                // With expansion: SVG 1.0 is now at position (1/scale) in GDI logic.

                // SVG Offset X maps to distance X*OldRadius.
                // New normalized distance = (X*OldRadius) / NewRadius = X / scale.

                // Add pad stop (fill the rest with last color)
                // Boundary (0.0 in GDI) corresponds to NewRadius.
                // The Original Edge (OldRadius) is at 1.0 - (1.0/scale).

                // We construct GDI+ positions (0..1)
                // We iterate SVG stops (0..1)
                // SVG Stop S maps to distance S*oldR.
                // Norm Dist D = (S*oldR)/newR = S/scale.
                // GDI Pos = 1.0 - D = 1.0 - (S/scale).

                // First, add the "pad" area (from boundary to original edge)
                // This corresponds to GDI positions 0.0 to (1.0 - 1.0/scale).
                colors.push_back(gradBase->stops.back().color);
                positions.push_back(0.0f); // Boundary (furthest point)

                for (int i = count - 1; i >= 0; --i) {
                    float svgOffset = gradBase->stops[i].offset; // 0..1
                    float gdiPos = 1.0f - (svgOffset / scale);

                    // Avoid duplicate positions which crash GDI+
                    if (!positions.empty() && abs(positions.back() - gdiPos) < 0.001f) continue;

                    colors.push_back(gradBase->stops[i].color);
                    positions.push_back(gdiPos);
                }

                // Ensure the center (1.0) is covered
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

// FIX: Helper apply transform using the new SetGdiMatrix
static void applyTransform(Graphics& graphics, const Transform& transform) {
    Matrix gdipMatrix;
    SetGdiMatrix(transform, gdipMatrix);
    graphics.MultiplyTransform(&gdipMatrix);
}

// Render Implementations
void Renderer::render(const SvgRect& r) {
    GraphicsState state = g.Save();
    applyTransform(g, r.getTransform());

    Brush* brush = createBrush(r, doc);
    if (brush) {
        g.FillRectangle(brush, r.getX(), r.getY(), r.getWidth(), r.getHeight());
        delete brush;
    }
    if (r.getStrokeOpacity() > 0 && r.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(r.getStrokeOpacity() * 255), r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()), r.getStrokeWidth());
        g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }
    g.Restore(state);
}

void Renderer::render(const SvgCircle& c) {
    GraphicsState state = g.Save();
    applyTransform(g, c.getTransform());

    Brush* brush = createBrush(c, doc);
    float cx = c.getCx(); float cy = c.getCy(); float r = c.getR();
    if (brush) {
        g.FillEllipse(brush, cx - r, cy - r, r * 2, r * 2);
        delete brush;
    }
    if (c.getStrokeOpacity() > 0 && c.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(c.getStrokeOpacity() * 255), c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()), c.getStrokeWidth());
        g.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
    }
    g.Restore(state);
}

void Renderer::render(const SvgEllipse& e) {
    GraphicsState state = g.Save();
    applyTransform(g, e.getTransform());

    Brush* brush = createBrush(e, doc);
    float x = e.getCx() - e.getRx(); float y = e.getCy() - e.getRy(); float w = e.getRx() * 2; float h = e.getRy() * 2;
    if (brush) {
        g.FillEllipse(brush, x, y, w, h);
        delete brush;
    }
    if (e.getStrokeOpacity() > 0 && e.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(e.getStrokeOpacity() * 255), e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()), e.getStrokeWidth());
        g.DrawEllipse(&pen, x, y, w, h);
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
    if (brush) {
        g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size());
        delete brush;
    }
    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
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

    Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());

    Brush* brush = createBrush(p, doc);
    if (brush) {
        g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size());
        delete brush;
    }
    g.DrawLines(&pen, gdiPoints.data(), (INT)gdiPoints.size());
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
        Pen pen(Color(static_cast<BYTE>(t.getStrokeOpacity() * 255), t.getStroke().GetR(), t.getStroke().GetG(), t.getStroke().GetB()), t.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }
    g.Restore(state);
}

void Renderer::render(const SvgPath& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());

    const string& d = p.getPathData();
    if (d.empty()) { g.Restore(state); return; }

    GraphicsPath path;
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