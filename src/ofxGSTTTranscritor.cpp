#include "ofxGSTTTranscriptor.h"

ofxGSTTTranscriptor::ofxGSTTTranscriptor() :
		ofThread() {
	this->id = id;
	this->bFinished = false;
	this->isEncoded = false;
	this->bFree = true;
	total_samples = 0;//TODO sinn hier
}

void ofxGSTTTranscriptor::setFilename(char * _filename){
	ofLog(OF_LOG_VERBOSE,"set filename: %s",_filename);
	memcpy(filename,_filename,strlen(_filename));
}

bool ofxGSTTTranscriptor::isFree(){
	return bFree;
}

void ofxGSTTTranscriptor::reserve(){
	bFree = false;
}

bool ofxGSTTTranscriptor::isFinished(){
	return bFinished;
}

void ofxGSTTTranscriptor::startTranscribing() {
	ofLog(OF_LOG_VERBOSE,"start transcribing");
	startThread();
}

void ofxGSTTTranscriptor::stopTranscribing(){
	//TODO
	bFinished = true;
}

void ofxGSTTTranscriptor::threadedFunction() {
	//loop while thread is running
	while (isThreadRunning() == true) {
		if(!isEncoded){

			isEncoded = encodeToFlac();
			if(!isEncoded){
				threadRunning = false;
			}
		}
		if(isEncoded){
			//transcribe via google
			bFinished = true;
		}
	}
}

