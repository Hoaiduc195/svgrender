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

RectF SvgGroup::getBoundingBox() const {
    RectF combinedBounds(0, 0, 0, 0);
    bool first = true;

    for (const auto& element : elements) {
        RectF childBounds = element->getBoundingBox();

        if (childBounds.Width <= 0 && childBounds.Height <= 0) continue;

        if (first) {
            combinedBounds = childBounds;
            first = false;
        }
        else {
            RectF::Union(combinedBounds, combinedBounds, childBounds);
        }
    }

    if (first) return RectF(0, 0, 0, 0);
    GraphicsPath path;
    path.AddRectangle(combinedBounds);

    Matrix matrix;
    SetGdiMatrix(transform, matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}
void SvgGroup::accept(Renderer& renderer) const {
	renderer.render(*this);
}

