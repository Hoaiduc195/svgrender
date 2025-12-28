#pragma once
#include "framework.h"
#include "SvgElement.h"

class Renderer;

class SvgPath : public SvgElement {
protected:
	string pathData;
public:
	SvgPath() = default;
	SvgPath(const SvgPath& other) = default;
	~SvgPath() = default;
	SvgPath(const string& path);
	SvgPath& operator=(const SvgPath& other) = default;
	void accept(Renderer& renderer) const override;
	
	// Getter 
	const string& getPathData() const;
	RectF getBoundingBox() const override;
};
