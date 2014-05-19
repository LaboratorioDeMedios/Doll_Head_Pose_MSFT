#include "KinectManager.h"

KinectManager::KinectManager ()  {
	// this->parent = parent;
	
	kinectInitialized = false;
	nearMode = false; // will be toggled at setupKinect()	
	seated = true;

	faceTrackerBuffersSetUp = false;
	videoFTBuffer = depthFTBuffer = NULL;
	
	videoFTBuffer = FTCreateImage(); // data to these are copied at the event handling
	depthFTBuffer = FTCreateImage();
	
	if (!videoFTBuffer || !depthFTBuffer) {
		cout << "ERROR: out of memory, can't create buffers for FaceTracking";
	}
	
	HRESULT hr;
	hr = videoFTBuffer->Allocate(640, 480, FTIMAGEFORMAT_UINT8_B8G8R8X8);
	hr = depthFTBuffer->Allocate(320, 240, FTIMAGEFORMAT_UINT16_D13P3);
	
	if (!videoFTBuffer || !depthFTBuffer) {
		cout << "ERROR: out of memory, can't allocate buffers";
	} else {
		cout << "Buffers created OK" << endl;
		faceTrackerBuffersSetUp = true;
	}

	oglVideoPixels = new unsigned char[640*480*3];
	oglDepthPixels = new unsigned char[320*240*3];

}

long KinectManager::getAngle() {
	return kinectAngle;
}

void KinectManager::modifyAngle (long d) {
	kinectAngle += d;	
	kinect->NuiCameraElevationSetAngle(kinectAngle);
}

unsigned char * KinectManager::getOglVideoPixels() {
	newVideoFrame = false;
	return oglVideoPixels;
}

unsigned char * KinectManager::getOglDepthPixels() {
	newDepthFrame = false;
	return oglDepthPixels;
}


HRESULT KinectManager::toggleNearMode(){
	HRESULT hr = E_FAIL;

	if (kinect)  {
		hr = kinect->NuiImageStreamSetImageFrameFlags(depthStreamHandle, nearMode ? 0 : NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);

		if (SUCCEEDED(hr)) {
			nearMode = !nearMode;
		}
	}

	return hr;
}

void KinectManager::toggleSeated() {
	seated=!seated;
	kinect->NuiSkeletonTrackingEnable(nextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE | (seated ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0));	
}

bool KinectManager::getSeated() {
	return seated;
}

bool KinectManager::getNearMode() {
	return nearMode;
}

