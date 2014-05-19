#include "FacesManagerWalrus.h"

FacesManagerWalrus::FacesManagerWalrus(int maxEqFaces) {
	maxEquivalentFaces = maxEqFaces;
}

void FacesManagerWalrus::setMaxEquivalentFaces (int maxEqFaces) {
		maxEquivalentFaces = maxEqFaces;
}

FaceFrameWalrus FacesManagerWalrus::getEquivalentFace(FaceFrameWalrus f, bool * foundEquivalent, int style, bool add) {
	int i = 0;
	if (foundEquivalent != NULL) *foundEquivalent = false;

	switch (style) {

	case COMPARATION_STYLE_ROTATION_ONLY:
		while (i < faces.size() && !f.hasSimilarRot(faces[i][0])) i++;
		break;

	case COMPARATION_STYLE_AU_ONLY:
		while (i < faces.size() && !f.hasSimilarAUs(faces[i][0])) i++;
		break;

	case COMPARATION_STYLE_ROTATION_AND_MOUTH:
		while (i < faces.size() && !(f.hasSimilarRot(faces[i][0]) && faceAnalizer.mouthOpen(f) == faceAnalizer.mouthOpen(faces[i][0]))) 
			i++;
		break;

	case COMPARATION_STYLE_DEFAULT: default:
		while (i < faces.size() && f != faces[i][0]) i++;
		break;
	}

	if (i < faces.size()){ // it's not a new pose
		if (add) {
			addFaceToVector (&faces[i], f);					
		}

		// cout << "f";

		if (foundEquivalent != NULL) *foundEquivalent = true;
		FaceFrameWalrus res = faces[i][ofRandom(0, faces[i].size())]; // get random pose 	
		
		return res;

	} else {  // it's a new pose, we add it and return thyself

		if (add && faces.size() < MAX_FACE_TYPES) {
			vector<FaceFrameWalrus> newFaceVector;
			newFaceVector.push_back(f);
			faces.push_back(newFaceVector);		
			// cout << "*";
		}

		// cout << "n";
		
		return f;
	}
}

void FacesManagerWalrus::addFaceToVector(vector<FaceFrameWalrus> * vec, FaceFrameWalrus f) {
	if (vec->size() < maxEquivalentFaces) {
		// cout << "+";
		vec->push_back(f);	
	}
	else if (ofRandom(0, 1) > PROBABILITY_OF_NOT_SUBSTITUTING_AN_EXISTING_FACE) { // todo parametrize this!!		
		// cout << "x";
		vec->erase(vec->begin());
		vec->push_back(f);
		//(* vec)[ofRandom(0,vec->size())] = p; // replace a random item
	}
}

int FacesManagerWalrus::getNumberOfFaces() {
	return faces.size();
}



void FacesManagerWalrus::save (string filename) {
	
	/* for each and every face
	   add a face tag
			add the following tags and their data:
		 		FACES_SCALE
					X
					Y
					Z
				FACE_ROT
					ROLL
					PITCH
					YAW
				FACE_POINTS
					POINT*
						X
						Y
						Z
					COUNT
				FACE_VERTICES
					VERTEX*
						X
						Y
						Z
				ANIMATION_UNITS
					AU*
					COUNT
				IS_BLINKING
	*/


	XML.clear(); 

	int whichPoses = XML.addTag("faces");
	XML.pushTag("faces", whichPoses);	

	cout << "*** SAVING " << faces.size() << " faces " << endl;
	for (int i = 0; i < faces.size(); i++){ // for each face type

		cout << "faces[" << i << "].size: " << faces[i].size() << endl;

		for (int j = 0; j < faces[i].size(); j++) {  // for each frame

			cout << "\tsaving face " << j << endl;
			int whichPose = XML.addTag("face");
			XML.pushTag("face", whichPose);

			XML.addValue("img", faces[i][j].saveImage()); // saves the actual PNG and returns the filename, which is added to the XML

			XML.addValue("faceScale", faces[i][j].faceScale);

			XML.addTag("facePos");
			XML.pushTag("facePos");
				XML.addValue("x", faces[i][j].facePos[0]);
				XML.addValue("y", faces[i][j].facePos[1]);
				XML.addValue("z", faces[i][j].facePos[2]);
			XML.popTag(); // facePos

			XML.addTag("faceRot");
			XML.pushTag("faceRot");
				XML.addValue("roll", faces[i][j].faceRot[0]);
				XML.addValue("pitch", faces[i][j].faceRot[1]);
				XML.addValue("yaw", faces[i][j].faceRot[2]);
			XML.popTag(); // faceRot		

			XML.addTag("facePoints");
			XML.pushTag("facePoints");				
				XML.addValue("facePointCount", (int)faces[i][j].facePointCount);
				for (int p = 0; p < faces[i][j].facePointCount; p++) {			
					int which = XML.addTag("point");
					XML.pushTag("point", which);
						XML.addValue("x", faces[i][j].facePoints[p].x);
						XML.addValue("y", faces[i][j].facePoints[p].y);
					XML.popTag(); // point
				}
			XML.popTag(); // facePoints

			
			XML.addTag("faceVertices");				
			XML.pushTag("faceVertices");				
				XML.addValue("vertexCount", (int)faces[i][j].vertexCount);
				for (int p = 0; p < faces[i][j].vertexCount; p++) {
					int which = XML.addTag("vertex");
					XML.pushTag("vertex", which);
						XML.addValue("x", faces[i][j].faceVertices[p].x);
						XML.addValue("y", faces[i][j].faceVertices[p].y);
					XML.popTag(); // vertex
				}
			XML.popTag(); // faceVertices			

			XML.addTag("animUnits");				
			XML.pushTag("animUnits");	
				XML.addValue("animUnitCount", (int)faces[i][j].animUnitCount);
				for (int p = 0; p < faces[i][j].animUnitCount; p++) {										
					XML.addValue("animUnit", faces[i][j].animUnits[p]);										
				}
			XML.popTag(); // animUnits
			
			XML.popTag(); // face
		} // for each face
	} // for each face type

	XML.popTag(); // faces

	// saving xml to hdd
	XML.saveFile(filename);
	cout << "FILE SAVED " << endl;
}



