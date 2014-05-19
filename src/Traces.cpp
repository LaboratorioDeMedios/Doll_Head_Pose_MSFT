#include "Traces.h"

/**
Traces is a project for trying out eye-processing related code.
It uses and works with openframeworks 0.071.

tomas laurenzo 
tomas@laurenzo.net


*/


void Traces::setup(){

	// setup
	ofSetLogLevel(OF_LOG_SILENT); // keep the console quiet
	ofEnableAlphaBlending();
	ofSetDataPathRoot("bin/data/"); // fix so that it corresponds to osx's - should add a define so that it only runs in win

	sender.setup(HEAD_HOST, HEAD_PORT);	

	cout << "FaceTracker. \ntomas laurenzo - tomas@laurenzo.net\n\n\n\n";

	ofBackground(0); // black background
	
	// setup the kinect
	kinectManager = new KinectManager();
	HRESULT hr = kinectManager->setup();

	if (FAILED(hr)) {
		cout << "ERROR setting kinect up (kinectManager)" << endl;
	} else {
		cout << "kinect set up OK" << endl;	
	}

	// setup the face tracker
	faceTrackerManager = new FaceTrackerManager (kinectManager);	
	cout << "ft set up" << endl;	
	

	// flags
	faceTracking = true;	
	showVideo = showDepth = true;	
	drawSuperImposedVertices = false;
	drawSuperImposedPoints = true;
	showBlinkDetector = false;
	drawStatus = true;
	showTraces = false;

	// members
	lastKeyPressed = -1;		
	tracesAlpha = 255;
	lastAddedFaceTimestamp = 0;


	// for testing/debugging
	testFlag = false;
	test1 = 0; 
	test2 = 0; 
	test3 = 0;

}

void Traces::keyReleased(int key){
	isKeyPressed = false;
}

void Traces::keyPressed(int key){
	lastKeyPressed = key;
	isKeyPressed = true;

	switch (key) {
	case 'v': 
		showVideo = !showVideo;
		break;	 

	case 'd': 
		showDepth = !showDepth;
		break;

	case 'z':
		drawSuperImposedPoints = !drawSuperImposedPoints;
		break;	 

	case 'x':		
		drawSuperImposedVertices = !drawSuperImposedVertices;		
		break;	 

	case 'g':
		faceTrackerManager->gettingVertices = !faceTrackerManager->gettingVertices;
		break;

	case 'e':
		faceAnalizer.referenceEyesAvailable = false;
		break;

	case ' ':
		drawStatus = !drawStatus;
		break;

	case '+':
		for (int i = 0; i < 20; i++); cout << endl;
		cout << status.str() << endl;
		break;

	case 'b':
		showBlinkDetector = !showBlinkDetector;
		break;

	case 'c':
		showTraces = !showTraces;
		break;
		
	case 'C':
		facesManagerTraces.reset();
		break;

	case 'F':
		ofToggleFullscreen();
		facesManagerTraces.init(); // reallocate size-dependant things
		break;	

	case 357:		
		kinectManager->modifyAngle(2);
		break;

	case 359:
		kinectManager->modifyAngle(-2);
		break;

	case 'n':
		kinectManager->toggleNearMode();	
		break;

	case 'f':
		faceTracking = !faceTracking;
		break;

	case 's':
		kinectManager->toggleSeated();
		break;

	case 'y': 
		testFlag = !testFlag;
		break;

	case 't': 
		//facesManagerTraces.renderBlended();
		reactToBlinking();
		break;

	case 'r':
		test1++;
		if (test1 > 121) test1 = 0;
		cout << test1 << endl;

		break;

	case 'R': 
		test1--;
		if (test1 < 0) test1 = 121;
		cout << test1 << endl;

		break;

	} // switch

}

// performs calculatios before drawing the faceframe: gets data from the kinect
void Traces::update(){

	// get the new pixel frames from the kinectManager
	oglVideoPixels = kinectManager->getOglVideoPixels();
	oglDepthPixels = kinectManager->getOglDepthPixels();

	// copy the frame from pixels to ofIamge
	videoFrame.setFromPixels(oglVideoPixels, 640, 480, OF_IMAGE_COLOR);
	depthFrame.setFromPixels(oglDepthPixels, 320, 240, OF_IMAGE_COLOR);		

	if (faceTracking) {
		faceTrackerManager->update();
	}

	facesManagerTraces.tick();
}



