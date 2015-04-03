#include "ofAppSTT.h"

//--------------------------------------------------------------
void ofAppSTT::setup(){
	ofBackground(0);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofxSSL::appendData = true; //only needed to show http errors
}

//--------------------------------------------------------------
void ofAppSTT::draw(){
    ofDrawBitmapString(result, 10, 10);
}

//--------------------------------------------------------------
void ofAppSTT::keyPressed (int key){
    if(key == ' '){

    	//TODO check the readme for generating your own key
    	curl.setURL("https://www.google.com/speech-api/v2/recognize?output=json&lang=en-us&key=yourkey");

    	// set form
		std::string uploadfile = ofToDataPath("speechToText.flac",true);
		curl.addFormFile("file",uploadfile,"audio/x-flac");

		// set header
		curl.addHeader("Expect:");
		curl.addHeader("Content-Type: audio/x-flac; rate=16000");

		curl.perform();
		result = curl.getResponseBody();

		curl.clear();
    }
}
