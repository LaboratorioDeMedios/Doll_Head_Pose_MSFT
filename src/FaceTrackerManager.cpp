#include "FaceTrackerManager.h"

FaceTrackerManager::FaceTrackerManager(KinectManager *kinectManager) {
	this->kinectManager = kinectManager;

	HRESULT hr;

	FT_CAMERA_CONFIG videoConfig = {640, 480, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS};
	FT_CAMERA_CONFIG depthConfig = {320, 240, NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS};

	faceTracker = FTCreateFaceTracker();
	if (!faceTracker) {
		cout << "ERROR: can't create the face tracker" << endl;
	}

	hr = faceTracker->Initialize(&videoConfig, &depthConfig, NULL, NULL); 
	if (FAILED(hr)) {
		cout << "ERROR: can't initialize ft images" << endl;
	}

	FTResult = NULL; // if it's not initialized as NULL it does not work

	// Create a face tracking result interface	
	hr = faceTracker->CreateFTResult(&FTResult);
	if(FAILED(hr) || !FTResult) {
		cout << "ERROR: can't create FT result" << endl;
		cout << hr << endl;
	} else {
		cout << "FTResult created" << endl;
	}

	videoFTFrame = depthFTFrame = NULL;
	videoFTFrame = FTCreateImage();
	depthFTFrame = FTCreateImage();

	videoFTFrame->Allocate(640, 480, FTIMAGEFORMAT_UINT8_B8G8R8X8);	 
	depthFTFrame->Allocate(320, 240, FTIMAGEFORMAT_UINT16_D13P3);

	faceTrackerSetUp = true;
	lastTrackSucceeded = false;
	gettingVertices = true;

	facePoints = NULL;
	faceVertices = NULL;
	animUnits = NULL;
}

void FaceTrackerManager::update() {
	// got the data in videoFTBuffer and depthFTBuffer
	// we copy it TO videoFTFrame and depthFTFrame 
	HRESULT hrCopy = kinectManager->videoFTBuffer->CopyTo(videoFTFrame, NULL, 0, 0);		
	if (SUCCEEDED(hrCopy)) {
		hrCopy = kinectManager->depthFTBuffer->CopyTo(depthFTFrame, NULL, 0, 0);
	}

	if (SUCCEEDED(hrCopy)) {

		FT_SENSOR_DATA sensorData;
		sensorData.pVideoFrame = videoFTFrame;
		sensorData.pDepthFrame = depthFTFrame;
		sensorData.ZoomFactor = 1.0f;       // Not used must be 1.0
		POINT viewOffset = {0, 0};
		sensorData.ViewOffset = viewOffset; // Not used must be (0,0)

		FT_VECTOR3D* hint = NULL;		// this will only work for one user
		int i = 0;

		while (!kinectManager->skeletonTracked[i] && i < NUI_SKELETON_COUNT) i++;
		if (i < NUI_SKELETON_COUNT){
			hint = new FT_VECTOR3D[2];
			hint[0] = kinectManager->necks[i];
			hint[1] = kinectManager->heads[i];
		}				

		HRESULT hrFT = E_FAIL;

		// we call the face tracker to get a new result
		if (lastTrackSucceeded) {				
			hrFT = faceTracker->ContinueTracking(&sensorData, hint, FTResult);			
		} else {				
			hrFT = faceTracker->StartTracking(&sensorData, NULL, hint, FTResult);			 
		}

		delete (hint);

		lastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(FTResult->GetStatus());

		if (lastTrackSucceeded) {				
			processFaceTrackingResult(FTResult);
		} else {				
			FTResult->Reset();
		}
	}
}

// gets the result from the facetrackers and populates facePoints, facePointCount, faceScale, faceRot, facePos, animUnits y animUnitCount.
void FaceTrackerManager::processFaceTrackingResult(IFTResult *res) {

	res->Get3DPose(&faceScale, faceRot, facePos);		
	res->Get2DShapePoints(&facePoints, &facePointCount);
	res->GetAUCoefficients(&animUnits, &animUnitCount);	

	if (gettingVertices) {
		// trying to get the 121 points starts here ****
		IFTModel* ftModel;
		faceTracker->GetFaceModel(&ftModel);

		FT_CAMERA_CONFIG videoCameraConfig, depthCameraConfig; // move as members

		videoCameraConfig.Height = 640; // todo move as defines or members
		videoCameraConfig.Width = 480;
		videoCameraConfig.FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;

		depthCameraConfig.Height = 320;
		depthCameraConfig.Width = 240;
		depthCameraConfig.FocalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;

		HRESULT hr;

		FLOAT *shapeUnitCoefficients;
		UINT shapeUnitCount;
		BOOL converged;

		hr = faceTracker->GetShapeUnits(&faceScale, &shapeUnitCoefficients, &shapeUnitCount, &converged);	

		//if (faceVertices != NULL) free(faceVertices);


		vertexCount = ftModel->GetVertexCount();
		faceVertices = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));


		float zoomFactor = 1.0f;
		hr = ftModel->GetProjectedShape(&videoCameraConfig, zoomFactor, POINT(), shapeUnitCoefficients, ftModel->GetSUCount(), animUnits, animUnitCount, 
			faceScale, faceRot, facePos, faceVertices, vertexCount);

		int dX = 80;
		int dY = -83;

		// TODO todo TODO see why this offset occurs.

		for (int i = 0; i < vertexCount; i++){
			faceVertices[i].x += dX;
			faceVertices[i].y += dY;
		}

		if (FAILED(hr)) {
			cout << "ERROR: could not get the 121 projected points (FaceTrackerManager)" << endl;
		} 		
	}
}

