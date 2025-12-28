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
    if (pathData.empty()) return Gdiplus::RectF(0, 0, 0, 0);

    // FIX: Use parentheses around (max) to stop Windows macro expansion
    float minX = (std::numeric_limits<float>::max)();
    float minY = (std::numeric_limits<float>::max)();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    // Helper to update bounds
    auto addPoint = [&](float x, float y) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        };

    // --- Minimal Parsing Logic to determine bounds ---
    size_t i = 0;
    float curX = 0, curY = 0;
    float startX = 0, startY = 0; // Track start of subpath for 'Z'

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
            startX = curX; startY = curY; // Move command sets new subpath start
            addPoint(x, y);
            cmd = (isRel ? 'l' : 'L');
            break;
        }
        case 'L': {
            float x = parseNum(); float y = parseNum();
            if (isRel) { x += curX; y += curY; }
            curX = x; curY = y;
            addPoint(x, y);
            break;
        }
        case 'H': {
            float x = parseNum();
            if (isRel) x += curX;
            curX = x;
            addPoint(curX, curY);
            break;
        }
        case 'V': {
            float y = parseNum();
            if (isRel) y += curY;
            curY = y;
            addPoint(curX, curY);
            break;
        }
        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();
            if (isRel) {
                x1 += curX; y1 += curY; x2 += curX; y2 += curY; x += curX; y += curY;
            }
            // Add control points to bounds to ensure the curve is contained
            addPoint(x1, y1); addPoint(x2, y2); addPoint(x, y);
            curX = x; curY = y;
            break;
        }
        case 'Z': {
            curX = startX; curY = startY;
            break;
        }
        default:
            // If unknown command, skip slightly to avoid infinite loop
            i++;
            break;
        }
    }

    if (minX > maxX) return Gdiplus::RectF(0, 0, 0, 0); // No points found
    return Gdiplus::RectF(minX, minY, maxX - minX, maxY - minY);
}