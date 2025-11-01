#include<iostream>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

class SvgElement {
    private:
        string id;
        float opacity;
        Color fill;
        Color stroke;
    public:
        SvgElement() {
            //cout << "SvgElement created." << endl;
        }
        
        void render() {
            //cout << "Rendering SVG Element." << endl;
        }


};