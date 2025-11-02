#include "SvgDocument.h"

void SvgDocument::addChild(unique_ptr<SvgElement> child)
{
    children.push_back(move(child));
}

void SvgDocument::draw(Graphics& graphic) {
    for (auto& child : children) {
        child->draw(graphic);
    }
}