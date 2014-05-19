#include "MatrixVibroSound.h"

/**
MatrixVibroSound is a project for trying out eye-processing related code.
It uses and works with openframeworks 0.071.

tomas laurenzo 
tomas@laurenzo.net


*/


void MatrixVibroSound::setup(){

	// setup
	ofSetLogLevel(OF_LOG_SILENT); // keep the console quiet
	ofEnableAlphaBlending();

	ofSetDataPathRoot("bin/data/"); // fix so that it corresponds to win and not to osx - should add a define so that it only runs in win

	cout << "MatrixVibroSound. \ntomas laurenzo - tomas@laurenzo.net\n\n\n\n";

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

	// walrus
	facesManagerWalrus.setMaxEquivalentFaces(20);

	// motors
	vibrationGenerator = new VibrationGenerator(&chromation);
	

	// flags
	faceTracking = true;	
	showVideo = showDepth = false;	
	drawSuperImposedVertices = false;
	drawSuperImposedPoints = false;
	drawStatus = false;
	showMatrixSimple = false;
	vibraLookMode = false;
	virtualChoirMode = true;
	matrixWalrus = false;
	

	// members
	lastKeyPressed = -1;		
	

	// for testing/debugging
	testFlag = false;
	test1 = 0; 
	test2 = 0; 
	test3 = 0;

	
}

void MatrixVibroSound::keyReleased(int key){
	isKeyPressed = false;
}

void MatrixVibroSound::keyPressed(int key){
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

	case 'm':
		showMatrixSimple = !showMatrixSimple;
		break;

	case 'c':
		virtualChoirMode = !virtualChoirMode;
		break;

	case 'w':
		matrixWalrus = !matrixWalrus;
		break;

	case 'h':
		vibraLookMode = !vibraLookMode;
		break;

	case 'H':
		chromation.sendPower(255, 0);
		break;

	case 'g':
		faceTrackerManager->gettingVertices = !faceTrackerManager->gettingVertices;
		break;

	case ' ':
		drawStatus = !drawStatus;
		break;

	case '+':
		for (int i = 0; i < 20; i++); cout << endl;
		cout << status.str() << endl;
		break;
	
	case 'k':		
		facesManagerWalrus.save("theFile.xml"); // todo add it in #define
		break;

	case 'K':
		facesManagerWalrus.load("theFile.xml", COMPARATION_STYLE_ROTATION_ONLY);

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

	case 's':
		kinectManager->toggleSeated();
		break;

	case 'y': 
		testFlag = !testFlag;
		break;


	case 't':		
		// chromation.allOn();		
		midiGenerator.midiOut.sendControlChange(1, test1, mouseY);
		break;

	case 'T':
		chromation.allOff();
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

// performs calculatios before drawing: gets data from the kinect
void MatrixVibroSound::update(){

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
void MatrixVibroSound::updateStatus() {
	status.str("");
	status << "seated (s): " << kinectManager->getSeated() << ", nearMode (n): " << kinectManager->getNearMode() << ", face tracking (f): " << faceTracking << endl <<
		"lastTrackingSucceed: " << faceTrackerManager->lastTrackSucceeded << endl <<
		"facePos: " << faceTrackerManager->facePos[0] << ", " << faceTrackerManager->facePos[1] << ", " << faceTrackerManager->facePos[2] << endl <<						
		"faceRot: " << faceTrackerManager->faceRot[0] << ", " << faceTrackerManager->faceRot[1] << ", " << faceTrackerManager->faceRot[2] << endl << endl;

	status << "virtualchoir (c): " << virtualChoirMode << " matrix walrus (w): " << matrixWalrus << " vibralook (h): " << vibraLookMode << endl << endl;

	status << "getting vertices (g): " << faceTrackerManager->gettingVertices << 
		", showing vertices (x): " << drawSuperImposedVertices << endl << endl;
	
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
	ofDrawBitmapString("tomás laurenzo, tomas@laurenzo.net", 748, 758);
	ofSetColor(255);
}


// draws every frame
void MatrixVibroSound::draw(){
	
	// ofRect(100, 100, 100 * vibrationGenerator->averageVolume, 100);	

	if (showMatrixSimple) { // the matrix "by hand" without processing or storing
		int deltaX, deltaY;

		faceTrackerManager->getCenteredDeltas(deltaX, deltaY);
		
		// todo use FaceFrame
		// we need different flavour of faceframes

		ofImage temp;		

		int w = ofGetWidth() / 3;  // if these are stored (here or in the manager), then we can maintain the video feedback even when the tracking fails without jumping back to 0 0
		int h = ofGetHeight() / 3;
		int x = deltaX - w / 2;
		int y = deltaY - h / 2;

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x + w > videoFrame.getWidth()) x = videoFrame.getWidth() - w;
		if (y + x > videoFrame.getHeight()) y = videoFrame.getHeight() - h;

		
		temp.cropFrom(videoFrame, x, y, w, h);

		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				temp.draw(i*w, j*h);
		
	} // end show matrix simple
	
	
	if (showVideo) {
		videoFrame.draw(0,0);
	}

	if (showDepth) {
		depthFrame.draw(640,0);
	}
	 
	// if i'm tracking faces and also we have a tracked face, then we react to it	
	if (faceTracking && faceTrackerManager->lastTrackSucceeded) {		
		reactToFaceTracking();

		// if we were to use eye blinking detection, then we'd do something like: 
		// bool blinked = faceAnalizer.blinked(faceTrackerManager->facePoints, oglVideoPixels, faceTrackerManager->faceRot, showBlinkDetector);
		// if (blinked) reactToBlinking(); 
	}		

	if (testFlag){
		// do some debugging
	}

	if (drawStatus) {
		updateStatus();
	}
}


// reacts to the facetracking results depending on the program flags
// the facetracking results are encapsulated in FAceTrackerManager.

void MatrixVibroSound::reactToFaceTracking() {

	long x, y;

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

	if (vibraLookMode) {		
		vibraLook();
	}

	if (virtualChoirMode) {
		virtualChoir();
	}
}

void MatrixVibroSound::virtualChoir() {
	/*
	get facedata
	if walrus, we create a faceframe and store in the walrus manager
	draw 9 heads (if walrus draw equivalent heads)
	call midigenerator for every channel (9 of them)
	call vibrationgenerator (FaceData).

	*/

	FaceData fd = faceTrackerManager->getFaceData();

/*
	FaceFrameWalrus ff(oglVideoPixels, 800, 600, faceTrackerManager->faceScale, faceTrackerManager->facePos, faceTrackerManager->faceRot, 
		faceTrackerManager->facePoints, faceTrackerManager->facePointCount, faceTrackerManager->faceVertices, faceTrackerManager->vertexCount, 
		faceTrackerManager->animUnits, faceTrackerManager->animUnitCount, FACE_FRAME_MATRIX_MODE);
		*/

	FaceFrameWalrus ff(oglVideoPixels, 640, 480, faceTrackerManager->faceScale, faceTrackerManager->facePos, faceTrackerManager->faceRot, 
		faceTrackerManager->facePoints, faceTrackerManager->facePointCount, faceTrackerManager->faceVertices, faceTrackerManager->vertexCount, 
		faceTrackerManager->animUnits, faceTrackerManager->animUnitCount, FACE_FRAME_MATRIX_MODE);

	ofSetColor (255, 255, 255, 255);

	bool found;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!(i == 1 && j == 1)) { // not center				
				FaceFrameWalrus eq = facesManagerWalrus.getEquivalentFace(ff, &found, COMPARATION_STYLE_ROTATION_ONLY, false);
				eq.draw(i * eq.faceImage.width, j * eq.faceImage.height);						
			} else { // j == 1 && i == 1
				ff.draw(ff.faceImage.width, ff.faceImage.height);
			}				
		}
	}
	
	facesManagerWalrus.getEquivalentFace(ff, &found, COMPARATION_STYLE_ROTATION_ONLY, true);

	if (matrixWalrus) { // walrus'ing the image but not storing it, matrix mode
		// storing the image
	}
	
	//generate midi
	for (int i = 0; i < 9; i++) {
		//midiGenerator.generate(MIDI_GENERATOR_MODE_SCREAM, fd, i);
		midiGenerator.generate(MIDI_GENERATOR_MODE_PENTATONIC, fd, i);
	}

	// generate vibro pattern
	// if mouth open...
	vibrationGenerator->playPattern(fd);
			
}

