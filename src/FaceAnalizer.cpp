#include "FaceAnalizer.h"


/**
returns two ofRectangles with were the eyes are
it does not rotate them: the rectangles are always parallel to the screen borders
left eye: facePoints 0..7
right eye: facePoints 8..15	
*/
void FaceAnalizer::getEyesRectangles(ofRectangle &leftEyeRegion, ofRectangle &rightEyeRegion, FT_VECTOR2D * facePoints) {
	leftEyeRegion.x = 9999.0f;
	leftEyeRegion.y = 9999.0f;
	leftEyeRegion.width = -1.0f;
	leftEyeRegion.height = -1.0f;

	rightEyeRegion.x = 9999.0f;
	rightEyeRegion.y = 9999.0f;
	rightEyeRegion.width = -1.0f;
	rightEyeRegion.height = -1.0f;

	for (int i = 0; i < 8; i++) {		
		// left eye
		if (facePoints[i].x < leftEyeRegion.x) leftEyeRegion.x = facePoints[i].x;
		if (facePoints[i].y < leftEyeRegion.y) leftEyeRegion.y = facePoints[i].y;

		if (facePoints[i].x > leftEyeRegion.x + leftEyeRegion.width) leftEyeRegion.width = facePoints[i].x - leftEyeRegion.x;
		if (facePoints[i].y > leftEyeRegion.y + leftEyeRegion.height) leftEyeRegion.height= facePoints[i].y - leftEyeRegion.y;

		// right eye
		int j = i + 8;
		if (facePoints[j].x < rightEyeRegion.x) rightEyeRegion.x = facePoints[j].x;
		if (facePoints[j].y < rightEyeRegion.y) rightEyeRegion.y = facePoints[j].y;

		if (facePoints[j].x > rightEyeRegion.x + rightEyeRegion.width) rightEyeRegion.width = facePoints[j].x - rightEyeRegion.x;
		if (facePoints[j].y > rightEyeRegion.y + rightEyeRegion.height) rightEyeRegion.height = facePoints[j].y - rightEyeRegion.y;
	}	
}


// to believe that a blink had happened we want this:
// 1) eyes are closed at time 0
// 2) eyes are open at time epsilon

// so it will work as follows:
// from a (fast) loop the client of this class asks blinked()

bool FaceAnalizer::blinked(FT_VECTOR2D * facePoints, unsigned char * oglVideoPixels, float * faceRot, float * facePos, bool draw){

	
	bool res = false;
	long long newTimeStamp = tom::getMillisSince1970();		
	bool changed = eyesChanged(facePoints, oglVideoPixels, faceRot, facePos, draw);	

	if (changed) {		
		res = newTimeStamp - timeStampBlink < BLINK_MAX_TIME;		
		cout << "time difference: " << newTimeStamp - timeStampBlink << endl;
	}
	
	timeStampBlink = tom::getMillisSince1970();	
	return res;
		
}

void FaceAnalizer::updateEyeReferences(ofxCvGrayscaleImage &leftEye, ofxCvGrayscaleImage &rightEye) {
		referenceLeftEye.clear();
		referenceRightEye.clear();

		referenceLeftEye.allocate(leftEye.width, leftEye.height);
		referenceRightEye.allocate(rightEye.width, rightEye.height);

		referenceLeftEye = leftEye;		
		referenceRightEye = rightEye;		

		referenceLeftEye.updateTexture();
		referenceRightEye.updateTexture();

		referenceEyesAvailable = true;		
}

// sort of checks if eyes are open or not 
// assumes we have information on the shared classes (image buffers and face tracked result)
// it needs a major refactoring
// it is NOT a robust classifier of closed eyes, but it works well finding sudden changes (it returns true if the eyes are closed
// but also returns true if the pupil is on a far corner of the eye)

