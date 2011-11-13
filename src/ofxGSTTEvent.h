/**
 * Custom Event for Googles Response
 * */

#ifndef _GSSTEVENTS_H_
#define _GSSTEVENTS_H_

#include "ofMain.h"

class ofxGSTTResponseArgs: public ofEventArgs {
public:
	ofxGSTTResponseArgs(){
		threadId = 0;
		confidence = 0.0;
		msg = "";
		source = 0;
		tSend = tReceived = -1;
		status = -1;
	}

	int status;
	int threadId;
	long tSend;
	long tReceived;
	int source;
	string msg;
	float confidence;
};

extern ofEvent<ofxGSTTResponseArgs> gsttApiResponseEvent;

#endif
