#pragma once

#include "ofMain.h"
#include "ofxSSL.h"

class ofAppSTT: public ofBaseApp{

public:
    void setup();
    void draw();

    void keyPressed  (int key);

    ofxSSL curl;
    string result;
};

