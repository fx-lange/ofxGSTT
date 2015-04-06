#include "ofGsttApp.h"

//--------------------------------------------------------------
void ofGsttApp::setup(){
	ofSetFrameRate(60);
	ofBackground(0, 0, 0);
	ofSetLogLevel(OF_LOG_VERBOSE);

	//init oF soundstream
	int sampleRate = 44100;
	int bufferSize = 256;
	int nChannels = 1;
	soundStream.printDeviceList();
//	soundStream.setDeviceID(3);
	soundStream.setup(0, nChannels, sampleRate, bufferSize, 4);

    //init ofxGSTT
 	gstt.setup(sampleRate,nChannels,"en-us",YOUR_KEY_AS_A_STRING_HERE,0.8f);//have a look at the README
 	//make gstt owner of the sound input stream
	soundStream.setInput(gstt);

 	//register listener function to transcript response events
	ofAddListener(ofxGSTTTranscriptionThread::gsttApiResponseEvent,this,&ofGsttApp::gsttResponse);
	bListening = true;

	responseStr = "";
}

//--------------------------------------------------------------
void ofGsttApp::update(){
}

void ofGsttApp::gsttResponse(ofxGSTTResponseArgs & response){
	cout << "Response: " << response.msg << endl << "with confidence: " << ofToString(response.confidence) << endl;
	cout << "processing time(ms): " << ofToString(response.tReceived - response.tSend) << endl;
	if(response.msg != ""){
		responseStr += response.msg + "\n";
	}
}

//--------------------------------------------------------------
void ofGsttApp::draw(){
	ofBackground(0);
	ofPushStyle();
	ofSetColor(255,255,255);

	if(gstt.isRecording()){
		ofSetColor(255,0,0);
	}else{
		ofSetColor(20,20,20);
	}
	ofDrawEllipse(ofGetWidth()/2,ofGetHeight()/2,200,200);

	ofSetColor(255,255,255);
	ofDrawBitmapString(responseStr,50,50);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofGsttApp::keyPressed (int key){
    switch(key){
        case ' ':
            bListening = !bListening;
            gstt.setListening(bListening);
            break;
    }
}

//--------------------------------------------------------------
void ofGsttApp::keyReleased  (int key){
}

//--------------------------------------------------------------
void ofGsttApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofGsttApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofGsttApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofGsttApp::mouseReleased(){

}

