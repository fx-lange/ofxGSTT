#ifndef _STTGTRANSCRIBER_H_
#define _STTGTRANSCRIBER_H_

#include "ofMain.h"
#include "ofThread.h"
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"
#include <curl/curl.h>

#define READSIZE 1024

class ofxGSTTTranscriptor : protected ofThread{
public:
	ofxGSTTTranscriptor();
	virtual ~ofxGSTTTranscriptor(){ }

	void setFilename(char filename[]);

	void startTranscription();
	void stopTranscribing();
	void reserve();
	bool isFree();
	bool isFinished();

protected:
	int id;
	char wavFile[64];
	char flacFile[64];
	bool bFinished;
	bool isEncoded;
	bool bFree;

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