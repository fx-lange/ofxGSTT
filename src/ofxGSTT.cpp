#include "ofxGSTT.h"

ofxGSTT::ofxGSTT(ofBaseApp * baseApp){
	/*** SOUND INPUT ***/
	sampleRate = 16000;
	bufferSize = 256;
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	smoothedVol     = 0.0;
	scaledVol		= 0.0;
	soundStream.setup(baseApp, 0, 2, sampleRate, bufferSize, 4);

	/*** SOUND RECORDING ***/
	bActiveVolume = false;
	bRecording = false;
	bRecordingBlocked = true;
	timerRecording.setup(500,false);
	soundThreshold = 0.f;
	transcriptorId = 0;

	info.format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	info.frames = sampleRate*60;
	info.samplerate = sampleRate;
	info.channels = 2;

	prepareRecording();

	ofAddListener(ofEvents.audioReceived,this,&ofxGSTT::audioIn);
}//TODO extra setup function

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
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
	float scaleCurVolume = ofMap(curVol, 0.0, 0.17, 0.0, 1.0, true);
//	cout << "Volume: cur: " << curVol << "smooth: " << curVol << "scaled: " << curVol << endl;

	bool bActiveVolume = scaleCurVolume > soundThreshold;

	//TODO should be in an extra update function?!
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
	ofxGSTTTranscriptor * nextTranscriber = NULL;
	transcriptorId = 0;
	for(int i=0;i<transcriber.size();++i){
		ofxGSTTTranscriptor * tmpTranscriber = transcriber[i];
		if(tmpTranscriber->isFree()){
			ofLog(OF_LOG_VERBOSE,"free transcriber found");
			transcriptorId = i;
			nextTranscriber = tmpTranscriber;
			break;
		}
	}

	//new one if nothing is free
	if(nextTranscriber==NULL){
		ofLog(OF_LOG_VERBOSE,"all transcribers reserved -> create new one");
		transcriptorId = transcriber.size();
		nextTranscriber = new ofxGSTTTranscriptor(transcriptorId,&events);
		transcriber.push_back(nextTranscriber);
	}


	char filename[32];
	sprintf(filename,"data/tmpaudio%i",transcriptorId);
	nextTranscriber->setFilename(filename);
	nextTranscriber->reserve();
	sprintf(filename,"%s.wav",filename);
	outfile = sf_open(filename, SFM_WRITE, &info) ;

	if (!outfile){
		ofLog(OF_LOG_ERROR,"CANT OPEN FILE");//TODO msg
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
