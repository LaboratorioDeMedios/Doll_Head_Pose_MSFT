#include "VibrationGenerator.h"

VibrationGenerator::VibrationGenerator(Chromation * chromation) {	 
	this->chromation = chromation;

	
	/*

	The motors are identified in the pattern as follows: 

		0 1 2
		3 4 5
		6 7 8

	but the actual motor ports are:
		
		Byte g_portGrid[4][3]= 
			{
				{4, 5, 6}, 
				{8, 9, 7}, 
				{10,13,15}, 
				{11,12,14}
		   };

	 we will use only the top 9, so we define the ports[9] as follows:
	*/

	int ports[9] = {4, 5, 6, 8, 9, 7, 10, 13, 15}; // the actual ports
	
	int distanceBetweenMotors = 10; // for some realtime calculation that we might want to do

	int t = 0;

	for (int j = -1; j < 2; j++) {
		for (int i = -1; i < 2; i++) {
			motors[t].x = i * distanceBetweenMotors;
			motors[t].y = j * distanceBetweenMotors;
			motors[t].port = ports[t];
			t++;
		}
	}


	// set up sound
	soundStream.setInput(this);   
	
    startBufferSize = 256;  

    sampleRate = 44100;        

    buffer = new float[startBufferSize];  
    memset(buffer, 0, startBufferSize * sizeof(float));        

	cout << "sound devices: ";
	// soundStream.listDevices();
	cout << endl;

    soundStream.setup(0, 1, sampleRate, startBufferSize, 4);
	
	averageVolume = 0;
}

void VibrationGenerator::audioReceived(float * input, int bufferSize, int nChannels ){            
      
    if( startBufferSize != bufferSize ){  
        ofLog(OF_LOG_ERROR, "your buffer size was set to %i - but the stream needs a buffer size of %i", startBufferSize, bufferSize);  
        return;  
    }     
      
	float temp = 0;
    // copy the input to the buffer and calculate average volume
    for (int i = 0; i < bufferSize; i++){  
        buffer[i] = input[i];  
		temp += input[i] * input[i];  		// rms
    }

	temp /= bufferSize;
	temp = sqrt(temp);

	/* // smooth
	averageVolume *= 0.93;
	averageVolume += 0.07 * temp;
	*/

	// not smooth
	averageVolume = temp;
	
}  

void VibrationGenerator::playPattern(FaceData fd) {
	/*	
		if not pattern playing: choose a pattern and play it.
			to choose a pattern we use the rotation: 9 quadrants. boring. i know. i'm sorry. i'm bored too.
		if pattern playing: tick it	
	*/
	
	// this "borders" are used to determine the cuadrant the head is at, i.e. to select a VIBRATION_PATTERN
	// yaw centers: 32, 13, -23  borders: 22, -3	
	// pitch centers: 30, 11, -5  borders: 20, 4
	int leftBorder = 22;  
	int rightBorder = -3;
	int topBorder = 20;
	int bottomBorder = 4;

	int pitch = fd.faceRot[0];
	int yaw = fd.faceRot[1];
	int pattern;

	vp.tick();	// next values

	if (!vp.playing()) {		// if no more values, start with the new post
		// calculate the corresponding id
		if (yaw > leftBorder) { // left
			if (pitch > topBorder) { // top
				pattern = VIBRATION_PATTERN_TOP_LEFT;
			} else if (pitch > bottomBorder) {
				pattern = VIBRATION_PATTERN_CENTER_LEFT;
			} else pattern = VIBRATION_PATTERN_BOTTOM_LEFT;

		} else if (yaw > rightBorder) {
			if (pitch > topBorder) { // top
				pattern = VIBRATION_PATTERN_TOP_CENTER;
			} else if (pitch > bottomBorder) {
				pattern = VIBRATION_PATTERN_CENTER_CENTER;
			} else pattern = VIBRATION_PATTERN_BOTTOM_CENTER;

		} else {
			if (pitch > topBorder) { // top
				pattern = VIBRATION_PATTERN_TOP_RIGHT;
			} else if (pitch > bottomBorder) {
				pattern = VIBRATION_PATTERN_CENTER_RIGHT;
			} else pattern = VIBRATION_PATTERN_BOTTOM_RIGHT;
		}
		
		vp.init(pattern);
	}
		
	vector<int> vals = vp.getValues();

	// cout << "\t- avg vol * 100: " << averageVolume * 10 << endl;	
	for (int i = 0; i < NUMBER_OF_MOTORS; i++) {	
		int res = vals[i] * averageVolume * 10;				
		//ofMap(res, 0, 1, 0, vals[i]);
		// cout << res << " ";		
		chromation->sendPower(motors[i].port, res);				
	}
	
}