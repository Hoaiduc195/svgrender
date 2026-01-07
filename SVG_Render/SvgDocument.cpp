#include "SvgDocument.h"
#include "SvgElement.h"
#include "Renderer.h"

#undef max
#undef min

void SvgDocument::addElement(unique_ptr<SvgElement> element)
{
    elements.push_back(move(element));
}

void SvgDocument::addGradient(unique_ptr<Gradient> grad) {
    if (!grad->id.empty()) {
        gradientPool[grad->id] = move(grad);
    }
}

const Gradient* SvgDocument::getGradient(const string& id) const {
    auto it = gradientPool.find(id);
    if (it != gradientPool.end()) {
        return it->second.get();
    }
    return nullptr;
}

void SvgDocument::draw(Graphics& graphic) {
    Renderer renderer(graphic, this);
    for (auto& element : elements) {
        element->accept(renderer);
    }
}

RectF SvgDocument::getBoundingBox() const {
    REAL minX = std::numeric_limits<REAL>::max();
    REAL minY = std::numeric_limits<REAL>::max();
    REAL maxX = -std::numeric_limits<REAL>::max();
    REAL maxY = -std::numeric_limits<REAL>::max();
    bool hasContent = false;

    for (const auto& element : elements) {
        RectF bounds = element->getBoundingBox();

        if (bounds.Width > 0 && bounds.Height > 0) {
            if (bounds.X < minX) minX = bounds.X;
            if (bounds.Y < minY) minY = bounds.Y;
            if (bounds.X + bounds.Width > maxX) maxX = bounds.X + bounds.Width;
            if (bounds.Y + bounds.Height > maxY) maxY = bounds.Y + bounds.Height;
            hasContent = true;
        }
    }

    if (!hasContent) return RectF(0, 0, 0, 0);
    return RectF(minX, minY, maxX - minX, maxY - minY);
}