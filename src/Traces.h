/**
Traces
------
An interactive installation.
------

------------------------------------------
  TODO: fix the eye blinking detection
------------------------------------------

It uses and works with openframeworks 0.071.

This is the main class. Inherits from ofBaseApp (openframeworks.cc)

tomas laurenzo 
tomas@laurenzo.net

*/

#pragma once

#define MINIMUM_TIME_BETWEEN_ADDING_FACES_IN_MILLISECONDS 1000 // one second
#define HEAD_HOST "164.73.36.246"
#define HEAD_PORT 12345

#include "tom.h"
#include "ofMain.h"
#include <FaceTrackLib.h>
#include "KinectManager.h"
#include "FaceTrackerManager.h"
#include "FaceAnalizer.h"
#include "FacesManagerTraces.h"
#include "FaceFrame.h"

#include <NuiApi.h>

//#include "ofxMidi.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"


using namespace tom;

class Traces : public ofBaseApp { 
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
	ofxOscSender sender;

	stringstream status;
	void updateStatus();


	// kinect data (we poll them from KinectManager)
	unsigned char * oglVideoPixels;
	unsigned char * oglDepthPixels;
	ofImage videoFrame, depthFrame;
 
	// flags
	bool showVideo, showDepth;
	bool drawSuperImposedVertices, drawSuperImposedPoints, drawStatus;	
	bool showBlinkDetector;	
	bool showTraces;
	bool faceTracking;

	// facetracker and blink detector
	void Traces::reactToFaceTracking();
	void Traces::reactToBlinking();

	FaceAnalizer faceAnalizer;

	// faces collection
	FacesManagerTraces facesManagerTraces;
	float tracesAlpha;
		
	long long lastAddedFaceTimestamp; // used not to add too many faces

	// for testing and debugging things
	bool testFlag;
	int test1, test2, test3;
	
};
