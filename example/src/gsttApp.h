#pragma once

#include "ofMain.h"
#include "ofxTimer.h"
#include "ofxGSTT.h"



class gsttApp: public ofSimpleApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased();

	void audioIn(float * input, int bufferSize, int nChannels);

	void gsttResponse(ofxGSTTResponseArgs & response);

	string responseStr;

	ofxGSTT gstt;
	ofSoundStream soundStream;
	bool bListening;
};

