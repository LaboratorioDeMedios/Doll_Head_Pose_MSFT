#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include <FaceTrackLib.h>

class FaceData {

public:

	// the results of the head tracking 
	float faceScale, facePos[3], faceRot[3];  	// tracked face 

	// the tracked face points (100)
	FT_VECTOR2D * facePoints; 
	UINT facePointCount; // amount of points

	// the tracked face vertex (121)
	UINT vertexCount; // amount of vertex
	FT_VECTOR2D* faceVertices; 
		
	// the animation units 
	FLOAT * animUnits;
	UINT animUnitCount; // amount ot AUs

	int x, y; // on screen (projected) positions.  

	void calculatePosition();
};