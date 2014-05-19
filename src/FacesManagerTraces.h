/**


http://laurenzo.net
*/

#pragma once
 
#include "ofMain.h"
#include "FaceFrame.h"
#include "ofxXmlSettings.h"

#define MAX_FACES 10000

class FacesManagerTraces { 

public:
	FacesManagerTraces();
	void init();

	void tick(); // age++
	void addFaceFrame(FaceFrame face);
	void render();
	void render(int alpha); //alpha 0..255	
	void renderBlended(int alpha = 255); // alpha 0..255
		
	void reset();
	int getNumberOfFaces();	

	FaceFrame getFaceFrame (int which);		

private:
	float tolerance;
	ofxXmlSettings XML;
	
	vector <FaceFrame> faces;

};