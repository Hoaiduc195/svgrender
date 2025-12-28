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

        // 1. Calculate the Full Transformation Matrix
        Matrix fullMatrix; // Identity by default

        if (gradBase->units == GradientUnits::ObjectBoundingBox) {
            fullMatrix.Translate(bounds.X, bounds.Y);
            fullMatrix.Scale(bounds.Width, bounds.Height);
        }

        // Apply SVG gradientTransform
        Matrix svgTransform;
        SetGdiMatrix(gradBase->transform, svgTransform); // FIX: Populate local matrix
        fullMatrix.Multiply(&svgTransform, MatrixOrderPrepend);

        // --- LINEAR GRADIENT ---
        if (gradBase->type == GradientType::Linear) {
            const auto* lGrad = static_cast<const LinearGradient*>(gradBase);
            PointF p1(lGrad->x1, lGrad->y1);
            PointF p2(lGrad->x2, lGrad->y2);

            // Transform the points directly to screen space
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
        // --- RADIAL GRADIENT ---
        else if (gradBase->type == GradientType::Radial) {
            const auto* rGrad = static_cast<const RadialGradient*>(gradBase);

            GraphicsPath path;
            path.AddEllipse(
                rGrad->cx - rGrad->r,
                rGrad->cy - rGrad->r,
                rGrad->r * 2,
                rGrad->r * 2
            );

            // Transform the path to screen space so the brush bounds are correct
            path.Transform(&fullMatrix);

            auto* brush = new PathGradientBrush(&path);

            PointF center(rGrad->fx, rGrad->fy);
            fullMatrix.TransformPoints(&center);
            brush->SetCenterPoint(center);

            int count = (int)gradBase->stops.size();
            if (count > 0) {
                std::vector<Color> colors(count);
                std::vector<REAL> positions(count);
                for (int i = 0; i < count; ++i) {
                    colors[i] = gradBase->stops[i].color;
                    positions[i] = 1.0f - static_cast<REAL>(gradBase->stops[i].offset);
                }
                std::reverse(colors.begin(), colors.end());
                std::reverse(positions.begin(), positions.end());
                brush->SetInterpolationColors(colors.data(), positions.data(), count);
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