/**
FaceWalrus

Tomas Laurenzo
tomas@laurenzo.net

This is part of an installation (to be named) by
Tomas Laurenzo and Qin Cai (qincai@microsoft.com)

*/

#pragma once

#define MINIMUM_TIME_BETWEEN_ADDING_FACES_IN_MILLISECONDS 1000 // one second

#include "tom.h"
#include "ofMain.h"
#include <FaceTrackLib.h>
#include "KinectManager.h"
#include "FaceTrackerManager.h"
#include "FaceAnalizer.h"
#include "FaceFrameWalrus.h"
#include "FacesManagerWalrus.h"

#include <NuiApi.h>

#include "ofxMidi.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"


using namespace tom;

class FaceWalrus : public ofBaseApp { 
public:
	void setup();
	void update();
	void draw();	
	void keyPressed(int key);
	void keyReleased(int key);
	void exit();
	void mouseDragged(int, int, int);
	
private:				
	KinectManager * kinectManager;
	FaceTrackerManager * faceTrackerManager;

	int lastKeyPressed;
	bool isKeyPressed;

	stringstream status;
	void updateStatus();

	// kinect data (we poll them from KinectManager)
	unsigned char * oglVideoPixels;
	unsigned char * oglDepthPixels;
	ofImage videoFrame, depthFrame;
 
	// flags
	bool showVideo, showDepth;
	bool drawSuperImposedVertices, drawSuperImposedPoints, drawStatus;			
	bool faceTracking;
	bool magic;

	// facetracker 
	void reactToFaceTracking();
	FacesManagerWalrus facesManagerWalrus;

	// for testing and debugging things
	bool testFlag;
	int test1, test2, test3;

	// location
	int resX, resY, resW, resH;
};
