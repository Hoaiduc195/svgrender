#include "SvgDocument.h"
#include "SvgElement.h"
#include "Renderer.h"

void SvgDocument::addElement(unique_ptr<SvgElement> element)
{
    elements.push_back(move(element));
}

void SvgDocument::draw(Graphics& graphic) {
    Renderer renderer(graphic);
    for (auto& element : elements) {
        element->accept(renderer);
    }
}