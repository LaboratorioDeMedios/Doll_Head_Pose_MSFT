#include "FaceFrameWalrus.h"


// it sets as faceimage the full frame
void FaceFrameWalrus::setFaceImageFullFrame(unsigned char * fullFramePixels) {			
	faceImage.setFromPixels(fullFramePixels, 640, 480, OF_IMAGE_COLOR); // hardcoded, this should be a parameter.. the size of the kinect vga frame
}

// it sets as faceimage the full frame
void FaceFrameWalrus::setFaceImageRectangle(unsigned char * fullFramePixels, int w, int h) {			
	// now that we have all the information we store the actual face image
	// to do that we extract the face from the pixels and process it: cut it out, make it greyscale, blur it, make it rgba with alpha = luminosity

	ofRectangle faceRegion;
	faceRegion.x = min(faceVertices[29].x, faceVertices[120].x);
	float right = max (faceVertices[62].x, faceVertices[116].x);
	faceRegion.width = right - faceRegion.x;

	faceRegion.y = (faceVertices[0].y);
	faceRegion.height = faceVertices[10].y - faceVertices[0].y;
	
	// slightly bigger
	tom::growRectangleTo(faceRegion, w, h);

	ofxCvColorImage tempImage;			

	getSubImage(fullFramePixels, 640, 480, faceRegion, tempImage);
			
	// now we blur the final result so that it gets more ghostly but not too much that it's not understood
	// bgImage.blur(1); // 6 for screen, 1 for projected
			
	// if not up-channeled
	faceImage.setFromPixels(tempImage.getPixels(), tempImage.getWidth(), tempImage.getHeight(), OF_IMAGE_COLOR);
}


// this is the one that is used, oval face
void FaceFrameWalrus::setFaceImageOval(unsigned char * fullFramePixels) {		
	// now that we have all the information we store the actual face image
	// to do that we extract the face from the pixels and process it: cut it out, make it greyscale, blur it, make it rgba with alpha = luminosity

	ofRectangle faceRegion;
	faceRegion.x = min(faceVertices[29].x, faceVertices[120].x);
	float right = max (faceVertices[62].x, faceVertices[116].x);
	faceRegion.width = right - faceRegion.x;

	faceRegion.y = (faceVertices[0].y);
	faceRegion.height = faceVertices[10].y - faceVertices[0].y;
	
	// slightly bigger
	tom::growRectangle(faceRegion, 10, 20);

	ofxCvColorImage tempImage;			

	getSubImage(fullFramePixels, 640, 480, faceRegion, tempImage);
	
	// copy the image onto the mask
	ofxCvColorImage mask(tempImage);
	
	// get the opencv image for the mask
	IplImage * maskCvIm = mask.getCvImage();			

	// erase the mask to black
	cvFillImage(maskCvIm, 0);	

	// draw the white ellipse onto the mask			
	// if it was full frame, the ellipse would be
	// cvDrawEllipse(maskCvIm, cvPoint(faceRegion.getCenter().x, faceRegion.getCenter().y), cvSize(faceRegion.width, faceRegion.height), 0, 0, 360, cvScalar(255, 255, 255), CV_FILLED);

	// for the cropped image, the ellipse is
	int shrink = 10; // we shrink it a little bit to compensate for the blur
	cvDrawEllipse(maskCvIm, cvPoint(tempImage.width / 2, tempImage.height / 2), cvSize(faceRegion.width / 2 - shrink, faceRegion.height / 2 - shrink), 0, 0, 360, cvScalar(255, 255, 255), CV_FILLED);

	// blur the ellipse (feather)
	mask.blur(20); 

	//multiply the image and the mask bitwise.
	tempImage *= mask; 	

	// now we blur the final result so that it gets more ghostly but not too much that it's not understood
	// bgImage.blur(1); // 6 for screen, 1 for projected
			
	// if not up-channeled
	faceImage.setFromPixels(tempImage.getPixels(), tempImage.getWidth(), tempImage.getHeight(), OF_IMAGE_COLOR);
}

bool FaceFrameWalrus::operator==(const FaceFrameWalrus& otherFace) const {
	return hasSimilarAUs (otherFace);
}

bool FaceFrameWalrus::operator!=(const FaceFrameWalrus& otherFace) const {
	return !(*this == otherFace);
}

bool FaceFrameWalrus::hasSimilarRot(const FaceFrameWalrus& otherFace) const {
	bool foundDifferent = false;
	int i = 0; 	
	float tolerance = 2.5f;

	while (i < 3 && !foundDifferent) { 	 		
		foundDifferent = !isNear (faceRot[i], otherFace.faceRot[i], tolerance);
		i++;
	}

	return !foundDifferent;				 
}

bool FaceFrameWalrus::hasSimilarAUs(const FaceFrameWalrus& otherFace) const {
	bool foundDifferent = false;
	int i = 0; 
	float tolerance = 0.1f;
	
	while (i < animUnitCount && !foundDifferent) { 	 
		foundDifferent = !isNear (animUnits[i], otherFace.animUnits[i], tolerance);
		i++;
	}

	return !foundDifferent;				 
}

void FaceFrameWalrus::draw() {	 
	faceImage.draw(x, y);
}

void FaceFrameWalrus::draw(int x, int y) {
	faceImage.draw(x, y);
}

void FaceFrameWalrus::draw(int x, int y, int w, int h) {
	faceImage.draw(x, y, w, h);
}

void FaceFrameWalrus::drawCentered(int w, int h) {
	int x = ofGetWidth() / 2 - w / 2;
	int y = ofGetHeight() / 2 - h / 2;
	faceImage.draw(x, y, w, h);
}


FaceFrameWalrus::FaceFrameWalrus(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
	float faceScale, float facePos[3], float faceRot[3], 
	FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
	FLOAT * animUnits, UINT animUnitCount, bool jittered) : FaceFrame(fullFrameWidth, fullFrameHeight,
	faceScale, facePos, faceRot, 
	facePoints, facePointCount, faceVertices, vertexCount, 
	animUnits, animUnitCount, jittered) {			

		setFaceImageOval(fullFramePixels); 
}


FaceFrameWalrus::FaceFrameWalrus(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
	float faceScale, float facePos[3], float faceRot[3], 
	FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
	FLOAT * animUnits, UINT animUnitCount, int mode, bool jittered) : FaceFrame(fullFrameWidth, fullFrameHeight,
	faceScale, facePos, faceRot, 
	facePoints, facePointCount, faceVertices, vertexCount, 
	animUnits, animUnitCount, jittered) {			
		switch (mode) {

		case FACE_FRAME_MATRIX_MODE:
			setFaceImageRectangle(fullFramePixels, ofGetWidth() / 3, ofGetHeight() / 3); 
			break;

		case FACE_FRAME_OVAL_MODE:
			setFaceImageOval(fullFramePixels); 
			break;
		
		default:
			setFaceImageFullFrame(fullFramePixels);
		}
}
