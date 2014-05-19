/*

This class manages all the actual interaction with the kinect sensor.

It feeds the unsigned char * oglVideoPixels and unsigned char * oglDepthPixels which are used by the OF app.

tomas laurenzo
tomas@laurenzo.net

*/

#pragma once

#include "windows.h"
#include <NuiApi.h>
#include "ofMain.h"
#include <FaceTrackLib.h>

class KinectManager {

public:
	KinectManager ();
		
	HRESULT toggleNearMode();
	void toggleSeated();

	bool getSeated();
	bool getNearMode();

	HRESULT setup();
	void stop();

	void modifyAngle(long);
	long getAngle();

	void getImageCoordinatesFromSkeletonCoordinates(Vector4 joint, LONG *x, LONG *y);

	// skeleton
	FT_VECTOR3D heads[NUI_SKELETON_COUNT];
	FT_VECTOR3D necks[NUI_SKELETON_COUNT];  
	bool skeletonTracked[NUI_SKELETON_COUNT];

	// communication with the facetracker
	IFTImage * videoFTBuffer, * depthFTBuffer; //todo create getter and setter
	bool faceTrackerBuffersSetUp;

	// communication with parent
	unsigned char * getOglVideoPixels();
	unsigned char * getOglDepthPixels();


private:
	ofBaseApp * parent;
	INuiSensor* kinect;	

	// kinect	
	long kinectAngle; 
	bool nearMode;
	RGBQUAD m_rgbWk[640*480];
	RGBQUAD Nui_ShortToQuad_Depth(USHORT);
	bool kinectInitialized;		

	HANDLE nextDepthFrameEvent, nextVideoFrameEvent, nextSkeletonEvent; // kinect events 	
	HANDLE depthStreamHandle, videoStreamHandle; // kinect streams	
	static DWORD WINAPI nuiProcessThread(LPVOID pParam); // data processing thread	
	HANDLE nuiProcess, nuiProcessStop; // thread handling

	
	BYTE * pBuffer;

	bool newVideoFrame;
	bool newDepthFrame;
	bool newSkeleton;

	// get skeleton todo ();


	// kinect events (alerts)
	void skeletonAlert();
	void depthAlert();
	void videoAlert();

	// skeleton
	bool seated;
		

	// of + opengl
	unsigned char *oglVideoPixels;
	unsigned char *oglDepthPixels;	

};