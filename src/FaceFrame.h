#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include <FaceTrackLib.h>
#include "tom.h"
#include "FaceData.h"

#define MAX_AGE 2000.0 // one per frame. It might be reasonable to change it to seconds instead of ticks
// as of now, expectancy is ranfom(2 * max / 3, max)

#define SIZE_MULTIPLIER 2.5f
#define JITTER 200

using namespace tom;

class FaceFrame : public FaceData {

public:	 	

	void updateScreenPos();

	FaceFrame(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
		float faceScale, float facePos[3], float faceRot[3], 
		FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
		FLOAT * animUnits, UINT animUnitCount, bool jittered = true);

	FaceFrame(float fullFrameWidth, float fullFrameHeight,
		float faceScale, float facePos[3], float faceRot[3], 
		FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
		FLOAT * animUnits, UINT animUnitCount, bool jittered = true);
	
	ofImage faceImage; // the face cut out
  		 
	void render();
	void render(int alpha);
	void render(float x, float y, int alpha = 255);	
	void tick();

	bool alive();

	inline int getAge(){ return age; };
	inline int getExpectancy(){ return expectancy; };
	inline bool isBlinking() { return blinking; };	

	void translate (int x, int y);

	void setFaceImage(unsigned char * fullFramePixels);

	string saveImage();

private:	
	int age;
	int expectancy;
	bool blinking;

	void init (float fullFrameWidth, float fullFrameHeight,
		float faceScale, float facePos[3], float faceRot[3], 
		FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
		FLOAT * animUnits, UINT animUnitCount, bool jittered);

	string fileName;

};