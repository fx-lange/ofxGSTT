#include "ofxGSTT.h"

ofxGSTT::ofxGSTT(){
	bListen = false;
	transcriberId = 0;
	sampleRate = volumeThreshold = -1;
}

void ofxGSTT::setup(int sampleRate, int nChannels, string language, string key, float _volumeThreshold){
	/*** SOUND RECORDING ***/
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	info.frames = sampleRate * 60; //TODO revisit -> why *60? because of 60fps?
	info.samplerate = sampleRate;
	info.channels = nChannels;
	volumeThreshold = _volumeThreshold;

	this->language = language;
	this->key = key;

	ofDirectory dir;
	if(!dir.doesDirectoryExist("tmpAudio")){
		dir.createDirectory("tmpAudio");
	}

	//add and setup default device
	addDevice(OFXGSTT_DEFAULTDEVICE_ID); //TODO multi device business

	bListen = true;
}

void ofxGSTT::setListening(bool listen){
	ofLogVerbose("set listening") << listen;
	bListen = listen;
}

void ofxGSTT::addDevice(int deviceId){
	deviceIds.push_back(deviceId);
	outfiles.push_back(NULL);
	smoothedVolume.push_back(0.f);
	tLastUpdate.push_back(0);
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
	for(int i=0;i<(int)deviceIds.size();++i){
		if(deviceIds[i]==deviceId){
			return bRecording[i];
		}
	}
	ofLog(OF_LOG_WARNING,"no deviceId %d fount",deviceId);
	return false;
}

void ofxGSTT::audioIn(ofSoundBuffer & buffer){
	//TODO make better use of soundbuffer obj
	audioIn(&buffer[0], buffer.getNumFrames(), buffer.getNumChannels(), OFXGSTT_DEFAULTDEVICE_ID); //TODO multidevice business
}

void ofxGSTT::audioIn(float * buffer,int bufferSize, int nChannels, int deviceId){
	int deviceIdx=0;
	for(int i=0;i<(int)deviceIds.size();++i){
		if(deviceIds[i]==deviceId){
			deviceIdx=i;
			break;
		}
	}

	if(bRecordingBlocked[deviceIdx]){
		return;
	}

	float curVol = 0.0;

	// samples are "interleaved"
	int numCounted = 0;

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	vector<float> left;
	vector<float> right;
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	for(int i = 0; i < bufferSize; i++){
		if(nChannels == 1){
			curVol += buffer[i] * buffer[i];
			numCounted += 1;
		}else if(nChannels == 2){
			left[i] = buffer[i * 2];
			right[i] = buffer[i * 2 + 1];

			curVol += left[i] * left[i];
			curVol += right[i] * right[i];
			numCounted += 2;
		}
	}

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
	unsigned long long tNow = ofGetElapsedTimeMillis();
	if(bActiveVolume && bListen){
		tLastUpdate[deviceIdx] = tNow;
		bRecording[deviceIdx] = true;
	}
	if(bRecording[deviceIdx]){
		if(tNow - tLastUpdate[deviceIdx] > 1000){ //TODO magic number, hard coded
			bRecording[deviceIdx] = false;
			finishRecording(deviceIdx);
			prepareRecording(deviceIdx);
		}else{
			sf_write_float(outfiles[deviceIdx], buffer, bufferSize * nChannels);
		}
	}
}

void ofxGSTT::prepareRecording(int deviceIdx){
	ofLog(OF_LOG_VERBOSE, "prepare recording (device%d)",deviceIds[deviceIdx]);
	ofxGSTTTranscriptionThread * nextTranscriber = NULL;
	transcriberId = 0;
	for(int i = 0; i < (int)transcriber.size(); ++i){
		ofxGSTTTranscriptionThread * tmpTranscriber = transcriber[i];
		if(tmpTranscriber->isFree()){
			ofLog(OF_LOG_VERBOSE, "free transcriber found");
			transcriberId = i;
			nextTranscriber = tmpTranscriber;
			break;
		}
	}

	//create new one if nothing is free
	if(nextTranscriber == NULL){
		ofLog(OF_LOG_VERBOSE, "no transcriber free -> create new one");
		transcriberId = transcriber.size();
		nextTranscriber = new ofxGSTTTranscriptionThread(transcriberId);
		transcriber.push_back(nextTranscriber);
	}

	string dataPath = ofToDataPath("tmpAudio");
	char filename[128];
	sprintf(filename, "%s/device%d/%d",dataPath.c_str(),deviceIds[deviceIdx],transcriberId);
	nextTranscriber->setup(deviceIds[deviceIdx],language,key);
	nextTranscriber->setFilename(filename);
	nextTranscriber->reserve();
	deviceTanscriber[deviceIdx] = nextTranscriber;
	sprintf(filename, "%s.wav", filename);
	outfiles[deviceIdx] = sf_open(filename, SFM_WRITE, &info);
	if(!outfiles[deviceIdx]){
		ofLog(OF_LOG_ERROR, "CAN NOT OPEN FILE "+ofToString(filename));
		ofLogError(ofToString(sf_strerror(outfiles[deviceIdx])));
		sf_close(outfiles[deviceIdx]);
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


