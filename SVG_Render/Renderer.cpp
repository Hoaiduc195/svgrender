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

// -------------------------------------------------------------------------
// Helper: createBrush
// Handles Solid Color, Linear Gradient, and Radial Gradient creation.
// -------------------------------------------------------------------------
Brush* createBrush(const SvgElement& element, const SvgDocument* doc) {
    if (element.getFillType() == FillType::None) return nullptr;
    if (element.getFillType() == FillType::SolidColor) {
        if (element.getFillOpacity() <= 0.0f) return nullptr;
        Color c = element.getFill();
        return new SolidBrush(Color((BYTE)(element.getFillOpacity() * 255), c.GetR(), c.GetG(), c.GetB()));
    }

    // Handle Gradients
    if (element.getFillType() == FillType::Gradient && doc != nullptr) {
        const Gradient* gradBase = doc->getGradient(element.getGradientId());

        if (gradBase) {
            RectF bounds = element.getBoundingBox();

            // Prepare Gradient Transform Matrix (GDI+)
            Matrix gradientTransformMatrix;
            Matrix3x3 m = gradBase->transform.getMatrix();
            gradientTransformMatrix.SetElements(
                (REAL)m.matrix[0][0], (REAL)m.matrix[0][1],
                (REAL)m.matrix[1][0], (REAL)m.matrix[1][1],
                (REAL)m.matrix[0][2], (REAL)m.matrix[1][2]
            );

            // --- LINEAR GRADIENT ---
            if (gradBase->type == GradientType::Linear) {
                const auto* lGrad = static_cast<const LinearGradient*>(gradBase);
                PointF p1, p2;

                if (gradBase->units == GradientUnits::UserSpaceOnUse) {
                    p1 = PointF(lGrad->x1, lGrad->y1);
                    p2 = PointF(lGrad->x2, lGrad->y2);
                }
                else {
                    // ObjectBoundingBox
                    p1 = PointF(bounds.X + lGrad->x1 * bounds.Width, bounds.Y + lGrad->y1 * bounds.Height);
                    p2 = PointF(bounds.X + lGrad->x2 * bounds.Width, bounds.Y + lGrad->y2 * bounds.Height);
                }

                // Protect against zero-area gradient
                if (abs(p1.X - p2.X) < 0.001f && abs(p1.Y - p2.Y) < 0.001f) {
                    p2.X += 0.1f;
                }

                auto* brush = new LinearGradientBrush(p1, p2, Color::Black, Color::White);

                brush->MultiplyTransform(&gradientTransformMatrix);

                // Setup stops
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

                float cx, cy, r, fx, fy;

                if (gradBase->units == GradientUnits::UserSpaceOnUse) {
                    cx = rGrad->cx;
                    cy = rGrad->cy;
                    r = rGrad->r;
                    fx = rGrad->fx;
                    fy = rGrad->fy;
                }
                else {
                    // ObjectBoundingBox
                    float diag = sqrt(bounds.Width * bounds.Width + bounds.Height * bounds.Height);
                    float normR = rGrad->r * (diag / sqrt(2.0f));

                    cx = bounds.X + rGrad->cx * bounds.Width;
                    cy = bounds.Y + rGrad->cy * bounds.Height;
                    r = normR;
                    fx = bounds.X + rGrad->fx * bounds.Width;
                    fy = bounds.Y + rGrad->fy * bounds.Height;
                }

                path.AddEllipse(cx - r, cy - r, r * 2, r * 2);
                auto* brush = new PathGradientBrush(&path);

                brush->SetCenterPoint(PointF(fx, fy));
                brush->MultiplyTransform(&gradientTransformMatrix);

                int count = (int)gradBase->stops.size();
                if (count > 0) {
                    std::vector<Color> colors(count);
                    std::vector<REAL> positions(count);

                    // Invert for GDI+: 0 is boundary, 1 is Center
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
    }
    return new SolidBrush(Color::Black);
}

// -------------------------------------------------------------------------
// Helper: applyTransform
// -------------------------------------------------------------------------
static void applyTransform(Graphics& graphics, const Transform& transform) {
    Matrix3x3 m = transform.getMatrix();
    Matrix gdipMatrix(
        (REAL)m.matrix[0][0], (REAL)m.matrix[0][1],
        (REAL)m.matrix[1][0], (REAL)m.matrix[1][1],
        (REAL)m.matrix[0][2], (REAL)m.matrix[1][2]
    );
    graphics.MultiplyTransform(&gdipMatrix);
}

// -------------------------------------------------------------------------
// Renderer Implementation
// -------------------------------------------------------------------------


void Renderer::render(const SvgRect& r) {
    GraphicsState state = g.Save();
    applyTransform(g, r.getTransform());

    Brush* brush = createBrush(r, doc);
    if (brush) {
        g.FillRectangle(brush, r.getX(), r.getY(), r.getWidth(), r.getHeight());
        delete brush;
    }

    if (r.getStrokeOpacity() > 0 && r.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(r.getStrokeOpacity() * 255),
            r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()),
            r.getStrokeWidth());
        g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }
    g.Restore(state);
}

// (The other render methods for Circle, Ellipse, Path, etc. remain the same structure 
// but ensure they call createBrush(element, doc) and pass 'doc' correctly.)

void Renderer::render(const SvgCircle& c) {
    GraphicsState state = g.Save();
    applyTransform(g, c.getTransform());

    Brush* brush = createBrush(c, doc);
    float cx = c.getCx();
    float cy = c.getCy();
    float rrad = c.getR();

    if (brush) {
        g.FillEllipse(brush, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
        delete brush;
    }

    if (c.getStrokeOpacity() > 0 && c.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(c.getStrokeOpacity() * 255),
            c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()),
            c.getStrokeWidth());
        g.DrawEllipse(&pen, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    }
    g.Restore(state);
}

void Renderer::render(const SvgEllipse& e) {
    GraphicsState state = g.Save();
    applyTransform(g, e.getTransform());

    Brush* brush = createBrush(e, doc);
    float x = e.getCx() - e.getRx();
    float y = e.getCy() - e.getRy();
    float w = e.getRx() * 2;
    float h = e.getRy() * 2;

    if (brush) {
        g.FillEllipse(brush, x, y, w, h);
        delete brush;
    }

    if (e.getStrokeOpacity() > 0 && e.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(e.getStrokeOpacity() * 255),
            e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()),
            e.getStrokeWidth());
        g.DrawEllipse(&pen, x, y, w, h);
    }
    g.Restore(state);
}