// updates the text status to be displayed
void Traces::updateStatus() {
	status.str("");
	status << "seated (s): " << kinectManager->getSeated() << ", nearMode (n): " << kinectManager->getNearMode() << ", face tracking (f): " << faceTracking << endl <<
		"lastTrackingSucceed: " << faceTrackerManager->lastTrackSucceeded << endl <<
		"facePos: " << faceTrackerManager->facePos[0] << ", " << faceTrackerManager->facePos[1] << ", " << faceTrackerManager->facePos[2] << endl <<						
		"faceRot: " << faceTrackerManager->faceRot[0] << ", " << faceTrackerManager->faceRot[1] << ", " << faceTrackerManager->faceRot[2] << endl << endl;

	status << "getting vertices (g): " << faceTrackerManager->gettingVertices << 
		", showing vertices (x): " << drawSuperImposedVertices << endl << "showing traces (c): " << showTraces << ", traces size: " << facesManagerTraces.getNumberOfFaces() << endl << endl;
	
	status << "animUnit[0] (mouth): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[0] : 0) << endl;
	status << "animUnit[1] (jawDrop): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[1] : 0) << endl;
	status << "animUnit[2] (mouthStretch): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[2] : 0) << endl;
	status << "animUnit[3] (??): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[3] : 0) << endl;

	status << endl << "framerate: " << ofToString(ofGetFrameRate(), 2) << endl;

	status << endl << "user skeleton heads: ";
	for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
		status << i << ": ";				
		if (kinectManager->skeletonTracked[i]) {
			status << kinectManager->heads[i].x << ", " << kinectManager->heads[i].y;
			ofEllipse(kinectManager->heads[i].x, kinectManager->heads[i].y, 10, 10);
		} else {
			status << "no";
		}
		status << "  ";
	}
	ofSetColor(255);

	ofDrawBitmapString(status.str(), 10, 500);	

	ofSetColor(100);
	ofDrawBitmapString("tomas laurenzo, tomas@laurenzo.net", 748, 758);
	ofSetColor(255);
	
	
	//m.setAddress("/mouse/position");
	ofxOscMessage m;
	m.setAddress("/head/orientationAngles/");
	m.addFloatArg((-1)*faceTrackerManager->faceRot[0]);
	m.addFloatArg((-1)*faceTrackerManager->faceRot[1]);
	m.addFloatArg((-1)*faceTrackerManager->faceRot[2]);
	sender.sendMessage(m);
}




// draws every frame
void Traces::draw(){
	
	if (showVideo) {
		videoFrame.draw(0,0);
	}

	if (showDepth) {
		depthFrame.draw(640,0);
	}


	// if i'm tracking faces and also we have a tracked face, then we react to it (for example drawing points over the face).
	// if it is blinking (eyes closed) then we react to that, too 
	if (faceTracking && faceTrackerManager->lastTrackSucceeded) {		
		reactToFaceTracking();
		bool blinked = faceAnalizer.blinked(faceTrackerManager->facePoints, oglVideoPixels, faceTrackerManager->faceRot, faceTrackerManager->facePos, showBlinkDetector);

		if (blinked) reactToBlinking(); 
	}	

	if (showTraces){
		facesManagerTraces.render();		 
		// facesManagerTraces.renderBlended(tracesAlpha);
		ofSetColor(255);
	}	

	if (testFlag){
		FaceFrame po = facesManagerTraces.getFaceFrame(0);		
	}

	if (drawStatus) {
		updateStatus();
	}
}

void Traces::reactToBlinking() {
	
	ofSetColor(0, 255, 0, 200);
	ofRect(100, 600, 300, 300);
	ofSetColor(255);
	// we create one FaceFrame and add it to the FacesCollection
	// 640 and 480 should come from kinectmanager 

	// if we are not getting vertices in the FaceTrackerManager, then this will crash, as the FaceFrame stores the vertices.
	if (!faceTrackerManager->gettingVertices){
		cout << "ERROR: we need the vertices for the FaceFrame" << endl;
	}
	else {
		// if the last face we added was less than MINIMUM_TIME_BETWEEN_ADDING_FACES_IN_MILLISECONDS then we add it.
		// we create the new face frame
		if (getMillisSince1970() - lastAddedFaceTimestamp > MINIMUM_TIME_BETWEEN_ADDING_FACES_IN_MILLISECONDS) {
			lastAddedFaceTimestamp = getMillisSince1970();

			FaceFrame ff (oglVideoPixels, 640, 480, faceTrackerManager->faceScale, faceTrackerManager->facePos, faceTrackerManager->faceRot, 
				faceTrackerManager->facePoints,	faceTrackerManager->facePointCount, faceTrackerManager->faceVertices, faceTrackerManager->vertexCount, 
				faceTrackerManager->animUnits, faceTrackerManager->animUnitCount);

			// store the FF in the facescollection
			facesManagerTraces.addFaceFrame(ff);
		}
	}		
}


// reacts to the facetracking results depending on the program mode (so far only "drawsuperimposed")
// the facetracking results are encapsulated in FAceTrackerManager.

void Traces::reactToFaceTracking() {
	Vector4 face;
	face.w = 1;
	face.x = faceTrackerManager->facePos[0]; 
	face.y = faceTrackerManager->facePos[1]; 
	face.z = faceTrackerManager->facePos[2];		

	long x, y;

	// todo draw the face pos and rot

	// now we draw the tracked points
	// todo move this into a function
	int fpc = (int)faceTrackerManager->facePointCount;		
	int deltaX, deltaY;
	faceTrackerManager->getCenteredDeltas(faceTrackerManager->facePoints, fpc, deltaX, deltaY);
	deltaX = ofGetWidth() / 2 - deltaX;
	deltaY = ofGetHeight() / 2 - deltaY;	
		
	if (drawSuperImposedVertices) {
		faceTrackerManager->drawSuperImposedVertices();
	}

	if (drawSuperImposedPoints) {
		faceTrackerManager->drawSuperImposedPoints();		
	}
	
}

// cleans up before exiting
void Traces::exit() {	

	kinectManager->stop();
	faceTrackerManager->stop();
	
}

void Traces::mouseDragged (int x, int y, int button) {

	int valueX = ofMap(x, 0, ofGetWidth(), 0, 127);
	int valueY = ofMap(y, 0, ofGetWidth(), 127, 0);

	if (isKeyPressed) {

		switch (lastKeyPressed) {	
		case 'a': // dragging with 'a' pressed changes the traces collection alpha
			tracesAlpha = mouseY - 10;
			if (tracesAlpha < 0) tracesAlpha = 0;
			if (tracesAlpha > 255) tracesAlpha = 255;
			break;			
		}

	} else { // !keypresed

	}

}



