#include "SvgDocument.h"

void SvgDocument::addElement(unique_ptr<SvgElement> element)
{
    elements.push_back(move(element));
}

void SvgDocument::draw(Graphics& graphic) {
    for (auto& element : elements) {
        element->draw(graphic);
    }
}