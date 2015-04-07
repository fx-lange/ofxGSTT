#pragma once

#include "ofMain.h"
#include "ofxGSTTTranscriptionThread.h"
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

	void setup(int sampleRate, int nChannels, string language, string key);

	void setAutoRecording(bool listen);
	bool isAutoRecording();

	void setVolumeThreshold(float volumeThreshold);
	float getVolumeThreshold();

	void startRecording();
	void stopRecording();

	bool isRecording(){
		return isRecording(OFXGSTT_DEFAULTDEVICE_ID);
	}

	float getSmoothedVolume(){
		return smoothedVolume[OFXGSTT_DEFAULTDEVICE_ID];
	}

	virtual void audioIn( ofSoundBuffer& buffer );
	virtual void audioIn(float * buffer,int bufferSize, int nChannels, int deviceId);

	//GUI settings

protected:
	void addDevice(int deviceId); //single device - multiple devices not thread safe at all
	bool isRecording(int deviceId);
	void prepareRecording(int deviceIdx);
	void finishRecording(int deviceIdx);

	vector<ofxGSTTTranscriptionThread*> transcriber;
	string language, key;

	/*** SOUND INPUT ***/
	int sampleRate;
	bool bAutoListen, bListen;
    float volumeThreshold;

	/*** SOUND RECORDING ***/
	int transcriberId;
	SF_INFO info;

	vector<int> deviceIds;
	vector<SNDFILE*> outfiles;
	vector<float> smoothedVolume;
	vector<unsigned long long> tLastUpdate;
	vector<bool> bRecording;
	vector<bool> bRecordingBlocked;
	vector<bool> bActiveVolume;
	vector<ofxGSTTTranscriptionThread*> deviceTanscriber;
};
