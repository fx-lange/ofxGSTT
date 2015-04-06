#pragma once

#include "ofMain.h"
#include "ofThread.h"
#include "googleResponseParser.h"
#include "ofxGSTTEvent.h"
#include "ofxSSL.h"

#define READSIZE 1024

using namespace std;

struct callBackData{
	int id;
	int deviceId;
	long timestamp;
};

class ofxGSTTTranscriptionThread: protected ofThread{
public:
	ofxGSTTTranscriptionThread(int id);
	virtual ~ofxGSTTTranscriptionThread(){
	}

	void setup(int deviceId, string language, string key);

	void setFilename(char filename[]);

	void startTranscription();
	void stopTranscription();
	void reserve(); //TODO mutex?
	bool isFree();
	bool isFinished();

	static ofEvent<ofxGSTTResponseArgs> gsttApiResponseEvent;

protected:
	int id;
	int deviceId;
	char wavFile[128];
	bool bFinished;
	bool bFree;

	virtual void threadedFunction();

	ofxSSL curl;
	void sendGoogleRequest();
};
