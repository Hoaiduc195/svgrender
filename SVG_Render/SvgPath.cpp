#include "SvgPath.h"
#include "Renderer.h"

// Fix for Windows min/max macro conflict
#undef max
#undef min

SvgPath::SvgPath(const string& d) : pathData(d) {
    fillType = FillType::SolidColor; // Default
}

const string& SvgPath::getPathData() const {
    return pathData;
}

void SvgPath::accept(Renderer& renderer) const {
    renderer.render(*this);
}

Gdiplus::RectF SvgPath::getBoundingBox() const {
    if (pathData.empty()) return RectF(0, 0, 0, 0);

    GraphicsPath path(FillModeWinding);

    size_t i = 0;
    PointF cur(0, 0), start(0, 0);
    PointF lastCubicControl(0, 0), lastQuadraticControl(0, 0);
    bool lastWasCubic = false, lastWasQuadratic = false, figureStarted = false;
    char cmd = 0;

    auto skip = [&]() { while (i < pathData.size() && (isspace(pathData[i]) || pathData[i] == ',')) i++; };
    auto parseNum = [&]() -> float {
        skip();
        bool neg = false;
        if (i < pathData.size() && pathData[i] == '-') { neg = true; i++; }
        float v = 0;
        while (i < pathData.size() && isdigit(pathData[i])) { v = v * 10 + (pathData[i] - '0'); i++; }
        if (i < pathData.size() && pathData[i] == '.') {
            i++; float mul = 0.1f;
            while (i < pathData.size() && isdigit(pathData[i])) { v += (pathData[i] - '0') * mul; mul *= 0.1f; i++; }
        }
        return neg ? -v : v;
        };

    while (i < pathData.size()) {
        skip();
        if (i >= pathData.size()) break;
        if (isalpha(pathData[i])) { cmd = pathData[i++]; skip(); }
        else if (cmd == 0) { i++; continue; }
        bool isRel = islower(cmd);
        char upperCmd = toupper(cmd);

        switch (upperCmd) {
        case 'M': {
            float x = parseNum(), y = parseNum();
            if (isRel) { x += cur.X; y += cur.Y; }
            path.StartFigure();
            cur = PointF(x, y); start = cur; figureStarted = true; lastWasCubic = false;
            cmd = isRel ? 'l' : 'L';
            break;
        }
        case 'L': {
            float x = parseNum(), y = parseNum();
            if (isRel) { x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddLine(cur, PointF(x, y));
            cur = PointF(x, y); lastWasCubic = false;
            break;
        }
        case 'H': {
            float x = parseNum();
            if (isRel) x += cur.X;
            if (figureStarted) path.AddLine(cur, PointF(x, cur.Y));
            cur.X = x; lastWasCubic = false;
            break;
        }
        case 'V': {
            float y = parseNum();
            if (isRel) y += cur.Y;
            if (figureStarted) path.AddLine(cur, PointF(cur.X, y));
            cur.Y = y; lastWasCubic = false;
            break;
        }
        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRel) { x1 += cur.X; y1 += cur.Y; x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }
            if (figureStarted) path.AddBezier(cur, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            cur = PointF(x, y); lastCubicControl = PointF(x2, y2); lastWasCubic = true;
            break;
        }
        case 'S': {
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRel) { x2 += cur.X; y2 += cur.Y; x += cur.X; y += cur.Y; }
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
        case 'A': {
            float rx = parseNum(), ry = parseNum(), angle = parseNum();
            bool largeArc = (parseNum() != 0);
            bool sweep = (parseNum() != 0);
            float x = parseNum(), y = parseNum();
            if (isRel) { x += cur.X; y += cur.Y; }
            if (figureStarted) {
                TraceArc(path, cur.X, cur.Y, rx, ry, angle, largeArc, sweep, x, y);
            }
            cur = PointF(x, y); lastWasCubic = false;
            break;
        }
        default: i++; break;
        }
    }

    Matrix matrix;
    SetGdiMatrix(getTransform(), matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}