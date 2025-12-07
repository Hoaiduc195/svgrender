#include"SvgPath.h"
#include"Renderer.h"

SvgPath::SvgPath() : pathData("") {}

SvgPath::SvgPath(const SvgPath& other) : pathData(other.pathData) {}

SvgPath::SvgPath(const string& path) : pathData(path) {}

SvgPath::~SvgPath() {}

SvgPath& SvgPath::operator=(const SvgPath& other) {
	if (this != &other) {
		pathData = other.pathData;
	}
	return *this;
}

void SvgPath::accept(Renderer& renderer) {
	renderer.visit(this);
}