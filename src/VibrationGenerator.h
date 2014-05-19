#pragma once

#include "ofMain.h"
#include "Chromation.h"
#include "FaceAnalizer.h"
#include "FaceData.h"
#include "VibrationPattern.h"

#define NUMBER_OF_MOTORS 9

typedef struct  {
	int x;
	int y;
	int port;
} VibratingMotor;

class VibrationGenerator : public ofBaseSoundInput {
	
public:	
	VibrationGenerator(Chromation *chromation);
	void audioReceived(float * input, int bufferSize, int nChannels);
	
	void playPattern(FaceData faceData);
	
	VibratingMotor motors[NUMBER_OF_MOTORS];

	float averageVolume;

private:
	Chromation * chromation;

	VibrationPattern vp;
	ofSoundStream soundStream;  

	int sampleRate;  
	int startBufferSize;  
	float* buffer;
	
};
