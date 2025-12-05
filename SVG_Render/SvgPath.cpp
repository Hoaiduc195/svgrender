#include"SvgPath.h"
#include"Renderer.h"

SvgPath::SvgPath() {
	pathData = "";
}

SvgPath::SvgPath(const SvgPath& other) {
	*this = other;
}

SvgPath::SvgPath(string& path) {
	pathData = path;
}

SvgPath::~SvgPath() {}

void SvgPath::accept(Renderer& renderer) {
	renderer.visit(this);
}