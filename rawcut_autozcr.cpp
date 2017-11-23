/**
 * 振幅とゼロクロス回数で音声区間検出し時間を出力する
 *
 * Copyright (C) 2017 HARA, Sunao, All rights reserved.
 *
 */
#define UNICODE

#include <stdio.h>
#include <stdlib.h>

#include "common/detect_zcr.hpp"
#include "common/label_utils.hpp"

#define WAVEBUFFER_LENGTH 1024
#define DEF_SAMPLINGRATE 16000

/**
 * Usage
 */
int usage()
{
	fprintf(stderr, "usage: wavcut_autozcr wave_filename\n");
	exit(1);
}

/**
 * Commandline Options
 */
struct _global_options {
	int sampling_rate;
	int frame_width;
	int frame_shift;
	char* threshold_filename;
} gopt;

int mygetopts(int argc, char* argv[])
{
	int ai = 1;

	for(ai=1; ai<argc; ++ai) {
		fprintf(stderr, "%d: %s\n", ai, argv[ai]);

		if( (argv[ai][0] == '-') && (strlen(argv[ai]) == 2) && (ai != (argc-1)) ) {
			switch(argv[ai][1]) {
			case 's': /// sampling rate
				gopt.sampling_rate = atoi(argv[++ai]);
				break;
			case 'f':
				gopt.frame_shift = atoi(argv[++ai]);
				break;
			case 'w':
				gopt.frame_width = atoi(argv[++ai]);
				break;
			case 't':
				gopt.threshold_filename = argv[++ai];
				break;
			case '-':
				goto END_OF_GOPTPARSE;
				break;
			default:
				usage();
				break;
			}
		} else {
			break; // for(ai=1; ai<argc; ++ai)
		}
	}
END_OF_GOPTPARSE:
	fprintf(stderr, "return %d\n", ai);
	return ai;
}

/**
 * Define detect handler for this application.
 */
class CAppDetectHandler : public CDetectHandler
{
public:
	void onBegin(unsigned long pos);
	void onAllData(short *data, int len){}; // nop.
	void onData(short *data, int len){};    // nop.
	void onEnd(unsigned long pos);
};

void CAppDetectHandler::onBegin(unsigned long pos)
{
	printf("%f ", (double)pos / (double)gopt.sampling_rate);
}

void CAppDetectHandler::onEnd(unsigned long pos)
{
	printf("%f\n", (double)pos / (double)gopt.sampling_rate);
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
	FILE *fp = NULL;

	if(argc <= 1){
		usage();
	}

	// default value
	gopt.sampling_rate = DEF_SAMPLINGRATE;
	gopt.frame_width = (int)((double)DEF_SAMPLINGRATE * 0.025); /*25ms = 400pt(16kHz)*/
	gopt.frame_shift = (int)((double)DEF_SAMPLINGRATE * 0.010); /*10ms = 160pt(16kHz)*/

	int ai = mygetopts(argc, argv);

	char* wavefilename = argv[ai];
	int nFrameWidth = gopt.frame_width;
	int nFrameShift = gopt.frame_shift;

	MarkerLabels::iterator th_ite;
	MarkerLabels th_labels = read_markerlabel(gopt.threshold_filename);

	// convert times[sec] to samples
	for(th_ite=th_labels.begin(); th_ite!=th_labels.end(); th_ite++) {
		th_ite->start = th_ite->start * gopt.sampling_rate;
	}

	fprintf(stderr, "Load: %s\n", wavefilename);
	fp = fopen(wavefilename, "rb");
	if(fp == NULL) {
		fprintf(stderr, "Error: can't open the file(%s).", wavefilename);
		usage();
	}

	CAppDetectHandler* handler = new CAppDetectHandler();
	CDetectZcr* adincut = new CDetectZcr(handler, nFrameWidth, nFrameShift);

	adincut->setAmplitudeThreshold(2000);
	adincut->setZcrThresholdPerSample(60.0/16000.0);
	adincut->setMarginSample(3600, 4800);

	try {
		short wavedata[WAVEBUFFER_LENGTH];
		long nRead;
		long nTotalRead = 0;

		th_ite = th_labels.begin();

		while( (nRead = fread(wavedata, sizeof(short), WAVEBUFFER_LENGTH, fp)) > 0) {
			if( th_ite != th_labels.end() ) {
				if((th_ite)->start <= (float)nTotalRead) {
					fprintf(stderr, "Info: ChangeThreshold: %f: %s\n", (th_ite)->start, (th_ite)->label.c_str());
					adincut->setAmplitudeThreshold(atoi((th_ite)->label.c_str()));
					th_ite++;
				}
			}
//			fprintf(stderr, "%d", (int)adincut->getFrameCounter());
			adincut->putData(wavedata, nRead);
			nTotalRead += nRead;
		}
		adincut->flushData();
	}catch(...){
		fprintf(stderr, "Unknown Error.");
	}

	delete adincut;
	delete handler;

	fclose(fp);

}
