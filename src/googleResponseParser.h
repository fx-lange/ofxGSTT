/***
 * Class to decode Googles response via jansson
 */

#ifndef _RESPONSEPARSER_H_
#define _RESPONSEPARSER_H_

#include "ofMain.h"
#include "jansson.h"

class googleResponseParser{
public:

	string utterance;
	int status;
	float confidence;

	void parseJSON(char * jsonLine){
		json_t* root;
		json_t* result;
		json_t* hypotheses;
		json_error_t error;

		// load json.
		root = json_loads(jsonLine, 0, &error);
		if(!root){
			ofLog(OF_LOG_ERROR, "error: on jsonLine:%d %s", error.line, error.text);
			return;
		}

		json_unpack(root, "{s:o}", "hypotheses", &hypotheses);

		result = json_object_get(root, "status");
		if(!json_is_integer(result)){
			ofLog(OF_LOG_ERROR, "error: cannot get status from json");
			return;
		}
		status = json_integer_value(result);
		if(status == 0){
			if(json_array_size(hypotheses) > 0){
				result = json_array_get(hypotheses, 0);
				parseGoogleJSON(result);
			}else{
				confidence = 0;
				utterance = "";
			}
		}

		delete root;
	}
private:
	void parseGoogleJSON(json_t*& root){

		// utterance
		json_t* node = json_object_get(root, "utterance");
		if(!json_is_string(node)){
			ofLog(OF_LOG_ERROR, "error: cannot get utterance from response");
			return;
		}
		utterance = json_string_value(node);

		// confidence
		node = json_object_get(root, "confidence");
		if(!json_is_real(node)){
			ofLog(OF_LOG_ERROR, "error: cannot get confidence from response");
			return;
		}
		confidence = json_real_value(node);
	}
};

#endif
