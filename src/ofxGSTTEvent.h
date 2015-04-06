/**
 * Custom Event for Googles Response
 * */

#pragma once

#include "ofMain.h"

class ofxGSTTResponseArgs: public ofEventArgs{
public:
	int deviceId;
	long tSend;
	long tReceived;
	int source;
	string msg;
	float confidence;
};
