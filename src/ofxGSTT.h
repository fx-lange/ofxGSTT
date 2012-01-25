#ifndef _STTG_H_
#define _STTG_H_

#include "ofMain.h"
#include "ofxGSTTTranscriptionThread.h"
#include "ofxTimer.h"
#include "sndfile.h"

#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

#define OFXGSTT_DEFAULTDEVICE_ID -1

using namespace std;

class ofxGSTT{
public:
	ofxGSTT();

	void setup(int sampleRate, string language, float _volumeThreshold = 0.05);
	void setListening(bool listen = true);
	void addDevice(int deviceId);

	bool isRecording(int deviceId = OFXGSTT_DEFAULTDEVICE_ID);

	void audioInByDevice(ofxAudioDeviceArgs & event);
	void audioInWODevice(ofAudioEventArgs & event);
	void audioIn(float * buffer,int bufferSize, int nChannels, int deviceId = OFXGSTT_DEFAULTDEVICE_ID);

	//GUI settings
	float volumeThreshold;

protected:
	void prepareRecording(int deviceIdx);
	void finishRecording(int deviceIdx);

	vector<ofxGSTTTranscriptionThread*> transcriber;
	string language;

	/*** SOUND INPUT ***/
	int bufferSize;
	int sampleRate;
	vector<float> left;
	vector<float> right;
	bool bListen;

	/*** SOUND RECORDING ***/
	int transcriptorId;
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

/*
 * TODO nice to have:
 * a SF_INFO per device - so each device can have its own settings!
 */

#endif
