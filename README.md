ofxGSTT is an openFrameworks addon / wrapper to the inofficial google speech to text api

The first prototype is finished. Have a look at the example and please let me know if you have any problems while using it.

feedback is welcome!

Credits:
----------------
It is ported from the processing library by [Florian Schulz](http://stt.getflourish.com/) which is based on Mike Pultz [article](http://mikepultz.com/2011/03/accessing-google-speech-api-chrome-11/) that shows how to use the technology offered by Google without a browser.

Dependencies
----------------
* [ofxTimer](https://github.com/fx-lange/ofxTimer)
* [ofxJSON](https://github.com/fx-lange/ofxJSON) 
* [ofxSSL](https://github.com/fx-lange/ofxSSL)
* sndfile - recording audio to wav
* libFlac - encoding wav to flac

sndfile and libFlac are included as static libs for unix(32 and 64bit).
