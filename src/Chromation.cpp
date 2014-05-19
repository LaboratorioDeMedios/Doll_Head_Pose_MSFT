#include "Chromation.h"

Chromation::Chromation() {
	serial.listDevices();
	serial.setup("COM3", 19200);
	port = "COM3";
}

Chromation::Chromation (string port) {
	serial.setup(port, 19200);
	this->port = port;
}

string Chromation::getPort(){
	return port;
}

void Chromation::allOn() {
	sendPower(255, 255);
}

void Chromation::allOff() {
	sendPower(255, 0);
}


bool Chromation::sendPower (Byte port, Byte power) {
	// cout << "sending " << (int)power << " to port " << (int)port << endl;
	unsigned char command[6];

	command[0] = 0xAA;
	command[1] = 0x55;
	command[2] = 0x00;
	command[3] = (Byte)port;
	command[4] = power;
	command[5] = (Byte)(0xFF + (Byte)port + power);

	serial.writeBytes(command, 6);
	
	unsigned char res[6];
	memset(res, 0, 6);

	int nTimesRead = 0;
	int nBytesRead = 0;
	int nRead  = 0; 

	unsigned char bytesReturned[6];
	memset(bytesReturned, 0xFF, 6);

	while ((nRead = serial.readBytes(bytesReturned, 6)) > 0){
		nTimesRead++;	
		nBytesRead = nRead;
	};

	return res[2] == 0x00;
}

void Chromation::stop() {
	sendPower(255, 0);
}