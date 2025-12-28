#pragma once
#include "framework.h"
#include "SvgElement.h"

class SvgGroup : public SvgElement {
protected:
	vector<unique_ptr<SvgElement>> elements;
public:
	SvgGroup() = default;
	~SvgGroup() = default;

	void addElement(unique_ptr<SvgElement> element);
	void accept(Renderer& renderer) const override;
	
	// Getter for accessing child elements
	const vector<unique_ptr<SvgElement>>& getElements() const;
};
