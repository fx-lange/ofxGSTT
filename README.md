ofxGSTT
================
openFrameworks addon/wrapper to the **Google Speech To Text API v2**. 

Unfortunately the first API version doesn't work anymore and API v2 is officially limited to 50 requests per day and it is still not possible to apply for a higher quota (but people reported that they were able to get up to 500 responses per day). 

Anyway I can't recommend using this addon in a production environment - only for prototyping purposes.

**For unlimited requests check out the workaround example via Chrome.**

Feedback is really welcome!

Credits:
----------------
Originally this addon was ported from the processing [library](http://stt.getflourish.com/) by Florian Schulz, which was based on Mike Pultz [article](http://mikepultz.com/2011/03/accessing-google-speech-api-chrome-11/) that showed how to use the technology offered by Google without a browser.

Informations to switch from v1 to v2 were found [here](https://github.com/gillesdemey/google-speech-v2).

Examples:
----------------

Before you start you need to generate your own [Speech API Key](http://www.chromium.org/developers/how-tos/api-keys).

### example-gstt_api_only: 
	
not using the addon at all, only showcasing the api request with a prerecorded audio file

### example-gstt_basics:

basic addon example for recording and transcripting

### example-gstt_chrome_workaround

Instead of using ofxGSTT, this workaround uses the Speech to Text API within Google Chrome and sends the results to openFrameworks via WebSockets. Idea (again) ported from Florian Schulz [example](http://mikepultz.com/2011/03/accessing-google-speech-api-chrome-11/). 

Dependencies: Google Chrome (or Chromium) and [ofxLibwebsockets](http://github.com:labatrockwell/ofxLibwebsockets)

Dependencies
----------------
* [ofxJSON](https://github.com/jefftimesten/ofxJSON) 
* [ofxSSL](https://github.com/fx-lange/ofxSSL)
* sndfile - recording audio to wav

libsndfile is included as static libs for unix(32 and 64bit). for mac you might need to `brew install libsndfile` and for win you can try to install the binaries provided [here](http://www.mega-nerd.com/libsndfile/#Download).
