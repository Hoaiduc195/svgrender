#pragma once
#include "framework.h"
#include "SvgElement.h"

class SvgGroup : public SvgElement {
protected:
	vector<unique_ptr<SvgElement>> elementsGroup;
public:
	SvgGroup();
	~SvgGroup();
	SvgGroup(const SvgGroup& other);
	void addElement(unique_ptr<SvgElement> element);
	void accept(Renderer& renderer);
};
