/**
Interface to the chromation board
it should implement a common interface with the arduino based solution... TODO

tomas laurenzo - tomas@laurenzo.net
*/

#pragma once

#include "ofMain.h"

class Chromation {

public:
	
	Chromation();
	Chromation (string port);
		
	bool sendPower (Byte port, Byte power);
	void stop();
	string getPort();
	
	void allOn();
	void allOff();

private:
	ofSerial serial;
	string port;	
};
