#include "FaceFrame.h"


 

/** all the information for creating the FaceFrame
- all the pixels from the vga frames and its size
- the coordinates of the face
- the points and vertices, and animation units
- if jittered is true then the same random values are added to x and y

**/

FaceFrame::FaceFrame(float fullFrameWidth, float fullFrameHeight,
	float faceScale, float facePos[3], float faceRot[3], 
	FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
	FLOAT * animUnits, UINT animUnitCount, bool jittered) {

		init (fullFrameWidth, fullFrameHeight,	faceScale, facePos, faceRot, facePoints, facePointCount, faceVertices, vertexCount, animUnits, animUnitCount, jittered);
}

FaceFrame::FaceFrame(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
	float faceScale, float facePos[3], float faceRot[3], 
	FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
	FLOAT * animUnits, UINT animUnitCount, bool jittered) {

		init (fullFrameWidth, fullFrameHeight,	faceScale, facePos, faceRot, facePoints, facePointCount, faceVertices, vertexCount, animUnits, animUnitCount, jittered);

		this->setFaceImage(fullFramePixels);	
}



void FaceFrame::init (float fullFrameWidth, float fullFrameHeight,
	float faceScale, float facePos[3], float faceRot[3], 
	FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
	FLOAT * animUnits, UINT animUnitCount, bool jittered) {
		expectancy = ofRandom(2 * MAX_AGE / 3, MAX_AGE);
		age = 0;

		this->faceScale = faceScale;
		for (int i = 0; i < 3; i++) {
			this->facePos[i] = facePos[i];
			this->faceRot[i] = faceRot[i];
		}

		// todo is std::copy faster?

		this->facePointCount = facePointCount;
		this->facePoints = new FT_VECTOR2D[facePointCount];			
		for (int i = 0; i < facePointCount; i++) {
			this->facePoints[i] = facePoints[i];
		}

		this->vertexCount = vertexCount;
		this->faceVertices = new FT_VECTOR2D[vertexCount];
		for (int i = 0; i < vertexCount; i++) {
			this->faceVertices[i] = faceVertices[i];
		}			

		this->animUnits = new FLOAT[animUnitCount];
		for (int i = 0; i < animUnitCount; i++) {
			this->animUnits[i]= animUnits[i];
		}
		this->animUnitCount = animUnitCount;	

		// now that we have everything, if jittered is true we translate this face jitY and jiyX 
		// this will add jitX and jitY to all the vertices, points, and pos
		if (jittered) {
			x += ofRandom (-JITTER, JITTER);
			y += ofRandom (-JITTER, JITTER);
		}

		fileName = "";
}


// sets the oval cut out face 
void FaceFrame::setFaceImage(unsigned char * fullFramePixels) {
		// now that we have all the information we stgore the actual face image
		// to do that we extract the face from the pixels and process it: cut it out, make it greyscale, blur it, make it rgba with alpha = luminosity

		ofRectangle faceRegion;
		faceRegion.x = min(faceVertices[29].x, faceVertices[120].x);
		float right = max (faceVertices[62].x, faceVertices[116].x);
		faceRegion.width = right - faceRegion.x;

		faceRegion.y = (faceVertices[0].y);
		faceRegion.height = faceVertices[10].y - faceVertices[0].y;

		tom::growRectangle(faceRegion, 10);

		ofxCvGrayscaleImage bgImage;			

		tom::getGrayscaleSubImage(fullFramePixels, faceRegion, bgImage);

		// copy the image onto the mask
		ofxCvGrayscaleImage mask(bgImage);

		// get the opencv image
		IplImage * maskCvIm = mask.getCvImage();			

		// erase the mask to black
		cvFillImage(maskCvIm, 0);

		// draw the white ellipse onto the mask		
		faceRegion.translate(-faceRegion.x + 35, -faceRegion.y + 55);  
		(faceRegion.width /= 2) -= 20;
		(faceRegion.height /= 2) -= 20;
		cvDrawEllipse(maskCvIm, cvPoint(faceRegion.getCenter().x, faceRegion.getCenter().y), cvSize(faceRegion.width, faceRegion.height), 0, 0, 360, cvScalar(255), CV_FILLED);

		// blur the ellipse (feather)
		mask.blur(6); // 10 for screen, 6 for projected

		//multiply the image and the mask bitwise.
		bgImage *= mask; 

		// now we blur the final result so that it gets more ghostly but not too much that it's not understood
		bgImage.blur(1); // 6 for screen, 1 for projected

		// "up-channel" the image from bg to rgba: x -> (xxxx) (rgba)
		unsigned char * newPixels = new unsigned char[(int) (bgImage.getWidth() * bgImage.getHeight() * 4)];

		for (int i = 0; i < (bgImage.getWidth() * bgImage.getHeight() * 4); i++) {
			newPixels[i] = 255;
		}

		unsigned char * oldPixels = bgImage.getPixels();

		for (int i = 0; i < (int) (bgImage.getWidth() * bgImage.getHeight()); i++) {
			for (int s = 0; s < 4; s++) {
				newPixels[i*4 + s] = oldPixels[i];
			}
		}

		faceImage.setFromPixels(newPixels, bgImage.getWidth(), bgImage.getHeight(), OF_IMAGE_COLOR_ALPHA);
}

void FaceFrame::translate(int x, int y) {	
	facePos[0] += x;
	facePos[1] += y;

	for (int i = 0; i < facePointCount; i++) {
		facePoints[i].x += x;
		facePoints[i].y += y;
	}

	for (int i = 0; i < vertexCount; i++) {
		faceVertices[i].x += x;
		faceVertices[i].y += y;
	}		
}



void FaceFrame::tick() {
	age++;
}

void FaceFrame::render(float x, float y, int alpha) {
	// alpha goes from 255 to 0 as age gets closer to expectancy.

	float localAlpha = 0;
	localAlpha = ofMap(age, expectancy, 0, 0, 255);	

	float mult = (float)alpha / 255.0f;
	localAlpha *= mult;

	ofSetColor(255, 255, 255, localAlpha);
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale (SIZE_MULTIPLIER, SIZE_MULTIPLIER);
	faceImage.draw(0, 0);	
	ofPopMatrix();
}


void FaceFrame::updateScreenPos() {
	x = faceVertices[0].x; 
	y = faceVertices[0].y; // Todo see why this vertex... in facedata i'm using facepoint 41 which is kinda centered
}

void FaceFrame::render() {
	updateScreenPos();
	float alpha = 255 - 255 * age / expectancy;
	render (x, y, alpha);
}

void FaceFrame::render(int alpha) {
	//render(facePos[0], facePos[1], 255); translations are in meters, we don't want to do the projection ourselves so we'll used the projected vertices 
	updateScreenPos();
	render (x, y, alpha);
}

bool FaceFrame::alive() {	
	// cout << "a: " << age << " e: " << expectancy << endl;
	return age < expectancy;
}

string FaceFrame::saveImage() {
	if (fileName == "") {
		string name = "face-" + ofToString(ofGetElapsedTimeMillis()) + ".png";
		faceImage.saveImage(name);
		fileName = name;		
	}

	return fileName;
}