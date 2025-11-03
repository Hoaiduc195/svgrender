#include <windows.h>
//#include "framework.h"
#include "SvgViewer.h"


int main() {
	SvgViewer viewer;
	viewer.run();
	viewer.loadSvg("../Assets/sample.svg");

	//viewer.render();

	return 0;
}