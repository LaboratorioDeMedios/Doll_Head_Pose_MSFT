#pragma once

#include "FaceFrame.h"

using namespace tom;
#define FACE_FRAME_MATRIX_MODE 0
#define FACE_FRAME_OVAL_MODE 1

class FaceFrameWalrus : public FaceFrame {

public:

	FaceFrameWalrus(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
		float faceScale, float facePos[3], float faceRot[3], 
		FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
		FLOAT * animUnits, UINT animUnitCount, bool jittered = false);

	FaceFrameWalrus(unsigned char * fullFramePixels, float fullFrameWidth, float fullFrameHeight,
		float faceScale, float facePos[3], float faceRot[3], 
		FT_VECTOR2D * facePoints, UINT facePointCount, FT_VECTOR2D * faceVertices, UINT vertexCount, 
		FLOAT * animUnits, UINT animUnitCount, int mode, bool jittered = false);

	void draw();
	void draw(int x, int y);
	void drawCentered(int w, int h);
	void draw(int x, int y, int w, int h);

	void setFaceImageFullFrame(unsigned char * fullFramePixels);
	void setFaceImageRectangle(unsigned char * fullFramePixels, int w, int h);
	void setFaceImageOval(unsigned char * fullFramePixels);

	bool operator==(const FaceFrameWalrus& otherFace) const;
	bool operator!=(const FaceFrameWalrus& otherFace) const;

	bool hasSimilarRot (const FaceFrameWalrus &otherFace) const;

	bool hasSimilarAUs (const FaceFrameWalrus& otherFace) const;
};