void Renderer::render(const SvgLine& l) {
    GraphicsState state = g.Save();
    applyTransform(g, l.getTransform());

    if (l.getStrokeOpacity() > 0 && l.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(l.getStrokeOpacity() * 255),
            l.getStroke().GetR(), l.getStroke().GetG(), l.getStroke().GetB()),
            l.getStrokeWidth());
        g.DrawLine(&pen, l.getX1(), l.getY1(), l.getX2(), l.getY2());
    }
    g.Restore(state);
}

void Renderer::render(const SvgPolygon& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());

    const auto& pts = p.getPoints();
    if (pts.size() < 2) {
        g.Restore(state);
        return;
    }
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);

    Brush* brush = createBrush(p, doc);
    if (brush) {
        g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
        delete brush;
    }

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity() * 255),
            p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()),
            p.getStrokeWidth());
        g.DrawPolygon(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    }
    g.Restore(state);
}

void Renderer::render(const SvgPolyline& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());

    const auto& pts = p.getPoints();
    if (pts.size() < 2) {
        g.Restore(state);
        return;
    }
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);

    Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity() * 255),
        p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()),
        p.getStrokeWidth());
    pen.SetLineJoin(LineJoinMiter);
    pen.SetStartCap(LineCapFlat);
    pen.SetEndCap(LineCapFlat);

    Brush* brush = createBrush(p, doc);
    if (brush) {
        g.FillPolygon(brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
        delete brush;
    }

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        g.DrawLines(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    }
    g.Restore(state);
}

