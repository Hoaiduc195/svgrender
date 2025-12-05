#pragma once
#include "framework.h"
#include "SvgElement.h"

class SvgPath : public SvgElement {
protected:
	string pathData;
public:
	SvgPath();
	SvgPath(const SvgPath& other);
	~SvgPath();
	SvgPath(string& path);
	void accept(Renderer& renderer);
};
