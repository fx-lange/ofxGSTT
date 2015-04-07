#include "ofGsttApp.h"

bool bAutoRecording = true;
float volumeThreshold = 0.05f;

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
 	gstt.setup(sampleRate,nChannels,"en-us",YOUR_KEY_AS_A_STRING_HERE);//check the README for generating an API key
 	gstt.setAutoRecording(bAutoRecording);
 	gstt.setVolumeThreshold(volumeThreshold);
 	//make gstt owner of the sound input stream
	soundStream.setInput(gstt);

 	//register listener function to transcript response events
	ofAddListener(ofxGSTTTranscriptionThread::gsttApiResponseEvent,this,&ofGsttApp::gsttResponse);


	responseStr = "responses:\n(your might want to order them via tSend)\n";
}

//--------------------------------------------------------------
void ofGsttApp::update(){
}

void ofGsttApp::gsttResponse(ofxGSTTResponseArgs & response){
	cout << "Response: " << response.msg << endl;
	cout << "with confidence: " << ofToString(response.confidence) << endl;
	float tProcessingTime = (response.tReceived - response.tSend)/1000.f;
	cout << "processing time(seconds): " << ofToString(tProcessingTime) << endl;
	if(response.msg != ""){
		responseStr += response.msg + " (after "+ofToString(tProcessingTime)+ "s )\n";
	}else{
		responseStr += "no results\n";
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
	std::string status = "(s/S) start/stop recording\n";
	status += "( a ) toggle auto recording: " + ofToString(gstt.isAutoRecording());
	if(gstt.isAutoRecording())
		status += " (+/-) volume threshold: " + ofToString(gstt.getVolumeThreshold());
	status += "\nsmoothed volume: " + ofToString(gstt.getSmoothedVolume(),3);
	ofDrawBitmapString(status,50,20);
	ofDrawBitmapString(responseStr,50,100);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofGsttApp::keyPressed (int key){
    switch(key){
        case 'a':
            bAutoRecording = !bAutoRecording;
            gstt.setAutoRecording(bAutoRecording);
            break;
        case 's':
        	gstt.startRecording();
        	break;
        case 'S':
        	gstt.stopRecording();
        	break;
        case '+':
        	volumeThreshold += 0.01f;
        	gstt.setVolumeThreshold(volumeThreshold);
        	break;
        case '-':
        	volumeThreshold -= 0.01f;
        	gstt.setVolumeThreshold(volumeThreshold);
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

