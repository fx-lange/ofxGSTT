#ifndef _STTG_H_
#define _STTG_H_

#include "ofMain.h"
#include "ofxGSTTTranscriber.h"

class ofxGSTT{
public:
	ofxGSTT(){ }
protected:
	vector<ofxGSTTTranscriber*> transcriber;
};

#endif
