#ifndef _STTGTRANSCRIPTOR_H_
#define _STTGTRANSCRIPTOR_H_

#include "ofMain.h"
#include "ofThread.h"
#include "googleResponseParser.h"
#include "ofxGSTTEvent.h"

#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"
#include <curl.h>

#define READSIZE 1024

using namespace std;

struct callBackData{
	int id;
	int deviceId;
	long timestamp;
};

//callback function for curl response
static size_t writeResponseFunc(void *ptr, size_t size, size_t nmemb, callBackData * data){
	ofxGSTTResponseArgs response;
	response.threadId = data->id;
	response.tSend = data->timestamp;
	response.tReceived = ofGetSystemTime();
	response.deviceId = data->deviceId;

//	char text[1024];
//	memcpy(text,ptr,size);
//	printf("TEXT: %s",text);
	//decode via json
	googleResponseParser parser;
	parser.parseJSON((char*) ptr);

	response.msg = parser.utterance;
	response.status = parser.status;
	response.confidence = parser.confidence;

	ofNotifyEvent(gsttApiResponseEvent, response);

	return size * nmemb;
}

class ofxGSTTTranscriptionThread: protected ofThread{
public:
	ofxGSTTTranscriptionThread(int id);
	virtual ~ofxGSTTTranscriptionThread(){
	}

	void setup(int deviceId, string language);

	void setFilename(char filename[]);

	void startTranscription();
	void stopTranscription();
	void reserve();
	bool isFree();
	bool isFinished();

	static ofEvent<ofxGSTTResponseArgs> gsttApiResponseEvent;

protected:
	int id;
	int deviceId;
	char wavFile[64];
	char flacFile[64];
	bool bFinished;
	bool isEncoded;
	bool bFree;

	string url;

	virtual void threadedFunction();

	bool flacToGoogle();

	bool encodeToFlac();
	//ENCODE TO FLAC STUFF
	unsigned total_samples; /* can use a 32-bit number due to WAVE size limitations */
	FLAC__byte buffer[READSIZE/*samples*/* 2/*bytes_per_sample*/* 2/*channels*/]; /* we read the WAVE data into here */
    FLAC__int32	pcm[READSIZE/*samples*/* 2/*channels*/];
};

#endif
