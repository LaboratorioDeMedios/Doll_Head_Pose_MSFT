/**

faces walrus
http://laurenzo.net
*/

#pragma once

#define MAX_EQUIVALENT_FACES 3 // faceswalrus oval set 5 ok
#define MAX_FACE_TYPES 9999
#define DEFAULT_TOLERANCE 3
#define COMPARATION_STYLE_DEFAULT 0
#define COMPARATION_STYLE_AU_ONLY 1
#define COMPARATION_STYLE_ROTATION_ONLY 2
#define COMPARATION_STYLE_ROTATION_AND_MOUTH 3
 
#include "ofMain.h"
#include "FaceFrameWalrus.h"
#include "ofxXmlSettings.h"
#include "FaceAnalizer.h"

#define PROBABILITY_OF_NOT_SUBSTITUTING_AN_EXISTING_FACE 0.95	

typedef vector < vector<FaceFrameWalrus> > FaceMatrix;

class FacesManagerWalrus { 

public:

	FacesManagerWalrus(int maxEq = MAX_EQUIVALENT_FACES);
	void setMaxEquivalentFaces (int maxEqFaces);
		 		
	int getNumberOfFaces();

	FaceFrameWalrus getFaceFrame (int which, int instance);
	 
	FaceFrameWalrus getEquivalentFace (FaceFrameWalrus f, bool * foundEquivalent, int style, bool add = true);

	void load(string filename, int style); // saves an xml and the images
	void save(string filename = "");  // loads the images from img files listed in the xml		

	int maxEquivalentFaces;

private:

	ofxXmlSettings XML;
	void addFaceToVector(vector<FaceFrameWalrus> * vec, FaceFrameWalrus f);

	FaceMatrix faces;
	FaceAnalizer faceAnalizer;


};