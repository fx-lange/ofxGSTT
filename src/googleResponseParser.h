/***
 * Class to decode Googles response via jansson
 * REVISIT naming: parser is also parsed object
 */

#pragma once

#include "ofMain.h"
#include "ofxJSONElement.h"

class googleResponseParser{
public:

	string utterance;
	int status;
	float confidence;

	ofxJSONElement root;

	void parseJSON(char * jsonLine){

		bool parsingSuccessful = root.parse(jsonLine);
		if (parsingSuccessful) {
			cout << root.getRawString() << endl;
		} else {
			cout  << "Failed to parse JSON" << endl;
		}

		if(root.isMember("errors")) {
			cout << "error " + root.getRawString();
		} else if(root.isArray()) {
			status = root["status"].asInt();

			if(status == 0){
				ofxJSONElement hypotheses = root["hypotheses"];
				if(hypotheses.size() > 0){
					utterance = hypotheses[0]["utterance"];
					confidence = hypotheses[0]["confidence"];
				}else{
					confidence = 0;
					utterance = "no hypotheses";
				}
			}else{
				cout << "status: " + status << endl;
			}
		}
	}
};
