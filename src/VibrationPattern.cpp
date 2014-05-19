#include "VibrationPattern.h"

VibrationPattern::VibrationPattern() {
	cursor = 0;
	length = 0;
	framesCounter = 0;
}

bool VibrationPattern::playing() {
	return cursor < length;
}

void VibrationPattern::tick() {	
	// we advance the cursors every VIBRATION_PATTERN_FRAMES_PER_TICK frames	
	if (framesCounter < VIBRATION_PATTERN_FRAMES_PER_TICK) {
		framesCounter++;
	} else {
		framesCounter = 0;
		cursor++;
	}
}


vector<int> VibrationPattern::getValues() {	
	return getValues(cursor);	
}

vector<int> VibrationPattern::getValues(int t) {
	return powers[t];
}


/*
								0 1 2
								3 4 5
								6 7 8
*/
void VibrationPattern::init(int patternId) {		
	cursor = 0; 
	vector<int> temp;
	temp.resize(9);

	switch (patternId) {			

	case VIBRATION_PATTERN_TOP_RIGHT:
		length = 5;
		zeroAll ();
		powers[0][2] = 255;
		powers[1][1] = powers[1][5] = 255;
		powers[2][0] = powers[2][4] = powers[2][8] = 255;
		powers[3][3] = powers[3][7] = 255;
		powers[4][6] = 255;		
		break;		

	case VIBRATION_PATTERN_CENTER_RIGHT:
		length = 3;
		zeroAll();
		powers[2][0] = powers[2][3] = powers[2][6] = 255;
		powers[1][1] = powers[1][4] = powers[1][7] = 255;
		powers[0][2] = powers[1][5] = powers[0][8] = 255;
		break;

	case VIBRATION_PATTERN_BOTTOM_RIGHT:
		length = 5;
		zeroAll ();
		powers[0][8] = 255;
		powers[1][7] = powers[1][5] = 255;
		powers[2][6] = powers[2][4] = powers[2][2] = 255;
		powers[3][3] = powers[3][1] = 255;
		powers[4][0] = 255;		
		break;

	case VIBRATION_PATTERN_TOP_LEFT:
		length = 5;
		zeroAll ();
		powers[0][0] = 255;
		powers[1][1] = powers[1][3] = 255;
		powers[2][6] = powers[2][4] = powers[2][2] = 255;
		powers[3][5] = powers[3][7] = 255;
		powers[4][8] = 255;		
		break;

	case VIBRATION_PATTERN_BOTTOM_LEFT:
		length = 5;
		zeroAll ();
		powers[0][6] = 255;
		powers[1][7] = powers[1][3] = 255;
		powers[2][0] = powers[2][4] = powers[2][8] = 255;
		powers[3][5] = powers[3][1] = 255;
		powers[4][2] = 255;		
		break;


	case VIBRATION_PATTERN_CENTER_LEFT:
		length = 3;
		zeroAll();
		powers[0][0] = powers[0][3] = powers[0][6] = 255;
		powers[1][1] = powers[1][4] = powers[1][7] = 255;
		powers[2][2] = powers[2][5] = powers[2][8] = 255;
		break;	

	case VIBRATION_PATTERN_TOP_CENTER:
		length = 3;
		zeroAll();
		powers[0][0] = powers[0][1] = powers [0][2] = 255;
		powers[1][3] = powers[1][4] = powers [1][5] = 255;
		powers[2][6] = powers[2][7] = powers [2][8] = 255;

	case VIBRATION_PATTERN_BOTTOM_CENTER:
		length = 3;
		zeroAll();
		powers[2][0] = powers[2][1] = powers [2][2] = 255;
		powers[1][3] = powers[1][4] = powers [1][5] = 255;
		powers[0][6] = powers[0][7] = powers [0][8] = 255;
	}
}

void VibrationPattern::zeroAll() {
	powers.clear();

	for (int i = 0; i < length; i++) {
		vector<int> temp;
		temp.resize(9);

		for (int j = 0; j < 9; j++) {
			temp[j] = 0;		
		}

		powers.push_back(temp);
	}	
}
