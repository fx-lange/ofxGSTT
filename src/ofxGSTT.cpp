#include "ofxGSTT.h"

ofxGSTT::ofxGSTT(){
	bListen = false;
	bActiveVolume = false;
	bRecording = false;
	bRecordingBlocked = true;
	timerRecording.setup(500,false);
	volumeThreshold = 0.f;
	transcriptorId = 0;
}

void ofxGSTT::setup(int sampleRate, float _volumeThreshold){
	/*** SOUND INPUT ***/
	sampleRate = 16000;
	bufferSize = 256;
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	smoothedVol     = 0.0;

	/*** SOUND RECORDING ***/
	info.format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	info.frames = sampleRate*60; //TODO revisit -> why *60? because of 60fps?
	info.samplerate = sampleRate;
	info.channels = 2;
	volumeThreshold = _volumeThreshold;

	ofDirectory dir;
	if(!dir.doesDirectoryExist("tmpAudio")){
		dir.createDirectory("tmpAudio");
	}

	prepareRecording();
	ofAddListener(ofEvents.audioReceived,this,&ofxGSTT::audioIn);
	bListen = true;
}

void ofxGSTT::setListening(bool listen){
	if(listen == bListen){
		return; //nothing to change/do
	}else{
		if(listen){
			ofAddListener(ofEvents.audioReceived,this,&ofxGSTT::audioIn);
		}else{
			ofRemoveListener(ofEvents.audioReceived,this,&ofxGSTT::audioIn);
		}
		bListen = listen;
	}
}

bool ofxGSTT::isRecording(){
	return bRecording;
}

void ofxGSTT::audioIn(ofAudioEventArgs& event){
	if(bRecordingBlocked){
		return;
	}

	float curVol = 0.0;

	// samples are "interleaved"
	int numCounted = 0;

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for (int i = 0; i < event.bufferSize; i++){
		left[i]		= event.buffer[i*2]*0.5;
		right[i]	= event.buffer[i*2+1]*0.5;

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}

	//this is how we get the mean of rms :)
	curVol /= (float)numCounted;

	// this is how we get the root of rms :)
	curVol = sqrt( curVol );

	smoothedVol *= 0.9; //TODO settings for this ratio needed
	smoothedVol += 0.1 * curVol;

	//lets scale the vol up to a 0-1 range
	float scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
	float scaledCurVolume = ofMap(curVol, 0.0, 0.17, 0.0, 1.0, true);

	bool bActiveVolume = scaledCurVolume > volumeThreshold;

	//TODO revisit: should be in an extra update function?!
	if(bActiveVolume){
		timerRecording.reset();
		timerRecording.startTimer();
		bRecording = true;
	}
	if(bRecording){
		if(timerRecording.isTimerFinished()){
			bRecording = false;
			finishRecording();
			prepareRecording();
		}else{
			sf_write_float(outfile, event.buffer, event.bufferSize*2);
		}
	}
}

void ofxGSTT::prepareRecording(){
	ofLog(OF_LOG_VERBOSE,"prepare recording");
	ofxGSTTTranscriptionThread * nextTranscriber = NULL;
	transcriptorId = 0;
	for(int i=0;i<transcriber.size();++i){
		ofxGSTTTranscriptionThread * tmpTranscriber = transcriber[i];
		if(tmpTranscriber->isFree()){
			ofLog(OF_LOG_VERBOSE,"free transciptor found");
			transcriptorId = i;
			nextTranscriber = tmpTranscriber;
			break;
		}
	}

	//create new one if nothing is free
	if(nextTranscriber==NULL){
		ofLog(OF_LOG_VERBOSE,"no transciptor free -> create new one");
		transcriptorId = transcriber.size();
		nextTranscriber = new ofxGSTTTranscriptionThread(transcriptorId);
		transcriber.push_back(nextTranscriber);
	}

	char filename[32];
	sprintf(filename,"data/tmpAudio/%i",transcriptorId);
	nextTranscriber->setFilename(filename);
	nextTranscriber->reserve();
	sprintf(filename,"%s.wav",filename);

	outfile = sf_open(filename, SFM_WRITE, &info) ;
	if (!outfile){
		ofLog(OF_LOG_ERROR,"CANT OPEN FILE");
	}else{
		bRecordingBlocked = false;
	}
}

void ofxGSTT::finishRecording(){
	ofLog(OF_LOG_VERBOSE,"finish recording");
	bRecordingBlocked = true;
	transcriber[transcriptorId]->startTranscription();
	sf_close(outfile);
}
