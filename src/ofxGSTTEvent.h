/**
 * Custom Event for Googles Response
 * */

#ifndef _GSSTEVENTS_H_
#define _GSSTEVENTS_H_

#include "ofMain.h"

class ofxGSTTResponseArgs: public ofEventArgs{
public:
	ofxGSTTResponseArgs(){
		confidence = 0.0f;
		msg = "";
		source = 0;
		tSend = tReceived = -1;
		status = -1;
	}

	int status;
	int deviceId;
	long tSend;
	long tReceived;
	int source;
	string msg;
	float confidence;
};
//
//class ofxAudioDeviceArgs: public ofAudioEventArgs{
//public:
//	ofxAudioDeviceArgs(){
//		deviceId = -1;
//	}
//	int deviceId;
//};
//
//extern ofEvent<ofxAudioDeviceArgs> audioDeviceEvent; //TODO hat hier in der Klasse nichts zu suchen!

#endif
