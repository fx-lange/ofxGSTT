#include "gsttApp.h"

//--------------------------------------------------------------
void gsttApp::setup(){
	ofSetFrameRate(60);
	ofBackground(0, 0, 0);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofxSSL::appendData = true; //only needed to get show http errors

	//init oF soundstream
	int sampleRate = 16000;
	int bufferSize = 256;
	soundStream.setup(this, 0, 2, sampleRate, bufferSize, 4);

    //init ofxGSTT which starts listening here
 	gstt.setup(sampleRate,"de",0.5f);

 	//register listener function to googles response events
	ofAddListener(ofxGSTTTranscriptionThread::gsttApiResponseEvent,this,&gsttApp::gsttResponse);
	bListening = true;

	responseStr = "";
}

//--------------------------------------------------------------
void gsttApp::update(){
}

//--------------------------------------------------------------
void gsttApp::audioIn(float * input, int bufferSize, int nChannels){
    //redirect audioIn as ofEvents.audioReceived
    //don't know why ofSoundStream doesn't send audioReceived events

	ofAudioEventArgs event;
	event.buffer = input;
	event.bufferSize = bufferSize;
	event.nChannels = nChannels;
	ofNotifyEvent(ofEvents().audioReceived,event);
}

void gsttApp::gsttResponse(ofxGSTTResponseArgs & response){
	cout << "Response: " << response.msg << endl << "with confidence: " << ofToString(response.confidence) << endl;
	cout << "processing time(ms): " << ofToString(response.tReceived - response.tSend) << endl;
	if(response.msg != ""){
		responseStr += response.msg + "\n";
	}
}


//--------------------------------------------------------------
void gsttApp::draw(){
	ofBackground(0);
	ofPushStyle();
	ofSetColor(255,255,255);

	if(gstt.isRecording()){
		ofSetColor(255,0,0);
	}else{
		ofSetColor(20,20,20);
	}
	ofEllipse(ofGetWidth()/2,ofGetHeight()/2,200,200);

	ofSetColor(255,255,255);
	ofDrawBitmapString(responseStr,50,50);

	ofPopStyle();
}

//--------------------------------------------------------------
void gsttApp::keyPressed (int key){
    switch(key){
        case 'l':
            bListening = !bListening;
            gstt.setListening(bListening);
            break;
    }
}

//--------------------------------------------------------------
void gsttApp::keyReleased  (int key){
}

//--------------------------------------------------------------
void gsttApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void gsttApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void gsttApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void gsttApp::mouseReleased(){

}

