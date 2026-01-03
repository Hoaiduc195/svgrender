#include "SvgDocument.h"
#include "SvgElement.h"
#include "Renderer.h"

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