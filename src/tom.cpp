#include "tom.h"

void tom::growRectangle (ofRectangle &r, float growFactor){	
	r.x -= growFactor;
	r.width += growFactor * 2;
	r.y -= growFactor;
	r.height += growFactor * 2;
}

void tom::growRectangle (ofRectangle &r, float growX, float growY){	
	r.x -= growX;
	r.width += growX * 2;
	r.y -= growY;
	r.height += growY * 2;
}

void tom::growRectangleTo (ofRectangle &r, float newW, float newH){	
	int growX = newW - r.width;
	int growY = newH - r.height;

	r.x -= growX / 2;
	r.width += growX;
	r.y -= growY / 2;
	r.height += growY;
}

// returns a sub image of as a ofxvc grayscale image
// assumes the image is 640 x 480
// deprecated :)
void tom::getGrayscaleSubImage(unsigned char * pixels, ofRectangle region, ofxCvGrayscaleImage &res) {
	return getGrayscaleSubImage(pixels, 640, 480, region, res);
}  

// returns a sub image of as a ofxvc grayscale image
void tom::getGrayscaleSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region, ofxCvGrayscaleImage &res) {
	ofImage temp;
	temp.setFromPixels(pixels, wPixels, hPixels, OF_IMAGE_COLOR, true);

	temp.crop(region.x, region.y, region.width, region.height);

	ofxCvColorImage tempCv;
	tempCv.setFromPixels(temp.getPixels(), temp.width, temp.height);		

	res = tempCv;		          
}  


void tom::getSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region, ofxCvColorImage &res) {
	ofImage temp;
	temp.setFromPixels(pixels, wPixels, hPixels, OF_IMAGE_COLOR);

	temp.crop(region.x, region.y, region.width, region.height);
		
	res.setFromPixels(temp.getPixels(), temp.width, temp.height);		
}

// returns a subregion of pixels
ofImage tom::getSubImage(unsigned char * pixels, int wPixels, int hPixels, ofRectangle region) {
	ofImage temp;
	temp.setFromPixels(pixels, wPixels, hPixels, OF_IMAGE_COLOR, pixels);
	temp.crop(region.x, region.y, region.width, region.height);
	return temp;
}

// returns a subregion of pixels
ofImage tom::getSubImage(ofImage im, ofRectangle region) {
	ofImage temp;
	temp = im;
	temp.crop(region.x, region.y, region.width, region.height);
	return temp;
}


// returns the numbner of milliseconds since jan 1, 1970
long long tom::getMillisSince1970() {
	Poco::Timestamp epoch(0);  
	Poco::Timestamp now;  
	Poco::Timestamp::TimeDiff diffTime = (now - epoch);  
	long long millisSince1970 = (long long) (diffTime/1000);  	
	return millisSince1970;
}


// returns true if |c - v| < f
bool tom::isNear (float c, float v, float f, bool isAngle) {
	bool n = ((v >= (c-f)) && (v <= (c+f)));
	bool n1 = ((v-180 >= (c-f)) && (v-180 <= (c+f)));
	bool n2 = ((v+180 >= (c-f)) && (v+180 <= (c+f)));

	if (!isAngle) return n;
	else return (n || n1 || n2);
}


// draws a box with a string in it.
void tom::drawBoxedString(string s, int x, int y, ofColor foreground, ofColor background) {
	int w = s.length() * 10 + 6;
	int h = 16;
	ofFill();
	ofSetColor(background);
	ofRect(x, y, w, h);

	ofNoFill();
	ofSetColor(foreground);
	ofRect(x, y, w, h);	

	ofDrawBitmapString(s, x + 4, y + 12);
}