void Renderer::render(const SvgText& t) {
    GraphicsState state = g.Save();
    applyTransform(g, t.getTransform());

    FontFamily fontFamily(L"Times New Roman");
    int fontStyle = FontStyleRegular;
    float fontSize = t.getFontSize();
    std::wstring wContent(t.getContent().begin(), t.getContent().end());

    GraphicsPath path;
    Font tempFont(&fontFamily, fontSize, fontStyle, UnitPixel);

    float yPos = t.getY() - tempFont.GetHeight(&g);
    float xPos = t.getX();

    string textAnchor = t.getTextAnchor();
    if (textAnchor == "middle" || textAnchor == "end") {
        RectF boundingBox;
        StringFormat format;
        g.MeasureString(wContent.c_str(), -1, &tempFont, PointF(0, 0), &format, &boundingBox);
        float textWidth = boundingBox.Width;
        if (textAnchor == "middle") xPos -= textWidth / 2.0f;
        else if (textAnchor == "end") xPos -= textWidth;
    }

    PointF origin(xPos, yPos);
    StringFormat format;
    path.AddString(wContent.c_str(), -1, &fontFamily, fontStyle, fontSize, origin, &format);

    Brush* brush = createBrush(t, doc);
    if (brush) {
        g.FillPath(brush, &path);
        delete brush;
    }

    if (t.getStrokeOpacity() > 0.0f && t.getStrokeWidth() > 0.0f) {
        Color strokeColor(static_cast<BYTE>(t.getStrokeOpacity() * 255),
            t.getStroke().GetR(), t.getStroke().GetG(), t.getStroke().GetB());
        Pen pen(strokeColor, t.getStrokeWidth());
        pen.SetLineJoin(LineJoinRound);
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

    // Track cubic control point for 'S' command reflection
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
            cur = PointF(x, y); start = cur; figureStarted = true;
            lastWasCubic = false;
            cmd = isRelative ? 'l' : 'L';
            break;
        }
        case 'L': {
            float x = parseNum(); float y = parseNum();
            if (isRelative) { x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddLine(cur, PointF(x, y));
            cur = PointF(x, y);
            lastWasCubic = false;
            break;
        }
        case 'H': {
            float x = parseNum();
            if (isRelative) x += cur.X;
            if (figureStarted) path.AddLine(cur, PointF(x, cur.Y));
            cur.X = x;
            lastWasCubic = false;
            break;
        }
        case 'V': {
            float y = parseNum();
            if (isRelative) y += cur.Y;
            if (figureStarted) path.AddLine(cur, PointF(cur.X, y));
            cur.Y = y;
            lastWasCubic = false;
            break;
        }
        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRelative) { x1 += cur.X; y1 += cur.Y; x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddBezier(cur, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            cur = PointF(x, y);
            lastCubicControl = PointF(x2, y2);
            lastWasCubic = true;
            break;
        }
        case 'S': {
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRelative) { x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }

            PointF ctrl1 = cur;
            if (lastWasCubic) {
                ctrl1.X = 2 * cur.X - lastCubicControl.X;
                ctrl1.Y = 2 * cur.Y - lastCubicControl.Y;
            }

            if (figureStarted) path.AddBezier(cur, ctrl1, PointF(x2, y2), PointF(x, y));
            cur = PointF(x, y);
            lastCubicControl = PointF(x2, y2);
            lastWasCubic = true;
            break;
        }
        case 'Z': {
            if (figureStarted) path.CloseFigure();
            cur = start;
            lastWasCubic = false;
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

    if (p.getStrokeOpacity() > 0.f && p.getStrokeWidth() > 0.f) {
        Color color((BYTE)(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB());
        Pen pen(color, p.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }
    g.Restore(state);
}

void Renderer::render(const SvgGroup& grp) {
    GraphicsState state = g.Save();
    applyTransform(g, grp.getTransform());

    const auto& elements = grp.getElements();
    for (const auto& element : elements) {
        if (element) {
            element->accept(*this);
        }
    }
    g.Restore(state);
}