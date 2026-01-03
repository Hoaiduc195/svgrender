#include "SvgPath.h"
#include "framework.h"
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

    // Initialize with extremes
    float minX = (std::numeric_limits<float>::max)();
    float minY = (std::numeric_limits<float>::max)();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    size_t i = 0;
    float curX = 0, curY = 0;
    float startX = 0, startY = 0;

    // Tracking for S command
    float lastCubicX = 0, lastCubicY = 0;
    bool lastWasCubic = false;

    auto skip = [&]() { while (i < pathData.size() && (isspace(pathData[i]) || pathData[i] == ',')) i++; };
    auto parseNum = [&]() -> float {
        // ... (Keep existing parseNum logic) ...
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

    auto addPoint = [&](float x, float y) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        };

    char cmd = 0;
    while (i < pathData.size()) {
        skip();
        if (i >= pathData.size()) break;
        if (isalpha(pathData[i])) { cmd = pathData[i++]; skip(); }
        else if (cmd == 0) { i++; continue; }

        bool isRel = islower(cmd);
        char upperCmd = toupper(cmd);

        switch (upperCmd) {
        case 'M': {
            float x = parseNum(); float y = parseNum();
            if (isRel) { x += curX; y += curY; }
            curX = x; curY = y;
            startX = curX; startY = curY;
            addPoint(x, y);
            lastWasCubic = false;
            cmd = (isRel ? 'l' : 'L');
            break;
        }
        case 'L': {
            float x = parseNum(); float y = parseNum();
            if (isRel) { x += curX; y += curY; }
            curX = x; curY = y;
            addPoint(x, y);
            lastWasCubic = false;
            break;
        }
        case 'H': {
            float x = parseNum();
            if (isRel) x += curX;
            curX = x;
            addPoint(curX, curY);
            lastWasCubic = false;
            break;
        }
        case 'V': {
            float y = parseNum();
            if (isRel) y += curY;
            curY = y;
            addPoint(curX, curY);
            lastWasCubic = false;
            break;
        }
        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRel) {
                x1 += curX; y1 += curY; x2 += curX; y2 += curY; x += curX; y += curY;
            }
            addPoint(x1, y1); addPoint(x2, y2); addPoint(x, y);
            curX = x; curY = y;
            lastCubicX = x2; lastCubicY = y2;
            lastWasCubic = true;
            break;
        }
        case 'S': {
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRel) {
                x2 += curX; y2 += curY; x += curX; y += curY;
            }

            // Calculate reflection for bounds checking
            float cx1 = curX;
            float cy1 = curY;
            if (lastWasCubic) {
                cx1 = 2 * curX - lastCubicX;
                cy1 = 2 * curY - lastCubicY;
            }
            addPoint(cx1, cy1); // Add implied control point
            addPoint(x2, y2);
            addPoint(x, y);

            curX = x; curY = y;
            lastCubicX = x2; lastCubicY = y2;
            lastWasCubic = true;
            break;
        }
        case 'Z': {
            curX = startX; curY = startY;
            lastWasCubic = false;
            break;
        }
        case 'A': {
            parseNum(); parseNum(); parseNum(); // rx, ry, angle
            parseNum(); parseNum(); // flags
            float x = parseNum();
            float y = parseNum();
            if (isRel) { x += curX; y += curY; }

            addPoint(x, y);
            curX = x; curY = y;
            break;
        }
        default:
            i++;
            break;
        }
    }

    if (minX > maxX) return RectF(0, 0, 0, 0);
    return RectF(minX, minY, maxX - minX, maxY - minY);
}