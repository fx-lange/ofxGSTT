#ifndef _STTGTRANSCRIPTOR_H_
#define _STTGTRANSCRIPTOR_H_

#include "ofMain.h"
#include "ofThread.h"
#include "googleResponseParser.h"

#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"
#include <curl/curl.h>

#define READSIZE 1024


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

struct extraData{
	int id;
	long timestamp;
	ofxGSTTEvents * events;
};

static size_t writeResponseFunc(void *ptr, size_t size, size_t nmemb, extraData * data)
{
  char jsonLine[1024];
  size_t cpySize = size*nmemb;
  if(cpySize > 1024){
	  cpySize =  1024;
  }
  memcpy(jsonLine, ptr, size*nmemb);
  printf("ID: %i ANTWORT: %s",data->id,jsonLine);

  //EVENT MESS
  ofxGSTTResponseArgs response;
  response.threadId = data->id;
  response.tSend = data->timestamp;
  response.tReceived = ofGetSystemTime();
//TODO json -> string

  googleResponseParser parser;
  parser.parseJSON((char*)ptr);
  response.msg = parser.utterance;
  response.confidence = parser.confidence;
  ofNotifyEvent(data->events->gsttApiResponseEvent, response);
  return size*nmemb;
}

static size_t writeResponseFunc(void *ptr, size_t size, size_t nmemb, extraData * data);

class ofxGSTTTranscriptor : protected ofThread{
public:
	ofxGSTTTranscriptor(int id,ofxGSTTEvents * events);
	virtual ~ofxGSTTTranscriptor(){ }

	void setFilename(char filename[]);

	void startTranscription();
	void stopTranscribing();
	void reserve();
	bool isFree();
	bool isFinished();

	static ofEvent<ofxGSTTResponseArgs> gsttApiResponseEvent;

protected:
	int id;
	char wavFile[64];
	char flacFile[64];
	bool bFinished;
	bool isEncoded;
	bool bFree;
	ofxGSTTEvents * events;

	virtual void threadedFunction();

	bool flacToGoogle();

	bool encodeToFlac();
	//ENCODE TO FLAC STUFF
	void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);

	unsigned total_samples; /* can use a 32-bit number due to WAVE size limitations */
	FLAC__byte buffer[READSIZE/*samples*/ * 2/*bytes_per_sample*/ * 2/*channels*/]; /* we read the WAVE data into here */
	FLAC__int32 pcm[READSIZE/*samples*/ * 2/*channels*/];
};



#endif
