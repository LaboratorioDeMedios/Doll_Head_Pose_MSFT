#include "FaceWalrus.h"

/**
FaceWalrus is a project for trying out eye-processing related code.
It uses and works with openframeworks 0.071.

tomas laurenzo 
tomas@laurenzo.net


*/


void FaceWalrus::setup(){

	// setup
	ofSetLogLevel(OF_LOG_SILENT); // keep the console quiet
	ofEnableAlphaBlending();

	ofSetDataPathRoot("bin/data/"); // fixes it so that it corresponds to win and not to osx - should add a define so that it only runs in win

	cout << "FaceWalrus. \ntomas laurenzo - tomas@laurenzo.net\n\n\n\n";

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
	showVideo = showDepth = false;	
	drawSuperImposedVertices = false;
	drawSuperImposedPoints = false;
	drawStatus = false;
	magic = true;
	

	// members
	lastKeyPressed = -1;		

	// location
	
	resX = 230;
	resY = 166;
	resW = 93;
	resH = 140;


	// for testing/debugging
	testFlag = false;
	test1 = 0; 
	test2 = 0; 
	test3 = 0;	
}

void FaceWalrus::keyReleased(int key){
	isKeyPressed = false;
}

void FaceWalrus::keyPressed(int key){
	lastKeyPressed = key;
	isKeyPressed = true;

	switch (key) {
	case 'v': 
		showVideo = !showVideo;
		break;	 

	case 'k': 
		showDepth = !showDepth;
		break;

	case 'z':
		drawSuperImposedPoints = !drawSuperImposedPoints;
		break;	 

	case 'x':		
		drawSuperImposedVertices = !drawSuperImposedVertices;		
		break;	 

	case 'm':
		magic = !magic;
		break;

	case 'g':
		faceTrackerManager->gettingVertices = !faceTrackerManager->gettingVertices;
		break;

	case ' ':
		drawStatus = !drawStatus;
		cout << status.str() << endl << endl;
		break;

	case '+':
		for (int i = 0; i < 20; i++); cout << endl;
		cout << status.str() << endl;
		break;
	

	case 'F':
		ofToggleFullscreen();
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

	case 'l':
		kinectManager->toggleSeated();
		break;

	case 'y': 
		testFlag = !testFlag;
		break;

	case 't': 
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

	case 'a':
		resX--;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;
		
	case 'd':
		resX++;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 's':
		resY++;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 'w':
		resY--;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 'A':
		resW--;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 'D':
		resW++;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 'W':
		resH--;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	case 'S':
		resH++;
		cout << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
		break;

	} // switch

}

// performs calculatios before drawing: gets data from the kinect
void FaceWalrus::update(){

	// get the new pixel frames from the kinectManager
	oglVideoPixels = kinectManager->getOglVideoPixels();
	oglDepthPixels = kinectManager->getOglDepthPixels();

	// copy the frame from pixels to ofIamge
	videoFrame.setFromPixels(oglVideoPixels, 640, 480, OF_IMAGE_COLOR);
	depthFrame.setFromPixels(oglDepthPixels, 320, 240, OF_IMAGE_COLOR);		

	if (faceTracking) {
		faceTrackerManager->update();
	}
}



// updates the text status to be displayed
void FaceWalrus::updateStatus() {
	status.str("");
	status << "(d = k), seated (l): " << kinectManager->getSeated() << ", nearMode (n): " << kinectManager->getNearMode() << ", face tracking (f): " << faceTracking << endl <<
		"lastTrackingSucceed: " << faceTrackerManager->lastTrackSucceeded << endl <<
		"facePos: " << faceTrackerManager->facePos[0] << ", " << faceTrackerManager->facePos[1] << ", " << faceTrackerManager->facePos[2] << endl <<						
		"faceRot: " << faceTrackerManager->faceRot[0] << ", " << faceTrackerManager->faceRot[1] << ", " << faceTrackerManager->faceRot[2] << endl << endl;

	status << "getting vertices (g): " << faceTrackerManager->gettingVertices << 
		", showing vertices (x): " << drawSuperImposedVertices << endl << endl;

	status << "x: " << resX << " y: " << resY << " w: " << resW << " h: " << resH << endl << endl;
	
	status << "animUnit[0] (mouth): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[0] : 0) << endl;
	status << "animUnit[1] (jawDrop): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[1] : 0) << endl;
	status << "animUnit[2] (mouthStretch): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[2] : 0) << endl;
	status << "animUnit[3] (??): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[3] : 0) << endl;
	status << "animUnit[4] (??): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[4] : 0) << endl;
	status << "animUnit[5] (??): " << (faceTrackerManager->lastTrackSucceeded ? faceTrackerManager->animUnits[5] : 0) << endl;

	status << endl << "stored faces: " << facesManagerWalrus.getNumberOfFaces() << endl;

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
}


// draws every frame
void FaceWalrus::draw(){
	
	if (showVideo) {
		videoFrame.draw(0,0);
	}

	if (showDepth) {
		depthFrame.draw(640,0);
	}
	 
	// if i'm tracking faces and also we have a tracked face, then we react to it	
	if (faceTracking && faceTrackerManager->lastTrackSucceeded) {
		reactToFaceTracking();
	} else {
		// if there is no frace, then we do not draw anything
		// videoFrame.draw(0, 0, ofGetWidth(), ofGetHeight());		
	}

	if (testFlag){
		// do some debugging
	}

	if (drawStatus) {
		updateStatus();
	}
}


// reacts to the facetracking results depending on the program mode (so far only "drawsuperimposed")
// the facetracking results are encapsulated in FAceTrackerManager.

void FaceWalrus::reactToFaceTracking() {
	Vector4 face;
	face.w = 1;
	face.x = faceTrackerManager->facePos[0]; 
	face.y = faceTrackerManager->facePos[1]; 
	face.z = faceTrackerManager->facePos[2];		

	// now we draw the tracked points
		
	if (drawSuperImposedVertices) {
		faceTrackerManager->drawSuperImposedVertices();
	}

	if (drawSuperImposedPoints) {
		faceTrackerManager->drawSuperImposedPoints();		
	}
	
	if (magic) {
		FaceFrameWalrus ffw (oglVideoPixels, 640, 480, faceTrackerManager->faceScale, faceTrackerManager->facePos, faceTrackerManager->faceRot, 
				faceTrackerManager->facePoints,	faceTrackerManager->facePointCount, faceTrackerManager->faceVertices, faceTrackerManager->vertexCount, 
				faceTrackerManager->animUnits, faceTrackerManager->animUnitCount);
	 
		bool res = false;
		FaceFrameWalrus equivalent = facesManagerWalrus.getEquivalentFace(ffw, &res, COMPARATION_STYLE_ROTATION_ONLY);
		
		
		if (res) {			 
			equivalent.draw(resX, resY, resW, resH);		
			
		} else {					 			
			ffw.draw(resX, resY, resW, resH);
		}		
	}	
}

// cleans up before exiting
void FaceWalrus::exit() {	

	kinectManager->stop();
	faceTrackerManager->stop();
	
}

void FaceWalrus::mouseDragged (int x, int y, int button) {

	int valueX = ofMap(x, 0, ofGetWidth(), 0, 127);
	int valueY = ofMap(y, 0, ofGetWidth(), 127, 0);

	if (isKeyPressed) {

		switch (lastKeyPressed) {	
		case 'a': // dragging with 'a' pressed changes spomething for debugging
		
			break;			
		}

	} else { // !keypresed

	}

}



