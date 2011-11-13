#ifndef _STTG_H_
#define _STTG_H_

#include "ofMain.h"
#include "ofxGSTTTranscriptionThread.h"
#include "ofxTimer.h"
#include "sndfile.h"

class ofxGSTT{
public:
	ofxGSTT();

	void setup(int sampleRate, float _volumeThreshold = 0.05);
	void setListening(bool listen = true);

	bool isRecording();

	void audioIn(ofAudioEventArgs & event);

	//GUI settings
	float volumeThreshold;

protected:
	void prepareRecording();
	void finishRecording();

	vector<ofxGSTTTranscriptionThread*> transcriber;

	/*** SOUND INPUT ***/
	int bufferSize;
	int sampleRate;
	vector<float> left;
	vector<float> right;
	float smoothedVol;
	bool bListen;

	/*** SOUND RECORDING ***/
	bool bActiveVolume;
	bool bRecording;
	bool bRecordingBlocked;
	int transcriptorId;
	ofxTimer timerRecording;
	SF_INFO info;
	SNDFILE* outfile;
};

#endif
