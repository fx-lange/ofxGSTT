#include "ofxGSTTTranscriptionThread.h"

ofEvent<ofxGSTTResponseArgs> ofxGSTTTranscriptionThread::gsttApiResponseEvent = ofEvent<ofxGSTTResponseArgs>();

ofxGSTTTranscriptionThread::ofxGSTTTranscriptionThread(int id) :
		ofThread(){
	this->id = id;

	this->bFinished = false;
	this->bFree = true;
	this->deviceId = -1;
}

void ofxGSTTTranscriptionThread::setup(int deviceId, string language, string key){
	string url = "https://www.google.com/speech-api/v2/recognize?output=json&lang="+language+"&key="+key;
	this->deviceId = deviceId;

	curl.setup();
	curl.setURL(url);
}

void ofxGSTTTranscriptionThread::setFilename(char * _filename){
	ofLog(OF_LOG_VERBOSE, "set filename: %s", _filename);
	sprintf(wavFile, "%s.wav", _filename);
}

bool ofxGSTTTranscriptionThread::isFree(){
	return bFree;
}

void ofxGSTTTranscriptionThread::reserve(){
	bFree = false;
}

bool ofxGSTTTranscriptionThread::isFinished(){
	return bFinished;
}

void ofxGSTTTranscriptionThread::startTranscription(){
	ofLog(OF_LOG_VERBOSE, "start transcription (device%d)",deviceId);
	startThread(true,false);
}

void ofxGSTTTranscriptionThread::stopTranscription(){
	ofLog(OF_LOG_VERBOSE, "stop transcription (device%d)",deviceId);
	bFinished = true;
	stopThread();
}

void ofxGSTTTranscriptionThread::threadedFunction(){
	//transcribe via google
	sendGoogleRequest();
	bFinished = true;
	bFree = true;
}

void ofxGSTTTranscriptionThread::sendGoogleRequest(){
	ofLog(OF_LOG_VERBOSE, "send request to google (device%d)",deviceId);


	curl.addFormFile("file",ofToString(wavFile),"audio/l16"); //TODO hardcoded

    // set header
    curl.addHeader("Expect:");
    curl.addHeader("Content-Type: audio/l16; rate=44100"); //TODO hardcoded

	ofxGSTTResponseArgs response;
    response.deviceId = deviceId;
    response.tSend = ofGetSystemTime();
    curl.perform();


    std::string responseJson = curl.getResponseBody();
    ofLogNotice("sendGoogleRequest") << responseJson;

    //decode via json
    googleResponseParser parser;
    bool validResponse = parser.parseJSON(responseJson);

    if(validResponse){
        response.tReceived = ofGetSystemTime();
        response.msg = parser.utterance;
        response.confidence = parser.confidence;

        ofNotifyEvent(gsttApiResponseEvent, response);
    }

    curl.clear();
}
