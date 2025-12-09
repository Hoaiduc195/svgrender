#include "Renderer.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"
#include "Transform.h"

using namespace Gdiplus;

// Helper function to apply transform to graphics context
static void applyTransform(Graphics& graphics, const Transform& transform) {
    Matrix3x3 m = transform.getMatrix();
    // Create GDI+ Matrix from our 3x3 matrix (using only 2x3 affine part)
    Gdiplus::Matrix gdipMatrix(
        m.matrix[0][0], m.matrix[0][1],
        m.matrix[1][0], m.matrix[1][1],
        m.matrix[0][2], m.matrix[1][2]
    );
    graphics.MultiplyTransform(&gdipMatrix);
}

Renderer::Renderer(Graphics& g) : g(g) {}

void Renderer::render(const SvgRect& r) {
    // Save graphics state
    GraphicsState state = g.Save();
    
    // Apply element transform
    applyTransform(g, r.getTransform());
    
    Pen pen(Gdiplus::Color(static_cast<BYTE>(r.getStrokeOpacity()*255), r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()), r.getStrokeWidth());
    SolidBrush brush(Gdiplus::Color(static_cast<BYTE>(r.getFillOpacity()*255), r.getFill().GetR(), r.getFill().GetG(), r.getFill().GetB()));
    g.FillRectangle(&brush, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    
    // Restore graphics state
    g.Restore(state);
}

void Renderer::render(const SvgCircle& c) {
    GraphicsState state = g.Save();
    applyTransform(g, c.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(c.getStrokeOpacity()*255), c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()), c.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(c.getFillOpacity()*255), c.getFill().GetR(), c.getFill().GetG(), c.getFill().GetB()));
    float cx = c.getCx();
    float cy = c.getCy();
    float rrad = c.getR();
    g.FillEllipse(&brush, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    g.DrawEllipse(&pen, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    
    g.Restore(state);
}

void Renderer::render(const SvgEllipse& e) {
    GraphicsState state = g.Save();
    applyTransform(g, e.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(e.getStrokeOpacity()*255), e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()), e.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(e.getFillOpacity()*255), e.getFill().GetR(), e.getFill().GetG(), e.getFill().GetB()));
    g.FillEllipse(&brush, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    g.DrawEllipse(&pen, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    
    g.Restore(state);
}

void Renderer::render(const SvgLine& l) {
    GraphicsState state = g.Save();
    applyTransform(g, l.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(l.getStrokeOpacity()*255), l.getStroke().GetR(), l.getStroke().GetG(), l.getStroke().GetB()), l.getStrokeWidth());
    g.DrawLine(&pen, l.getX1(), l.getY1(), l.getX2(), l.getY2());
    
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

    if (p.getFillOpacity() > 0) {
        SolidBrush brush(Color(static_cast<BYTE>(p.getFillOpacity()*255), p.getFill().GetR(), p.getFill().GetG(), p.getFill().GetB()));
        g.FillPolygon(&brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
    }

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity()*255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
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

    Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity()*255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
    pen.SetLineJoin(LineJoinMiter);
    pen.SetStartCap(LineCapFlat);
    pen.SetEndCap(LineCapFlat);

    if (p.getFillOpacity() > 0.0f) {
        SolidBrush brush(Color(static_cast<BYTE>(p.getFillOpacity()*255), p.getFill().GetR(), p.getFill().GetG(), p.getFill().GetB()));
        g.FillPolygon(&brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
    }

    g.DrawLines(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    
    g.Restore(state);
}

void Renderer::render(const SvgText& t) {
    GraphicsState state = g.Save();
    applyTransform(g, t.getTransform());
    
    StringFormat format;
    FontFamily fontFamily(L"Times New Roman");
    Font font(&fontFamily, t.getFontSize(), FontStyleRegular, UnitPixel);
    SolidBrush brush(Color(static_cast<BYTE>(t.getFillOpacity() * 255), t.getFill().GetR(), t.getFill().GetG(), t.getFill().GetB()));

    RectF layoutRect(t.getX(), t.getY() - font.GetHeight(&g), 1000, font.GetHeight(&g));
    g.DrawString(std::wstring(t.getContent().begin(), t.getContent().end()).c_str(), -1, &font, layoutRect, &format, &brush);
    
    g.Restore(state);
}


void Renderer::render(const SvgPath& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());
    
    // Parse SVG path data and render using GraphicsPath
    const string& pathData = p.getPathData();
    if (pathData.empty()) {
        g.Restore(state);
        return;
    }

    GraphicsPath path;
    PointF current(0, 0);
    PointF controlPoint(0, 0);
    bool figureStarted = false;

    // Simple path parser: supports M (moveto), L (lineto), H (horizontal), V (vertical), Z (closepath), C (cubic bezier)
    size_t i = 0;
    while (i < pathData.size()) {
        while (i < pathData.size() && (isspace(pathData[i]) || pathData[i] == ',')) i++;
        if (i >= pathData.size()) break;

        char cmd = pathData[i];
        bool isRelative = (cmd >= 'a' && cmd <= 'z');
        char absCmd = isRelative ? (char)(cmd - 32) : cmd;

        i++;

        // Parse numbers until next command
        auto parseNumber = [&]() -> float {
            while (i < pathData.size() && (isspace(pathData[i]) || pathData[i] == ',')) i++;
            float num = 0;
            int sign = 1;
            if (i < pathData.size() && pathData[i] == '-') { sign = -1; i++; }
            while (i < pathData.size() && isdigit(pathData[i])) {
                num = num * 10 + (pathData[i] - '0');
                i++;
            }
            if (i < pathData.size() && pathData[i] == '.') {
                i++;
                float decimal = 0.1f;
                while (i < pathData.size() && isdigit(pathData[i])) {
                    num += decimal * (pathData[i] - '0');
                    decimal *= 0.1f;
                    i++;
                }
            }
            return sign * num;
        };

        switch (absCmd) {
        case 'M': {
            // MoveTo
            float x = parseNumber();
            float y = parseNumber();
            if (isRelative) { x += current.X; y += current.Y; }
            current = PointF(x, y);
            if (figureStarted) path.CloseFigure();
            path.StartFigure();
            figureStarted = true;
            break;
        }
        case 'L': {
            // LineTo
            float x = parseNumber();
            float y = parseNumber();
            if (isRelative) { x += current.X; y += current.Y; }
            if (figureStarted) {
                path.AddLine(current, PointF(x, y));
            }
            current = PointF(x, y);
            break;
        }
        case 'H': {
            // Horizontal LineTo
            float x = parseNumber();
            if (isRelative) x += current.X;
            if (figureStarted) {
                path.AddLine(current, PointF(x, current.Y));
            }
            current = PointF(x, current.Y);
            break;
        }
        case 'V': {
            // Vertical LineTo
            float y = parseNumber();
            if (isRelative) y += current.Y;
            if (figureStarted) {
                path.AddLine(current, PointF(current.X, y));
            }
            current = PointF(current.X, y);
            break;
        }
        case 'C': {
            // Cubic Bezier
            float x1 = parseNumber(), y1 = parseNumber();
            float x2 = parseNumber(), y2 = parseNumber();
            float x = parseNumber(), y = parseNumber();
            if (isRelative) {
                x1 += current.X; y1 += current.Y;
                x2 += current.X; y2 += current.Y;
                x += current.X; y += current.Y;
            }
            if (figureStarted) {
                path.AddBezier(current, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            }
            controlPoint = PointF(x2, y2);
            current = PointF(x, y);
            break;
        }
        case 'Z': {
            // ClosePath
            if (figureStarted) path.CloseFigure();
            figureStarted = false;
            break;
        }
        default:
            // Unknown command, skip next number
            parseNumber();
            break;
        }
    }

    if (figureStarted) path.CloseFigure();

    // Render the path
    if (p.getFillOpacity() > 0.0f) {
        SolidBrush brush(Color(static_cast<BYTE>(p.getFillOpacity() * 255), p.getFill().GetR(), p.getFill().GetG(), p.getFill().GetB()));
        g.FillPath(&brush, &path);
    }
    if (p.getStrokeOpacity() > 0.0f && p.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity() * 255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }
    
    g.Restore(state);
}

void Renderer::render(const SvgGroup& g) {
    // Render all child elements in the group
    // Note: Transform is applied by each child element individually
    const auto& elements = g.getElements();
    for (const auto& element : elements) {
        if (element) {
            element->accept(*this);
        }
    }
}