HRESULT KinectManager::setup() {

	HRESULT hr, hr0;
	INuiSensor * pNuiSensor;

	int iSensorCount = 0;
	hr0 = NuiGetSensorCount(&iSensorCount);

	if (FAILED(hr0)) {
		return hr0;
	}

	cout << "Number of kinects: " << iSensorCount << endl;	

	// Look at each Kinect sensor
	for (int i = 0; i < iSensorCount; ++i) {
		// " Create the sensor so we can check status, if we can't create it, move on to the next" << endl;

		hr0 = NuiCreateSensorByIndex(i, &pNuiSensor);
		if (FAILED(hr0)) {
			continue;
		}

		// " Get the status of the sensor, and if connected, then we can initialize it"<<endl;
		hr0 = pNuiSensor->NuiStatus();
		if (S_OK == hr0) {
			kinect = pNuiSensor;
			break;
		}

		// "     This sensor wasn't OK, so release it since we're not using it. " << endl;
		pNuiSensor->Release();
	}

	if (NULL != kinect) { // we got one

		// Initialize the Kinect and specify that we'll be using color, depth, and skeleton
		hr = kinect->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR); 
		if (FAILED(hr)) cout << "ERROR: can't initialize kinect" << endl;
		else cout << "Kinect initialized" << endl;

		// init skeleton

		hr = kinect->NuiSkeletonTrackingEnable(nextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE | (seated ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0));
		if (FAILED(hr)) cout << "ERROR: Skeleton tracking enabling FAILED" << endl;
		else cout << "Skeleton tracking enabled OK" << endl;		 	 

		// create events
		nextDepthFrameEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
		nextVideoFrameEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
		nextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

		// init video stream
		hr = kinect->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextVideoFrameEvent, &videoStreamHandle);
		if (FAILED(hr)) cout << "ERROR: video stream opening FAILED" << endl;
		else cout << "Video stream opened OK" << endl;

		// init depth stream at 320x240 
		// todo see if it should be changed to 640x480
		hr = kinect->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE, 2, nextDepthFrameEvent, &depthStreamHandle);
		if (FAILED(hr)) { 
			cout << "ERROR: depth stream opening FAILED" << endl;
			if (hr ==  E_FAIL) cout << "An unspecified error occurred" << endl;
			else if (hr == E_INVALIDARG) cout << "	The value of the dwFlags parameter is NULL." << endl;
			else if (hr == E_NUI_DEVICE_NOT_READY) cout << "	Kinect has not been initialized." << endl;
			else if (hr == E_OUTOFMEMORY) cout << "	The allocation failed." << endl;
			else if (hr == E_POINTER) cout << "	The hNextFrameEvent parameter is an invalid handle." << endl;
			else cout << "Unkown HRESULT!!: " << hr << endl;
		}
		else cout << "Depth stream opened OK" << endl;


		// Start with near mode on
		toggleNearMode();		

		// create threads
		nuiProcessStop = CreateEventW(NULL, TRUE, FALSE, NULL);
		nuiProcess = CreateThread(NULL, 0, nuiProcessThread, this, 0, NULL);	

		// init skeleton tracking data
		for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
			heads[i] = necks[i] = FT_VECTOR3D(0, 0, 0);
			skeletonTracked[i] = false;
		}

		kinectInitialized = true;
		kinect->NuiCameraElevationGetAngle(&kinectAngle);
	}

	if (NULL == kinect || FAILED(hr0)) {
		cout << "ERROR: kinect not found!" << endl;
		return E_FAIL;
	}

	return hr;
}

//  process that polls the kinect
DWORD WINAPI KinectManager::nuiProcessThread(LPVOID param){
	HANDLE events[4];
	int eventIdx;

	// parent
	KinectManager *parent=(KinectManager *) param;

	// Configure events to be listened on
	events[0] = parent->nuiProcessStop;
	events[1] = parent->nextDepthFrameEvent;
	events[2] = parent->nextVideoFrameEvent;
	events[3] = parent->nextSkeletonEvent;

	while(true){	
		// Wait for an event to be signaled
		WaitForMultipleObjects(sizeof(events)/sizeof(events[0]), events, FALSE, 100);

		// If the stop event is set, stop looping and exit
		if (WAIT_OBJECT_0 == WaitForSingleObject(parent->nuiProcessStop, 0)) {			
			break;
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(parent->nextVideoFrameEvent, 0)) {
			parent->videoAlert();
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(parent->nextDepthFrameEvent, 0))  {
			// if we have received any valid new depth data we may need to draw
			parent->depthAlert();        

		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(parent->nextSkeletonEvent, 0)) {			
			parent->skeletonAlert();
		}		
	}
	return (0);	
}

