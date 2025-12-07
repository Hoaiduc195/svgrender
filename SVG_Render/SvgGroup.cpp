#include "SvgGroup.h"
#include "Renderer.h"

SvgGroup::SvgGroup() {}

SvgGroup::~SvgGroup() {}

SvgGroup::SvgGroup(const SvgGroup& other) {
	elements.clear();
	for (const auto& elem : other.elements) {
		elements.push_back(elem->clone());
	}
}

SvgGroup& SvgGroup::operator=(const SvgGroup& other) {
	if (this != &other) {
		elements.clear();
		for (const auto& elem : other.elements) {
			elements.push_back(elem->clone());
		}
	}
	return *this;
}

void SvgGroup::addElement(unique_ptr<SvgElement> element) {
	if (element) {
		elements.push_back(move(element));
	}
}

void SvgGroup::accept(Renderer& renderer) {
	renderer.visit(this);
}