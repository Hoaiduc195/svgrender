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

void SvgGroup::accept(Renderer& renderer) const {
	renderer.render(*this);
}

