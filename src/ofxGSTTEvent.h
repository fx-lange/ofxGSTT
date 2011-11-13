#ifndef _GSSTEVENTS_H_
#define _GSSTEVENTS_H_

#include "ofMain.h"

class ofxGSTTResponseArgs : public ofEventArgs{
  public:
	int threadId;
	long tSend;
	long tReceived;
	int source;
	string msg;
	float confidence;
};

class ofxGSTTEvents{
public:
	ofEvent<ofxGSTTResponseArgs> gsttApiResponseEvent;
};

#endif
