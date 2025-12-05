#include "SvgGroup.h"
#include "Renderer.h"

SvgGroup::SvgGroup() {}

SvgGroup::~SvgGroup() {}

SvgGroup::SvgGroup(const SvgGroup& other) {
	*this = other;
}

void SvgGroup::addElement(unique_ptr<SvgElement> element) {
	if (element) {
		elementsGroup.push_back(element);
	}
}

void SvgGroup::accept(Renderer& renderer) {
	renderer.visit(this);
}