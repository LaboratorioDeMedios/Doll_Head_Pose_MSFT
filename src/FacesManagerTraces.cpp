#include "FacesManagerTraces.h"

void FacesManagerTraces::render(int alpha) { //alpha 0..255
	for(vector<FaceFrame>::iterator it = faces.begin(); it != faces.end(); ++it) {
		it->render(alpha);
	}
}

void FacesManagerTraces::render() {
	for(vector<FaceFrame>::iterator it = faces.begin(); it != faces.end(); ++it) {
		it->render();
	}
}

void FacesManagerTraces::reset() {
	faces.clear();
}

// this is incredibly slow and useless.. images are now rgba there's no need for this.
// also, it does not makje sense to manipulate all these pixels in the cpu
void FacesManagerTraces::renderBlended(int alpha) {

	cout << "WARNING: THIS IS UTTERLY SLOW, DO NOT USE" << endl;
	ofSetColor (255, 255, 255, alpha);
	
	// Create an image
	// add all the faces
	// draw the image

	// this is slow
	unsigned char * bgPixels = new unsigned char[ofGetWidth() * ofGetHeight()];
	for (int i = 0; i < ofGetWidth() * ofGetHeight(); i++) bgPixels[i] = 0;

	// this is also incredibly slow
	for (int i = 0; i < faces.size(); i++) {
		
		faces[i].updateScreenPos();

		ofImage t = faces[i].faceImage;
		unsigned char * facePixels = t.getPixels();
		
		// we add the facePixels to the bgPixels
		for (int x = 0; x < t.width; x++) {
			for (int y = 0; y < t.height; y++) {
				
				int facePos = y * t.width + x;
				int bgPos = (faces[i].y + y)* ofGetWidth() + faces[i].x + x;

				bgPixels[bgPos] += facePixels[facePos];
			}
		}
	}

	ofImage res;
	res.setFromPixels(bgPixels, ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);
	res.draw(0, 0);


}



FacesManagerTraces::FacesManagerTraces() {
	init();
}

void FacesManagerTraces::init() {
}

void FacesManagerTraces::addFaceFrame(FaceFrame face) {
	faces.push_back(face);
}


// all the faces grow older and those who died are removed
void FacesManagerTraces::tick() {
	
	for (int i = 0; i < faces.size(); i++) {
		faces[i].tick();
		if (!faces[i].alive()) faces.erase(faces.begin() + i);
	}
}

int FacesManagerTraces::getNumberOfFaces(){
	return faces.size();
}

FaceFrame FacesManagerTraces::getFaceFrame (int which) {
	return faces[which];
}
