#include"SvgPath.h"
#include"Renderer.h"

SvgPath::SvgPath(const string& path) : pathData(path) {}

SvgPath& SvgPath::operator=(const SvgPath& other) {
	if (this != &other) {
		pathData = other.pathData;
	}
	return *this;
}

const string& SvgPath::getPathData() const {
	return pathData;
}

void SvgPath::accept(Renderer& renderer) {
	renderer.render(*this);
}