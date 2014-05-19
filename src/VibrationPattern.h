#pragma once

#include "ofMain.h"
 

typedef vector < vector<int> > PowerMatrix;

#define VIBRATION_PATTERN_TOP_LEFT 0			
#define VIBRATION_PATTERN_CENTER_LEFT 1
#define VIBRATION_PATTERN_BOTTOM_LEFT 2
#define VIBRATION_PATTERN_TOP_CENTER 3
#define VIBRATION_PATTERN_CENTER_CENTER 4
#define VIBRATION_PATTERN_BOTTOM_CENTER 5
#define VIBRATION_PATTERN_TOP_RIGHT 6
#define VIBRATION_PATTERN_CENTER_RIGHT 7
#define VIBRATION_PATTERN_BOTTOM_RIGHT 8

#define VIBRATION_PATTERN_FRAMES_PER_TICK 5

class VibrationPattern {
public:
	int nMotors;
	int length;

	PowerMatrix powers;
	int cursor;

	VibrationPattern();

	vector<int> getValues(int t);
	vector<int> getValues();
	void tick();

	bool playing ();

	void init (int patternId);

private:
	int framesCounter;
	void zeroAll();
	
};