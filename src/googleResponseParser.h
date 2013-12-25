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

	bool parseJSON(const string & jsonLine){

		bool parsingSuccessful = root.parse(jsonLine);
		if (!parsingSuccessful) {
			ofLogError( "Failed to parse JSON:" + jsonLine);
			return false;
		}

		if(root.isMember("errors")) {
			cout << "error " + root.getRawString();
		} else if(root.isObject()) {
			status = root["status"].asInt();

			if(status == 0){
				ofxJSONElement hypotheses = root["hypotheses"];
				if(hypotheses.size() > 0){
					utterance = hypotheses[0]["utterance"].asString();
					confidence = hypotheses[0]["confidence"].asFloat();
				}else{
					confidence = 0;
					utterance = "no hypotheses";
				}
				return true;
			}
		}

        ofLogVerbose("no valid response: ") << root.getRawString() << endl;
        return false;
	}
};
