#include "ofxGSTT.h"

ofxGSTT::ofxGSTT(){
	bListen = false;
	volumeThreshold = 0.f;
	transcriptorId = 0;
}

void ofxGSTT::setup(int sampleRate,string language, float _volumeThreshold){
	/*** SOUND INPUT ***/
	bufferSize = 256;
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);

	/*** SOUND RECORDING ***/
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	info.frames = sampleRate * 60; //TODO revisit -> why *60? because of 60fps?
	info.samplerate = sampleRate;
	info.channels = 2;
	volumeThreshold = _volumeThreshold;

	this->language = language;

	ofDirectory dir;
	if(!dir.doesDirectoryExist("tmpAudio")){
		dir.createDirectory("tmpAudio");
	}

	//add and setup default device
	addDevice(OFXGSTT_DEFAULTDEVICE_ID);

	ofAddListener(ofEvents.audioReceived, this, &ofxGSTT::audioInWODevice);
	ofAddListener(audioDeviceEvent, this, &ofxGSTT::audioInByDevice);
	bListen = true;
}

void ofxGSTT::setListening(bool listen){
	if(listen == bListen){
		return; //nothing to change/do
	}else{
		if(listen){
			ofAddListener(ofEvents.audioReceived, this, &ofxGSTT::audioInWODevice);
			ofAddListener(audioDeviceEvent, this, &ofxGSTT::audioInByDevice);
		}else{
			ofRemoveListener(ofEvents.audioReceived, this, &ofxGSTT::audioInWODevice);
			ofRemoveListener(audioDeviceEvent, this, &ofxGSTT::audioInByDevice);
		}
		bListen = listen;
	}
}

void ofxGSTT::addDevice(int deviceId){
	deviceIds.push_back(deviceId);
	outfiles.push_back(NULL);
	smoothedVolume.push_back(0.f);
	ofxTimer * t=new ofxTimer();
	t->setup(1000, false);
	timer.push_back(t);
	bRecording.push_back(false);
	bRecordingBlocked.push_back(false);
	deviceTanscriber.push_back(NULL);

	ofDirectory dir;
	if(!dir.doesDirectoryExist("tmpAudio/device"+ofToString(deviceId)+"/")){
		dir.createDirectory("tmpAudio/device"+ofToString(deviceId));
	}

	prepareRecording(deviceIds.size()-1);
}

bool ofxGSTT::isRecording(int deviceId){
	for(int i=0;i<deviceIds.size();++i){
		if(deviceIds[i]==deviceId){
			return bRecording[i];
		}
	}
	ofLog(OF_LOG_WARNING,"no deviceId %d fount",deviceId);
	return false;
}

void ofxGSTT::audioInByDevice(ofxAudioDeviceArgs & event){
	audioIn(event.buffer,event.bufferSize,event.nChannels,event.deviceId);
}

void ofxGSTT::audioInWODevice(ofAudioEventArgs & event){
	audioIn(event.buffer,event.bufferSize,event.nChannels);
}

void ofxGSTT::audioIn(float * buffer,int bufferSize, int nChannels, int deviceId){
	int deviceIdx=0;
	for(int i=0;i<deviceIds.size();++i){
		if(deviceIds[i]==deviceId){
			deviceIdx=i;
			break;
		}
	}

	if(bRecordingBlocked[deviceIdx]){//TODO ARRAY
		return;
	}

	float curVol = 0.0;

	// samples are "interleaved"
	int numCounted = 0;

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for(int i = 0; i < bufferSize; i++){
		left[i] = buffer[i * 2];
		right[i] = buffer[i * 2 + 1];

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted += 2;
	}//TODO mehrfachverwendung von left & right - problem

	//this is how we get the mean of rms :)
	curVol /= (float) numCounted;

	// this is how we get the root of rms :)
	curVol = sqrt(curVol);

	smoothedVolume[deviceIdx] *= 0.9; //TODO settings for this ratio needed
	smoothedVolume[deviceIdx] += 0.1 * curVol;

	//lets scale the vol up to a 0-1 range
	//float scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);//TODO set by settings!
	float scaledCurVolume = ofMap(curVol, 0.0, 0.17, 0.0, 1.0, true);

	bool bActiveVolume = scaledCurVolume > volumeThreshold;

	//TODO revisit: should be in an extra update function?!
	if(bActiveVolume){
		timer[deviceIdx]->reset();
		timer[deviceIdx]->startTimer();
		bRecording[deviceIdx] = true;
	}
	if(bRecording[deviceIdx]){
		if(timer[deviceIdx]->isTimerFinished()){
			bRecording[deviceIdx] = false;
			finishRecording(deviceIdx);
			prepareRecording(deviceIdx);
		}else{
			sf_write_float(outfiles[deviceIdx], buffer, bufferSize * 2);
		}
	}
//
//	if(deviceId>10){
//		delete[] buffer;
//	}
}

void ofxGSTT::prepareRecording(int deviceIdx){
	ofLog(OF_LOG_VERBOSE, "prepare recording (device%d)",deviceIds[deviceIdx]);
	ofxGSTTTranscriptionThread * nextTranscriber = NULL;
	transcriptorId = 0;
	for(int i = 0; i < transcriber.size(); ++i){
		ofxGSTTTranscriptionThread * tmpTranscriber = transcriber[i];
		if(tmpTranscriber->isFree()){
			ofLog(OF_LOG_VERBOSE, "free transciptor found");
			transcriptorId = i;
			nextTranscriber = tmpTranscriber;
			break;
		}
	}

	//create new one if nothing is free
	if(nextTranscriber == NULL){
		ofLog(OF_LOG_VERBOSE, "no transciptor free -> create new one");
		transcriptorId = transcriber.size();
		nextTranscriber = new ofxGSTTTranscriptionThread(transcriptorId);
		transcriber.push_back(nextTranscriber);
	}

	char filename[32];
	sprintf(filename, "data/tmpAudio/device%d/%d ",deviceIds[deviceIdx],transcriptorId);
	nextTranscriber->setup(deviceIds[deviceIdx],language);
	nextTranscriber->setFilename(filename);
	nextTranscriber->reserve();
	deviceTanscriber[deviceIdx] = nextTranscriber;
	sprintf(filename, "%s.wav", filename);

	outfiles[deviceIdx] = sf_open(filename, SFM_WRITE, &info);
	if(!outfiles[deviceIdx]){
		ofLog(OF_LOG_ERROR, "CANT OPEN FILE");
	}else{
		bRecordingBlocked[deviceIdx] = false;
	}
}

void ofxGSTT::finishRecording(int deviceIdx){
	ofLog(OF_LOG_VERBOSE, "finish recording (device%d)",deviceIds[deviceIdx]);
	bRecordingBlocked[deviceIdx] = true;
	sf_close(outfiles[deviceIdx]);
	deviceTanscriber[deviceIdx]->startTranscription();
}


