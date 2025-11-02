#include<iostream>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

class SvgElement {
    private:
        string id;
        Color fill;
        Color stroke;
        float strokeWidth;
        float strokeOpacity;
        float fillOpacity;
    public:
        SvgElement() {
            //cout << "SvgElement created." << endl;
        }
        
        void render() {
            //cout << "Rendering SVG Element." << endl;
        }
		virtual void draw(Gdiplus::Graphics& graphics) = 0;

};