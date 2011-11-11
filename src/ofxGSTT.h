#ifndef _STTG_H_
#define _STTG_H_

#include "ofMain.h"
#include "ofxGSTTTranscriptor.h"
#include "ofxTimer.h"
#include "sndfile.h"

class ofxGSTT{
public:
	ofxGSTT(ofBaseApp * baseApp);

	bool isRecording();

	void audioIn(ofAudioEventArgs & event);

	//GUI settings
	float soundThreshold;

protected:

	void prepareRecording();
	void finishRecording();

	vector<ofxGSTTTranscriptor*> transcriber;

	/*** SOUND INPUT ***/
	int bufferSize;
	int sampleRate;
	vector <float> left;
	vector <float> right;

	float smoothedVol;
	float scaledVol;
	ofSoundStream soundStream;

	/*** SOUND RECORDING ***/
	bool bActiveVolume;
	bool bRecording;
	bool bRecordingBlocked;
	int recorderId;
	ofxTimer timerRecording;
	SF_INFO info;
	SNDFILE* outfile;
};

#endif
