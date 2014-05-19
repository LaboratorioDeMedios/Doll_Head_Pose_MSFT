#include <conio.h>
#include "Traces.h"
#include "ofAppGlutWindow.h"
//#include "MatrixVibroSound.h"
//#include "FaceWalrus.h"


//--------------------------------------------------------------
int main(){
	// create a window
	ofAppGlutWindow window; 

	// set width, height, mode (OF_WINDOW or OF_FULLSCREEN)
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	ofRunApp(new Traces());
}
