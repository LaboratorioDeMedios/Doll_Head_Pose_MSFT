#pragma once

/**
Various unsorted utility methods by Tomas Laurenzo
tomas@laurenzo.net
*/

#include "ofMain.h"
#include "ofxOpenCv.h"

namespace tom {
	// extends a ofRrectangle by the growFactor
	void growRectangle (ofRectangle &r, float growFactor);
	void growRectangle (ofRectangle &r, float growX, float growY);
	void growRectangleTo (ofRectangle &r, float newW, float newH);
	
	void getGrayscaleSubImage(unsigned char * pixels, ofRectangle region, ofxCvGrayscaleImage &res);
	void getGrayscaleSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region, ofxCvGrayscaleImage &res);

	void getSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region, ofxCvColorImage &res);
	
	ofImage getSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region);	
	ofImage getSubImage(ofImage im, ofRectangle region);

	long long getMillisSince1970();	

	bool isNear (float c, float v, float f, bool isAngle = false);

	void drawBoxedString(string, int, int, ofColor, ofColor);
};