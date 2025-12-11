#pragma once
#include "framework.h"
#include "SvgElement.h"

class SvgPath : public SvgElement {
protected:
	string pathData;
public:
	SvgPath() = default;
	SvgPath(const SvgPath& other) = default;
	~SvgPath() = default;
	SvgPath(const string& path);
	SvgPath& operator=(const SvgPath& other);
	void accept(Renderer& renderer);
	
	// Getter 
	const string& getPathData() const;
};
