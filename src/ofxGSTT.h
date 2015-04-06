#pragma once

#include "ofMain.h"
#include "ofxGSTTTranscriptionThread.h"
#include "ofxTimer.h"
#include "sndfile.h"

#define OFXGSTT_DEFAULTDEVICE_ID 0

using namespace std;


/* TODO nice to have:
 	 SF_INFO per device - so each device can have its own settings!
 	 thread safe?
 	 naming
 	 volume calculation, smart detection instead of threshold?
 */

class ofxGSTT : public ofBaseSoundInput{
public:
	ofxGSTT();

	void setup(int sampleRate, int nChannels, string language, string key, float _volumeThreshold = 0.05);
	void setListening(bool listen = true);

	bool isRecording(){
		return isRecording(OFXGSTT_DEFAULTDEVICE_ID);
	}

	virtual void audioIn( ofSoundBuffer& buffer );
	virtual void audioIn(float * buffer,int bufferSize, int nChannels, int deviceId);

	//GUI settings
	float volumeThreshold;

protected:
	void addDevice(int deviceId);
	bool isRecording(int deviceId);
	void prepareRecording(int deviceIdx);
	void finishRecording(int deviceIdx);

	vector<ofxGSTTTranscriptionThread*> transcriber;
	string language, key;

	/*** SOUND INPUT ***/
	int sampleRate;
	bool bListen;

	/*** SOUND RECORDING ***/
	int transcriberId;
	SF_INFO info;

	vector<int> deviceIds;
	vector<SNDFILE*> outfiles;
	vector<float> smoothedVolume;
	vector<ofxTimer*> timer;
	vector<bool> bRecording;
	vector<bool> bRecordingBlocked;
	vector<bool> bActiveVolume;
	vector<ofxGSTTTranscriptionThread*> deviceTanscriber;
};