void FacesManagerWalrus::load (string filename, int style) {
	if (XML.loadFile(filename)){
		XML.pushTag("faces");

		for (int i = 0; i < XML.getNumTags("face"); i++) {
			XML.pushTag("face", i);

			string img = XML.getValue("img", "");			
			ofImage u;
			if (!u.loadImage(img)) {
				cout << "******* ERROR LOADING IMAGE " << img << endl;
			} else {				
				
				float faceScale = XML.getValue("faceScale", -1);
				int facePointCount = XML.getValue("facePointCount", -1);
				int vertexCount = XML.getValue("vertexCount", -1);
				int animUnitCount = XML.getValue("animUnitCount", -1);
				float facePos[3];
				XML.pushTag("facePos");
					facePos[0] = XML.getValue("x", -1);
					facePos[1] = XML.getValue("y", -1);
					facePos[2] = XML.getValue("z", -1);
				XML.popTag(); // pos
				float faceRot[3];
				XML.pushTag("faceRot");
					faceRot[0] = XML.getValue("roll", -1);
					faceRot[1] = XML.getValue("pitch", -1);
					faceRot[2] = XML.getValue("yaw", -1);
				XML.popTag(); // rot
				XML.pushTag("facePoints");
				FT_VECTOR2D * facePoints;
				facePoints = new FT_VECTOR2D[facePointCount];
				for (int p = 0; p<facePointCount; p++) {
					XML.pushTag("point");
					facePoints[p].x = XML.getValue("x", -1);
					facePoints[p].y = XML.getValue("y", -1);
					XML.popTag();  
				}
				XML.popTag(); //fpoints
				XML.pushTag("faceVertices");
				FT_VECTOR2D * faceVertices;
				faceVertices = new FT_VECTOR2D[vertexCount];
				for (int p = 0; p<vertexCount; p++) {
					XML.pushTag("vertex");
					faceVertices[p].x = XML.getValue("x", -1);
					faceVertices[p].y = XML.getValue("y", -1);
					XML.popTag();  
				}
				XML.popTag(); //faceVertices
				XML.pushTag("animUnits");
				float * animUnits;
				animUnits = new float[animUnitCount];
				for (int p = 0; p<animUnitCount; p++) {					
					animUnits[p] = XML.getValue("animUnit", -1.0f);
				}
				XML.popTag(); //animUnits														

				FaceFrameWalrus ff(u.getPixels(), 640.0f, 480.0f, faceScale, facePos, faceRot, 
				facePoints, facePointCount, faceVertices, vertexCount, 
				animUnits, animUnitCount, FACE_FRAME_MATRIX_MODE);
				
				bool foundEquivalent;
				getEquivalentFace (ff, &foundEquivalent, style, true);			
			}
			
			XML.popTag(); // frame

		} // for each face
		XML.popTag(); // faces
		cout << "XML file loaded" << endl;
	} else { 
		cout << "**** FIRST TIME? file not found" << endl;
	}
}