void KinectManager::depthAlert() {		
	const NUI_IMAGE_FRAME * depthFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);

	if(FAILED(hr)) {		
		cout << "Cannot get next DEPTH frame!" << endl;
		return;
	} 

	INuiFrameTexture * pTexture = depthFrame->pFrameTexture;
	// lastDepthFrameTexture = depthFrame->pFrameTexture; 

	NUI_LOCKED_RECT LockedRect; // see coments in videoAlert
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if(LockedRect.Pitch != 0) {		
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;

		// Copy depth frame to face tracking
		memcpy(depthFTBuffer->GetBuffer(), PBYTE(LockedRect.pBits), min(depthFTBuffer->GetBufferSize(), UINT(pTexture->BufferLen())));

		// draw the bits to the bitmap
		RGBQUAD * rgbrun = m_rgbWk;

		USHORT * pBufferRun = (USHORT*) pBuffer;

		for (int i = 0; i < 320*240; i++){				
			RGBQUAD quad = Nui_ShortToQuad_Depth(*pBufferRun);
			pBufferRun++;
			*rgbrun = quad;
			rgbrun++;
		}

		// D3D -> OGL
		int t = 0;
		for(int i = 0 ; i < 320*240 ; i++) { // todo: make this a function
			oglDepthPixels[i*3] = ((BYTE*) m_rgbWk)[t+2];
			oglDepthPixels[i*3+1] = ((BYTE*) m_rgbWk)[t+1];
			oglDepthPixels[i*3+2] = ((BYTE*) m_rgbWk)[t+0];			
			t+=4;		
		}
	}
	else {
		cout << "Buffer length of received texture is bogus\r\n";
	}

	newDepthFrame = true;

	NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);
}

void KinectManager::videoAlert() {
	newVideoFrame = true;

	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;
	// const NUI_IMAGE_FRAME* imageFrame = NULL;  
	// in the example, NuiImageCamera::NuiImageStreamGetNextFrame() is called, while here we call NuiSensor::NuiImageStreamGetNextFrame();
	// both methods seem to do the same, but one takes a const ** parameter, while the other takes a * parameter (?!).

	// Attempt to get the color frame
	hr = kinect->NuiImageStreamGetNextFrame(videoStreamHandle, 0, &imageFrame); 

	if (FAILED(hr)) {
		cout << "Cannot get next VIDEO frame!" << endl;
		return;
	}

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0) {                	
		pBuffer = (BYTE*) LockedRect.pBits;	// memory buffer, sRGB space, XRGB (32 bpp)

		// Copy video frame to face tracking
		if (faceTrackerBuffersSetUp) {			
			memcpy(videoFTBuffer->GetBuffer(), PBYTE(LockedRect.pBits), min(videoFTBuffer->GetBufferSize(), UINT(pTexture->BufferLen())));
		}

		// convert pixels from XRGB (D3D) to RGB (OGL)		
		int t = 0;

		for (int i = 0; i < 640*480; i++) {
			oglVideoPixels[i*3] = pBuffer[t+2];
			oglVideoPixels[i*3+1] = pBuffer[t+1];
			oglVideoPixels[i*3+2] = pBuffer[t+0];			
			t+=4;				
		}

	} else {
		cout << "Buffer length of received texture is wrong" << endl;
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	// Release the frame
	kinect->NuiImageStreamReleaseFrame(videoStreamHandle, &imageFrame);
}

void KinectManager::skeletonAlert() {
	NUI_SKELETON_FRAME SkeletonFrame = {0};
	HRESULT hr = NuiSkeletonGetNextFrame(0, &SkeletonFrame); // i'm not storing the skeletons cause we are not using them. 
	// When in need it is needed to store them skeletons instead of only head and neck.

	if(FAILED(hr)) {
		cout << "ERROR getting skeleton frame" << endl;
		return;
	}

	for(int i = 0 ; i < NUI_SKELETON_COUNT; i++) { // for each skeleton trackable (6?)

		// if the skeleton is tracked, and the head and neck are tracked (what we need for the FT).
		if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
			NUI_SKELETON_POSITION_TRACKED == SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] &&
			NUI_SKELETON_POSITION_TRACKED == SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER]) {

				skeletonTracked[i] = true; // flag
				heads[i].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
				heads[i].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
				heads[i].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
				necks[i].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
				necks[i].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;
				necks[i].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;

				newSkeleton = true;

		} else {
			heads[i] = necks[i] = FT_VECTOR3D(0, 0, 0);
			skeletonTracked[i] = false;
		}
	}
}


