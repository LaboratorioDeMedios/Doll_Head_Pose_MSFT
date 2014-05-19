#pragma once

/*
Class to process eye images. 
It should allow to detect if an eye is closed.
Future versions should include the position of the pupil as to do some basic gaze tracking

Tomas Laurenzo
tomas@laurenzo.net

*/

#include "ofxOpenCv.h"
#include <FaceTrackLib.h>
#include "tom.h"
#include "FaceData.h"

#define BLINK_YAW_TOLERANCE 3
#define BLINK_PITCH_TOLERANCE 3
#define BLINK_MAX_TIME 30 // millisecons

#define MOUTH_OPEN_POINTS_THRESHOLD 7

using namespace tom;


class FaceAnalizer {

public:
	FaceAnalizer();
	bool blinked(FT_VECTOR2D * facePoints, unsigned char * oglVideoPixels, float * faceRot, float * facePos, bool draw = false);		
	bool eyesChanged(FT_VECTOR2D * facePoints, unsigned char * oglVideoPixels, float * faceRot, float * facePos, bool draw = false);		
	bool referenceEyesAvailable;
	void getEyesRectangles(ofRectangle &leftEyeRegion, ofRectangle &rightEyeRegion, FT_VECTOR2D * facePoints);

	bool mouthOpen(FT_VECTOR2D * facePoints, UINT facePointCount, FLOAT * animUnits, UINT animUnitCount);
	bool mouthOpen(FaceData face);
	
	float distFacePoints (FT_VECTOR2D * facePoints, int fp1, int fp2);
	float distFacePoints (FaceData face, int fp1, int fp2);


private:	
	void updateEyeReferences(ofxCvGrayscaleImage &leftEye, ofxCvGrayscaleImage &rightEye);
	ofxCvGrayscaleImage referenceLeftEye, referenceRightEye;			
	float lastYawEyes, lastPitchEyes;
	long long timeStampBlink;	
};