bool ofxGSTTTranscriptor::encodeToFlac() {
	ofLog(OF_LOG_VERBOSE,"init encoding");
	FLAC__bool ok = true;
	FLAC__StreamEncoder *encoder = 0;
	FLAC__StreamEncoderInitStatus init_status;
//	FLAC__StreamMetadata *metadata[2];
	FLAC__StreamMetadata_VorbisComment_Entry entry;
	FILE *fin;
	unsigned sample_rate = 0;
	unsigned channels = 0;
	unsigned bps = 0;

	char wavFile[32];
	sprintf(wavFile,"%s.wav",filename);
	if ((fin = fopen(wavFile, "rb")) == NULL) {
		ofLog(OF_LOG_ERROR, "ERROR: opening %s for output\n", wavFile);
		return false;
	}

	/* read wav header and validate it */
	if (fread(buffer, 1, 44, fin) != 44 || memcmp(buffer, "RIFF", 4) || memcmp(buffer + 8, "WAVEfmt \020\000\000\000\001\000\002\000", 16)
			|| memcmp(buffer + 32, "\004\000\020\000data", 8)) {
		fprintf(stderr, "ERROR: invalid/unsupported WAVE file, only 16bps stereo WAVE in canonical form allowed\n"); //TODO use ofLog
		fclose(fin);
		return false;
	}
	sample_rate = ((((((unsigned) buffer[27] << 8) | buffer[26]) << 8) | buffer[25]) << 8) | buffer[24];
	cout << "SAMPLE RATE: " << sample_rate;
	channels = 2;
	bps = 16;
	total_samples = (((((((unsigned) buffer[43] << 8) | buffer[42]) << 8) | buffer[41]) << 8) | buffer[40]) / 4;

	/* allocate the encoder */
	if ((encoder = FLAC__stream_encoder_new()) == NULL) {
		fprintf(stderr, "ERROR: allocating encoder\n"); //TODO use ofLog
		fclose(fin);
		return false;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

//	/* now add some metadata; we'll add some tags and a padding block */
//	if(ok) {
//		if(
//			(metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
//			(metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
//			/* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
//			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
//			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
//			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
//			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
//		) {
//			fprintf(stderr, "ERROR: out of memory or tag error\n");
//			ok = false;
//		}
//
//		metadata[1]->length = 1234; /* set the padding length */
//
//		ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
//	}

	/* initialize encoder */
	if (ok) {
		char flacFile[32];
		sprintf(flacFile,"%s.flac",filename);
		init_status = FLAC__stream_encoder_init_file(encoder, flacFile, /*progress_callback*/NULL, /*client_data=*/NULL);//TODO progress callback
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);//TODO use ofLog
			ok = false;
		}
	}

	ofLog(OF_LOG_VERBOSE,"start encoding");
	/* read blocks of samples from WAVE file and feed to encoder */
	if (ok) {
		size_t left = (size_t) total_samples;
		while (ok && left) {
			size_t need = (left > READSIZE ? (size_t) READSIZE : (size_t) left);
			if (fread(buffer, channels * (bps / 8), need, fin) != need) {
				fprintf(stderr, "ERROR: reading from WAVE file\n");//TODO use ofLog
				ok = false;
			} else {
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				size_t i;
				for (i = 0; i < need * channels; i++) {
					/* inefficient but simple and works on big- or little-endian machines */
					pcm[i] = (FLAC__int32) (((FLAC__int16) (FLAC__int8) buffer[2 * i + 1] << 8) | (FLAC__int16) buffer[2 * i]);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
			}
			left -= need;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	fprintf(stderr, "encoding: %s\n", ok ? "succeeded" : "FAILED");//TODO use ofLog
	fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);//TODO use ofLog

	/* now that encoding is finished, the metadata can be freed */
//	FLAC__metadata_object_delete(metadata[0]);
//	FLAC__metadata_object_delete(metadata[1]);

	FLAC__stream_encoder_delete(encoder);
	fclose(fin);

	return ok;
//
//	 CURL *curl;
//	  CURLcode res;
//
//	  struct curl_httppost *formpost=NULL;
//	  struct curl_httppost *lastptr=NULL;
//	  struct curl_slist *headerlist=NULL;
//	  static const char buf[] = "Expect:";
//
//	  curl_global_init(CURL_GLOBAL_ALL);
//
//	  /* Fill in the file upload field */
//	  curl_formadd(&formpost,
//	               &lastptr,
//	               CURLFORM_COPYNAME, "file",
//	               CURLFORM_FILE, "data/test.flac",
//	               CURLFORM_CONTENTTYPE, "audio/x-flac; rate=16000",
//	               CURLFORM_END);
//
//	//  /* Fill in the filename field */
//	//  curl_formadd(&formpost,
//	//               &lastptr,
//	//               CURLFORM_COPYNAME, "filename",
//	//               CURLFORM_COPYCONTENTS, "test.flac",
//	//               CURLFORM_END);
//	//
//	//
//	//  /* Fill in the submit field too, even if this is rarely needed */
//	//  curl_formadd(&formpost,
//	//               &lastptr,
//	//               CURLFORM_COPYNAME, "submit",
//	//               CURLFORM_COPYCONTENTS, "send",
//	//               CURLFORM_END);
//
//	  curl = curl_easy_init();
//	  /* initalize custom header list (stating that Expect: 100-continue is not
//	     wanted */
//	  headerlist = curl_slist_append(headerlist, buf);
//	  headerlist = curl_slist_append(headerlist, "Content-Type: audio/x-flac; rate=16000");
//	  if(curl) {
//	    /* what URL that receives this POST */
//	//    curl_easy_setopt(curl, CURLOPT_URL, "http://spta.yourweb.de/fxlange/upload.php");
//	    curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&lang=de-de");
//	      /* only disable 100-continue header if explicitly requested */
//	//      curl_easy_setopt(curl, CURLOPT_POST, 1);
//	      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
//	    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
//	//      curl_easy_setopt(curl, CURLOPT_POSTFIELDS,"file=\"/home/spta/workspaceTest/curlTest/Debug/test.flac\"");
//	//      curl_easy_setopt(curl, CURLOPT_POSTFIELDS,"filename=test.flac");
//	//      curl_easy_setopt(curl,CURLOPT_UPLOAD,1);
//	//      curl_easy_setopt(curl,CURLOPT_READDATA,fin);
//	    res = curl_easy_perform(curl);
//
//	    /* always cleanup */
//	    curl_easy_cleanup(curl);
//
//	    /* then cleanup the formpost chain */
//	    curl_formfree(formpost);
//	    /* free slist */
//	    curl_slist_free_all (headerlist);
//	  }
}


void ofxGSTTTranscriptor::progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	(void)encoder, (void)client_data;

#ifdef _MSC_VER
	fprintf(stderr, "wrote %I64u bytes, %I64u/%u samples, %u/%u frames\n", bytes_written, samples_written, total_samples, frames_written, total_frames_estimate);
#else
	fprintf(stderr, "wrote %llu bytes, %llu/%u samples, %u/%u frames\n", bytes_written, samples_written, total_samples, frames_written, total_frames_estimate);
#endif
}
