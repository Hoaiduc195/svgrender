#include "SvgGroup.h"
#include "Renderer.h"

void SvgGroup::addElement(unique_ptr<SvgElement> element) {
	if (element) {
		elements.push_back(move(element));
	}
}

const vector<unique_ptr<SvgElement>>& SvgGroup::getElements() const {
	return elements;
}

Gdiplus::RectF SvgGroup::getBoundingBox() const {
    if (elements.empty()) return Gdiplus::RectF(0, 0, 0, 0);

    Gdiplus::RectF unionRect;
    bool first = true;

    for (const auto& el : elements) {
        if (!el) continue;

        Gdiplus::RectF childBox = el->getBoundingBox();

        if (childBox.Width == 0 && childBox.Height == 0) continue;

        if (first) {
            unionRect = childBox;
            first = false;
        }
        else {
            Gdiplus::RectF::Union(unionRect, unionRect, childBox);
        }
    }

    return unionRect;
}
void SvgGroup::accept(Renderer& renderer) const {
	renderer.render(*this);
}

