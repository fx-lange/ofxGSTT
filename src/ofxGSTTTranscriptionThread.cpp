#include "ofxGSTTTranscriptionThread.h"

ofxGSTTTranscriptionThread::ofxGSTTTranscriptionThread(int id) :
		ofThread(){
	this->id = id;

	this->bFinished = false;
	this->isEncoded = false;
	this->bFree = true;
	total_samples = 0;
}

void ofxGSTTTranscriptionThread::setup(int deviceId, string language){
	url = "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&lang="+language;
	this->deviceId = deviceId;
}

void ofxGSTTTranscriptionThread::setFilename(char * _filename){
	ofLog(OF_LOG_VERBOSE, "set filename: %s", _filename);
	sprintf(flacFile, "%s.flac", _filename);
	sprintf(wavFile, "%s.wav", _filename);
}

bool ofxGSTTTranscriptionThread::isFree(){
	return bFree;
}

void ofxGSTTTranscriptionThread::reserve(){
	bFree = false;
}

bool ofxGSTTTranscriptionThread::isFinished(){
	return bFinished;
}

void ofxGSTTTranscriptionThread::startTranscription(){
	ofLog(OF_LOG_VERBOSE, "start transcription (device%d)",deviceId);
	isEncoded = false;
	startThread(true,false);
}

void ofxGSTTTranscriptionThread::stopTranscription(){
	ofLog(OF_LOG_VERBOSE, "stop transcription (device%d)",deviceId);
	bFinished = true;
	stopThread();
}

void ofxGSTTTranscriptionThread::threadedFunction(){
	if(!isEncoded){
		//encode with libFlac
		isEncoded = encodeToFlac();
		if(!isEncoded){
			ofLog(OF_LOG_ERROR, "ENCODING FAILD (device%d)",deviceId);
			threadRunning = false;
			return;
		}
	}
	if(isEncoded){
		//transcribe via google
		flacToGoogle();
		bFinished = true;
		bFree = true;
	}
}

bool ofxGSTTTranscriptionThread::flacToGoogle(){
	ofLog(OF_LOG_VERBOSE, "send flac to google (device%d)",deviceId);
	CURL *curl;
	CURLcode res;

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;

	curl_global_init(CURL_GLOBAL_ALL);

	// set form
	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME,
			"file",
			CURLFORM_FILE,
			flacFile,
			CURLFORM_CONTENTTYPE,
			"audio/x-flac",// rate=16000",
			CURLFORM_END);

	curl = curl_easy_init();

	// set header
	headerlist = curl_slist_append(headerlist, "Expect:");
	headerlist = curl_slist_append(headerlist, "Content-Type: audio/x-flac; rate=44000");
	if(curl){
		//set options
		curl_easy_setopt(curl,
				CURLOPT_URL,
				url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		//set callback function and callback data for the https response
		callBackData data;
		data.id = id;
		data.timestamp = ofGetSystemTime();
		data.deviceId = deviceId;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponseFunc);

		//submit form
		res = curl_easy_perform(curl);

		//cleanup
		curl_easy_cleanup(curl);
		curl_formfree(formpost);
		curl_slist_free_all(headerlist);
		return true;
	}else{
		ofLog(OF_LOG_ERROR, "google request failed");
		return false;
	}
}

bool ofxGSTTTranscriptionThread::encodeToFlac(){
	ofLog(OF_LOG_VERBOSE, "init encoding (device%d)",deviceId);
	FLAC__bool ok = true;
	FLAC__StreamEncoder *encoder = 0;
	FLAC__StreamEncoderInitStatus init_status;
	FILE *fin;
	unsigned sample_rate = 0;
	unsigned channels = 0;
	unsigned bps = 0;

	if((fin = fopen(wavFile, "rb")) == NULL){
		ofLog(OF_LOG_ERROR, "ERROR: opening %s for output\n", wavFile);
		return false;
	}

	// read and validate wav header
	if(fread(buffer, 1, 44, fin) != 44 || memcmp(buffer, "RIFF", 4)
			|| memcmp(buffer + 8, "WAVEfmt \020\000\000\000\001\000\002\000", 16)
			|| memcmp(buffer + 32, "\004\000\020\000data", 8)){
		ofLog(OF_LOG_ERROR,
				"invalid/unsupported WAVE file, only 16bps stereo WAVE in canonical form allowed");
		fclose(fin);
		return false;
	}
	sample_rate = ((((((unsigned) buffer[27] << 8) | buffer[26]) << 8) | buffer[25]) << 8)
			| buffer[24];
	channels = 2;
	bps = 16;
	total_samples = (((((((unsigned) buffer[43] << 8) | buffer[42]) << 8) | buffer[41]) << 8)
			| buffer[40]) / 4;

	// allocate the encoder
	if((encoder = FLAC__stream_encoder_new()) == NULL){
		ofLog(OF_LOG_ERROR, " allocating encoder\n");
		fclose(fin);
		return false;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

	// initialize encoder
	if(ok){
		init_status = FLAC__stream_encoder_init_file(encoder, flacFile, NULL, NULL);
		if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			ofLog(OF_LOG_ERROR, "initializing encoder: ");
			ofLog(OF_LOG_ERROR, FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	ofLog(OF_LOG_VERBOSE, "start encoding (device%d)",deviceId);
	/* read blocks of samples from WAVE file and feed to encoder */
	if(ok){
		size_t left = (size_t) total_samples;
		while(ok && left){
			size_t need = (left > READSIZE ? (size_t) READSIZE : (size_t) left);
			if(fread(buffer, channels * (bps / 8), need, fin) != need){
				ofLog(OF_LOG_ERROR, "reading from WAVE file");
				ok = false;
			}else{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				size_t i;
				for(i = 0; i < need * channels; i++){
					/* inefficient but simple and works on big- or little-endian machines */
					pcm[i] = (FLAC__int32) (((FLAC__int16) (FLAC__int8) buffer[2 * i + 1] << 8)
							| (FLAC__int16) buffer[2 * i]);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
			}
			left -= need;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

//	fprintf(stderr, "encoding: %s\n", ok ? "succeeded" : "FAILED");
//	fprintf(stderr,
//			"   state: %s\n",
//			FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

	FLAC__stream_encoder_delete(encoder);
	fclose(fin);

	return ok;
}