void KinectManager::getImageCoordinatesFromSkeletonCoordinates(Vector4 joint, LONG *x, LONG *y) {
	LONG depthX, depthY;
	USHORT depthZ;

	NuiTransformSkeletonToDepthImage(joint, &depthX, &depthY, &depthZ);		

	NuiImageGetColorPixelCoordinatesFromDepthPixel(
		NUI_IMAGE_RESOLUTION_640x480, NULL,
		depthX * 320 + 0.5f, depthY * 240 + 0.5f, // depth coordinate
		depthZ,  // depth value
		x, y);  // color coordinate
}

RGBQUAD KinectManager::Nui_ShortToQuad_Depth(USHORT s){
	USHORT RealDepth = (s & 0xfff8) >> 3;
	USHORT user = s & 7;

	// transform 13-bit depth information into an 8-bit intensity appropriate for display (we do disregard information in most significant bit)
	BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

	RGBQUAD q;
	q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

	switch(user) {
	case 0:
		q.rgbRed = l / 2;
		q.rgbBlue = l / 2;
		q.rgbGreen = l / 2;
		break;
	case 1:
		q.rgbRed = l;
		break;
	case 2:
		q.rgbGreen = l;
		break;
	case 3:
		q.rgbRed = l / 4;
		q.rgbGreen = l;
		q.rgbBlue = l;
		break;
	case 4:
		q.rgbRed = l;
		q.rgbGreen = l;
		q.rgbBlue = l / 4;
		break;
	case 5:
		q.rgbRed = l;
		q.rgbGreen = l / 4;
		q.rgbBlue = l;
		break;
	case 6:
		q.rgbRed = l / 2;
		q.rgbGreen = l / 2;
		q.rgbBlue = l;
		break;
	case 7:
		q.rgbRed = 255 - (l / 2);
		q.rgbGreen = 255 - (l / 2);
		q.rgbBlue = 255 - (l / 2);
	}

	return q;
}

void KinectManager::stop() {
	// Stop the Nui processing thread
	if(nuiProcessStop != NULL) {
		// Signal the thread
		SetEvent(nuiProcessStop);

		// Wait for thread to stop 
		if(nuiProcess != NULL) {
			WaitForSingleObject(nuiProcess,INFINITE);
			CloseHandle(nuiProcess);
			nuiProcess = NULL;
		}

		CloseHandle(nuiProcessStop);
		nuiProcessStop = NULL;
		cout << "closed the nuiProcess. " << endl;
	}

	if (kinectInitialized) {
		cout << "calling NuiShutDown() it might hung. There is a known issue, see http://msdn.microsoft.com/en-us/library/nuiapi.nuishutdown.aspx" << endl;
		NuiShutdown();
		cout << "nui shut down OK. " << endl;
		// todo there is an issue with this method. See http://msdn.microsoft.com/en-us/library/nuiapi.nuishutdown.aspx
	}
	kinectInitialized = false;

	if (nextSkeletonEvent && nextSkeletonEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(nextSkeletonEvent);
		nextSkeletonEvent = NULL;
		cout << "skel off" << endl;
	}

	if (nextDepthFrameEvent && nextDepthFrameEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(nextDepthFrameEvent);
		nextDepthFrameEvent = NULL;
		cout << "depth off" << endl;
	}

	if (nextVideoFrameEvent && nextVideoFrameEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(nextVideoFrameEvent);
		nextVideoFrameEvent = NULL;
		cout << "video off" << endl;
	}

	if (videoFTBuffer) {
		videoFTBuffer->Release();
		videoFTBuffer = NULL;
		cout << "vidftbuff off" << endl;
	}

	if (depthFTBuffer) {
		depthFTBuffer->Release();
		depthFTBuffer = NULL;
		cout << "depthftbuff off" << endl;
	}
}