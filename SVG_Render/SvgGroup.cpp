#include "SvgGroup.h"
#include "Renderer.h"

SvgGroup::SvgGroup() {}

SvgGroup::~SvgGroup() {}

void SvgGroup::addElement(unique_ptr<SvgElement> element) {
	if (element) {
		elements.push_back(move(element));
	}
}

void SvgGroup::accept(Renderer& renderer) {
	// TODO: Implement group rendering logic
}