void MatrixVibroSound::vibraLook() {
	float pitch = faceTrackerManager->faceRot[0];
	float yaw = faceTrackerManager->faceRot[1];

	/*

	test with four motors located as:
	0   1
	2   3


	pitch 42 up, 10 center, -10 down
	yaw -30 left, 0 center, 30 right
	*/		


	int leftPower = ofMap (yaw, 0, -30, 0, 255, true);
	int rightPower = ofMap (yaw, 0, 30, 0, 255, true);
	int upPower = ofMap (pitch, 10, 40, 0, 255, true);
	int downPower = ofMap (pitch, 10, -10, 0, 255, true);

	int power[4];

	power [0] = (upPower + rightPower) / 2;
	power [1] = (upPower + leftPower) / 2;		
	power [2] = (downPower + rightPower) / 2;
	power [3] = (downPower + leftPower) / 2;


	int x = 400;
	int y = 200;

	ofSetColor (255);
	for (int i = 0; i < 4; i++) {
		ofRect(x, y + 15 * i, power[i], 10);
	}		


	/*
	0 = 17		1 = 18
	2 = 19		3 = 16
	*/		

	power[0] = rightPower;
	power[1] = leftPower;
	power[2] = 0;
	power[3] = 0;

	int multi = 2;

	for (int i = 0; i < 4; i++) {
		power[i] *= multi;
		if (power[i] > 255) power[i] = 255;
		if (power[i] < 0) power[i] = 0;
	}

	chromation.sendPower(17, power[0]);
	chromation.sendPower(18, power[1]);
	chromation.sendPower(19, power[2]);
	chromation.sendPower(16, power[3]);		
}


// cleans up before exiting
void MatrixVibroSound::exit() {	
	chromation.stop();
	kinectManager->stop();
	faceTrackerManager->stop();	
}

void MatrixVibroSound::mouseDragged (int x, int y, int button) {

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



