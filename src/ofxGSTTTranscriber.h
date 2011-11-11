#ifndef _STTGTRANSCRIBER_H_
#define _STTGTRANSCRIBER_H_

#include "ofxThread.h"

class ofxGSTTTranscriber : public ofThread{
public:
protected:

	virtual void threadedFunction(){
		if(verbose)printf("ofxGSTTTranscriber: overide threadedFunction with your own\n");
	}
};

#endif