HRESULT FaceTrackerManager::getClosestHint(FT_VECTOR3D* pHint3D) {
	int selectedSkeleton = -1;
	float smallestDistance = 0;

	if (!pHint3D){
		return(E_POINTER);
	}

	if (pHint3D[1].x == 0 && pHint3D[1].y == 0 && pHint3D[1].z == 0) {
		// Get the skeleton closest to the camera
		for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++) {
			if (kinectManager->skeletonTracked[i] && (smallestDistance == 0 || kinectManager->heads[i].z < smallestDistance)) {
				smallestDistance = kinectManager->heads[i].z;
				selectedSkeleton = i;
			}
		}
	} else {   // Get the skeleton closest to the previous position
		for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++) {
			if (kinectManager->skeletonTracked[i]) {
				float d = abs(kinectManager->heads[i].x - pHint3D[1].x) +
					abs(kinectManager->heads[i].y - pHint3D[1].y) +
					abs(kinectManager->heads[i].z - pHint3D[1].z);

				if (smallestDistance == 0 || d < smallestDistance) {
					smallestDistance = d;
					selectedSkeleton = i;
				}
			}
		}
	}
	if (selectedSkeleton == -1) {
		return E_FAIL;
	}

	pHint3D[0] = kinectManager->necks[selectedSkeleton];
	pHint3D[1] = kinectManager->heads[selectedSkeleton];

	return S_OK;
}

void FaceTrackerManager::getCenteredDeltas(int &x, int &y) { 
	getCenteredDeltas (facePoints, facePointCount, x, y);
}

// returns roughly how much you need to offset the points of the facetracker so to draw them centered
void FaceTrackerManager::getCenteredDeltas(FT_VECTOR2D p[], int fpc, int &x, int &y) { 
	int minX, minY, maxX, maxY;
	maxX = 0; 
	minX = ofGetWidth();
	maxY = 0; 
	minY = ofGetHeight();


	for (int i = 0; i < fpc; i++) {
		if (p[i].x > maxX) maxX = p[i].x;
		if (p[i].x < minX) minX = p[i].x;
		if (p[i].y > maxY) maxY = p[i].y;
		if (p[i].x < minY) minY = p[i].y;
	}

	x = (maxX + minX) / 2;
	y = (maxY + minY) / 2;	
}

// clean up
void FaceTrackerManager::stop() {	
	if (videoFTFrame) {
		videoFTFrame->Release();
		videoFTFrame = NULL;
		cout << "vidftframe off" << endl;
	}

	if (depthFTFrame) {
		depthFTFrame->Release();
		depthFTFrame = NULL;
		cout << "depthftframe off" << endl;
	}
}

// this has not been used, the values need to be stored as members, now they are impossible to access todo
void FaceTrackerManager::processAUs(FLOAT * animUnits, UINT animUnitCount){
	const float AU0LipLiftCoefficient = 0.5f / 16.0f;  //  what for?
	const float AU1JawDropCoefficient = 1.0f / 12.0f;
	const float AU2LipStretchCoefficient = 1.0f / 24.0f;
	const float AU3EyebrowLowerCoefficient = 2.0f / 16.0f;
	const float AU4MouthCornerCoefficient = -1.0f / 24.0f;
	const float AU5OuterBrowRaiserCoefficient = 1.0f / 16.0f;

	if (animUnits[0] < 0) animUnits[0] = 0; // what for?
	if (animUnits[1] < 0) animUnits[1] = 0;

	float UpperLipLift = animUnits[0] * AU0LipLiftCoefficient;
	float JawDrop = animUnits[1]* AU1JawDropCoefficient;
	float MouthStretch = animUnits[2] * AU2LipStretchCoefficient;
	float BrowLower = animUnits[3] * AU3EyebrowLowerCoefficient;
	float MouthCornerLift = animUnits[4] * AU4MouthCornerCoefficient;
	float OuterBrowRaiser = animUnits[5] * AU5OuterBrowRaiserCoefficient;        
}

bool FaceTrackerManager::mouthOpen(FT_VECTOR2D * facePoints, UINT facePointCount, FLOAT * animUnits, UINT animUnitCount) {
	ofVec2f s (facePoints[66].x, facePoints[66].y);
	ofVec2f e (facePoints[52].x, facePoints[52].y);	
	float d = s.distance(e);
	float threshold = MOUTH_OPEN_POINTS_THRESHOLD;

	//	if (faceRot[0] < 30 && faceRot[0] > -5)  // cout pass faceRot as parameter
	return (d > threshold);	
	//	else 
	//		return (animUnits[1] > 0.3);
}

void FaceTrackerManager::drawSuperImposedVertices() {
	if (!gettingVertices) {
		gettingVertices = true;
	} else {
		ofSetColor (255, 0, 0, 128);
		for (int i = 0; i < vertexCount; i++) {
			ofCircle(faceVertices[i].x, faceVertices[i].y, 2);
		}			
	}
}


void FaceTrackerManager::drawSuperImposedPoints(){
	ofSetColor (255, 255, 255, 255);
	for (int i = 0; i < facePointCount; i++) {				
		ofCircle(facePoints[i].x, facePoints[i].y, 1);
	}				
}

FaceData FaceTrackerManager::getFaceData() {
	FaceData res;
	res.animUnitCount = animUnitCount;
	res.animUnits = animUnits;
	res.facePointCount = facePointCount;
	res.facePoints = facePoints;
	res.facePos[0] = facePos[0]; res.facePos[1] = facePos[1]; res.facePos[2] = facePos[2];
	res.faceRot[0] = faceRot[0]; res.faceRot[1] = faceRot[1]; res.faceRot[2] = faceRot[2];
	res.faceScale = faceScale;
	res.faceVertices = faceVertices;
	res.vertexCount = vertexCount;	
	res.calculatePosition();
	return res;
}