bool FaceAnalizer::eyesChanged(FT_VECTOR2D * facePoints, unsigned char * oglVideoPixels, float * faceRot, float * facePos, bool draw) {
	
	ofRectangle leftEyeRegion;
	ofRectangle rightEyeRegion;

	getEyesRectangles(leftEyeRegion, rightEyeRegion, facePoints);
	
	int growAmount = 1; // todo set this as parameter?
	growRectangle (leftEyeRegion, growAmount);
	growRectangle (rightEyeRegion, growAmount);


	if (draw) {// draw two rectangles over the eye regions
		ofNoFill();
		ofSetColor (255, 0, 0, 128);
		ofRect (leftEyeRegion);

		ofSetColor (0, 0, 255, 128);
		ofRect (rightEyeRegion);

		ofSetColor (255);	
	}



	/*
	1) get one image of the eye.
	2) set it to black and white
	3) compare it with the previous one  (or with the stored open one that we know it's open)
	4) if dif > threshold -> blink
	*/

	// get one image of each eye and set them to black and white
	ofxCvGrayscaleImage leftEye, rightEye;	

	// get leftEye and rightEye
	getGrayscaleSubImage(oglVideoPixels, leftEyeRegion, leftEye);
	getGrayscaleSubImage(oglVideoPixels, rightEyeRegion, rightEye);	

	// if the face is rotated (yaw) more than BLINK_YAW_TOLERANCE (10) then we get a reference eyes
	if (abs(faceRot[1] - lastYawEyes) > BLINK_YAW_TOLERANCE) { 
		lastYawEyes = faceRot[1];		
		referenceEyesAvailable = false;
	}

	// idem for the pitch	
	if (abs(faceRot[0] - lastPitchEyes) > BLINK_PITCH_TOLERANCE) { 
		lastPitchEyes = faceRot[0];		
		referenceEyesAvailable = false;
	}


	if (!referenceEyesAvailable) {

		updateEyeReferences(leftEye, rightEye);

	} else { // we have reference eyes			
		
		ofSetLogLevel(OF_LOG_SILENT);
		ofxCvGrayscaleImage difL, difR;	

		int lw = referenceLeftEye.width; int lh = referenceLeftEye.height;
		int rw = referenceRightEye.width; int rh = referenceRightEye.height;

		// resize the eye images so that it corresponds with the references' size
		leftEye.resize(lw, lh); 
		rightEye.resize(rw, rh);		

		// difX = eye - reference
		difL = leftEye;
		difR = rightEye;

		difL.absDiff(leftEye, referenceLeftEye);
		difR.absDiff(rightEye, referenceRightEye);

		// we threshold (binarize) the difference. 
		int tr = 17;		
		difL.threshold(tr);		
		difR.threshold(tr);

		if (draw) {
			ofDrawBitmapString("eyes: ", 580, 560); 	leftEye.draw(630, 550);				rightEye.draw(660, 550);
			ofDrawBitmapString("dif: ", 580, 595); 		difL.draw (630, 585);				difR.draw (660, 585);
			ofDrawBitmapString("ref: ", 580, 630);		referenceLeftEye.draw (630, 620);	referenceRightEye.draw (660, 620);	
		}

		// we find contours in the binarized difference
		ofxCvContourFinder contourFinderLeft;
		ofxCvContourFinder contourFinderRight; // we use two because it's faster to have two for different sized images

		// image, min area, max area, # considered, use approximation
		float area = difL.width * difL.height;
		contourFinderLeft.findContours(difL, 10, area, 10, true);
		contourFinderRight.findContours(difL, 10, area, 10, true);	

		int nBlobsL, nBlobsR;

		nBlobsL = contourFinderLeft.nBlobs;				
		nBlobsR = contourFinderRight.nBlobs;	
				

		if (draw) {
			for (int i = 0; i < nBlobsL; i++){
				contourFinderLeft.blobs[i].draw(580,665);
			}

			for (int i = 0; i < nBlobsR; i++){
				contourFinderRight.blobs[i].draw(630,665);
			}
		
			ofDrawBitmapString("left: ", 750, 350);

			difL.draw(750, 360, 100, 100);
			contourFinderLeft.draw(750, 360, 100, 100);

			leftEye.draw(870, 360, 100, 100);
			contourFinderLeft.draw(870, 360, 100, 100);

			ofDrawBitmapString("right: ", 750, 490);

			difR.draw(750, 500, 100, 100);
			contourFinderRight.draw(750, 500, 100, 100);

			rightEye.draw(870, 500, 100, 100);
			contourFinderRight.draw(870, 500, 100, 100);

		}


		// we add all the pixels of the binarized images and divide the result by 10 (this gives us a change estimation)
		unsigned char * difLPx = difL.getPixels();
		int lb = 0;
		for (int i = 0; i < difL.width * difL.height; i++) {
			lb += difLPx[i];
		}
		lb /= 10;

		unsigned char * difRPx = difR.getPixels();
		int rb = 0;
		for (int i = 0; i < difR.width * difR.height; i++) {
			rb += difRPx[i];
		}
		rb /= 10;

		float difThreshold = 200 / facePos[2]; // we divide the threshold between the distance, so that we need less change when the user is far away
		
		if (draw) {
			ofColor(255, 255, 255, 255);
			ofFill();
			ofRect(580, 680, lb, 10);
			ofRect(580, 695, rb, 10);
			ofColor(128);
			ofRect(580, 710, difThreshold, 10);
		}
		
		// bool res = lb > difThreshold && rb > difThreshold && nBlobsL >= 1 && nBlobsR >= 1;

		bool res = lb > difThreshold && rb > difThreshold;

		if (res) {
			if (draw) { // if we are returning true, we set the whole screen to a transparent white				 
				glColor4f(1, 1, 1, 0.3f);
				ofRect (0, 0, ofGetWidth(), ofGetHeight());
				ofColor (255, 255, 255, 255);
			}

			// if the eyes changed, then this are the new references to check against
			updateEyeReferences(leftEye, rightEye);
			return true;
		}		
	}

	return false; 
}

FaceAnalizer::FaceAnalizer() {
	// eyes
	referenceEyesAvailable = false;
	lastYawEyes = 0;	
}

float FaceAnalizer::distFacePoints (FT_VECTOR2D * facePoints, int fp1, int fp2) {
	ofVec2f s (facePoints[fp1].x, facePoints[fp1].y);
	ofVec2f e (facePoints[fp2].x, facePoints[fp2].y);	
	return s.distance(e);
}

float FaceAnalizer::distFacePoints (FaceData face, int fp1, int fp2) {
	return distFacePoints (face.facePoints, fp1, fp2);
}

bool FaceAnalizer::mouthOpen (FT_VECTOR2D * facePoints, UINT facePointCount, FLOAT * animUnits, UINT animUnitCount) {
	float d = distFacePoints(facePoints, 66, 52);
	float threshold = MOUTH_OPEN_POINTS_THRESHOLD;

	// cout << d << endl;
	return (d > threshold);	
	//	return (animUnits[1] > 0.3); // if we believe animunits...
}

bool FaceAnalizer::mouthOpen(FaceData face) {
	return mouthOpen(face.facePoints, face.facePointCount, face.animUnits, face.animUnitCount);
}