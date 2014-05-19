#pragma once

#include "windows.h"
#include <NuiApi.h>
#include "ofMain.h"
#include <FaceTrackLib.h>
#include "KinectManager.h"
#include "FaceData.h"

/**
This class communicates our OF program with MSR's FaceTracker.
*/

#define MOUTH_OPEN_POINTS_THRESHOLD 13 

class FaceTrackerManager {

public:
	FaceTrackerManager(KinectManager *kinectManager);
	
	// how to center a point
	void getCenteredDeltas(FT_VECTOR2D p[], int, int &x, int &y);
	void getCenteredDeltas(int &x, int &y); // defaulting in facepoints and facepointcount

	// the results of the head tracking 
	float faceScale, facePos[3], faceRot[3];  	// tracked face (hardcoded onto only one user)

	// the tracked face points (100)
	FT_VECTOR2D * facePoints; // hardcoded, only one face  todo change that si that we hold many faces
	UINT facePointCount; // amount of points

	// the tracked face vertex (121)
	UINT vertexCount; // amount of vertex
	FT_VECTOR2D* faceVertices; // harcoded again: only one face. 
	bool gettingVertices;
	
	// the animation units 
	FLOAT * animUnits;
	UINT animUnitCount; // amount ot AUs

	void update();	// tick
	void stop(); // clean up
	
	bool lastTrackSucceeded; 

	bool mouthOpen(FT_VECTOR2D * facePoints, UINT facePointCount, FLOAT * animUnits, UINT animUnitCount);

	void drawSuperImposedVertices();
	void drawSuperImposedPoints();
	
	FaceData getFaceData();

private:
	
	IFTFaceTracker* faceTracker;
	bool faceTracking;
	
	IFTResult * FTResult;	
	IFTImage * videoFTFrame, * depthFTFrame;

	HRESULT getClosestHint(FT_VECTOR3D* pHint3D);
	void processFaceTrackingResult(IFTResult *res);	
	
	void processAUs(FLOAT * animUnits, UINT animUnitCount);	
		
	bool faceTrackerSetUp;
	
	KinectManager *kinectManager;
};