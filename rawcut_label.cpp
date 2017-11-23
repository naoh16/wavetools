/**
 * speech file segmentation by label file
 *
 *    Last Modified: 2008/01/13 11:43:55.
 *
 *    Copyright (c) 2006-2008 HARA Sunao, All rights reserved.
 */
#define UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "common/label_utils.hpp"

#define DEF_FREQ 16000
#define DEF_CHANNEL 1
#define DEF_SAMPLEBYTE 2

/**
 *  行数カウント
 */
long count_lines(FILE *fp)
{
	int c;
	long count=0L;
	long orgpos = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	while( (c = fgetc(fp)) != EOF){
		if(c == '\n') ++count;
	}
	fseek(fp, orgpos, SEEK_SET);
	return count;
}

/**
 *  ファイルサイズカウント
 */
unsigned long count_filesize(FILE *fp)
{
	long count=0L;
	long orgpos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	count = ftell(fp);
	fseek(fp, orgpos, SEEK_SET);
	return count;
}

double get_max_duration(AreaLabels& labels)
{
	AreaLabels::iterator ite;
	double bl, max_byte_length = 0.0;
	for(ite=labels.begin(); ite!=labels.end(); ite++) {
		AREALABEL lab = *ite;
		bl = lab.end - lab.start;
		if(bl > max_byte_length) max_byte_length = bl;
	}
	return max_byte_length;
}

void swapbytes_as_int16(unsigned char* array_data, int length) {
    unsigned char tmp;
    int j;
    for(j=0; j<length; j+=2) {
        tmp = array_data[j];
        array_data[j] = array_data[j+1];
        array_data[j+1] = tmp;
    }
}

int wavecut_by_label(char *wavefile, char *labelfile, char *output_basename, long frequency, int fSwapBytes)
{
	AreaLabels labels;
	int i;

	char outputfilename[BUFSIZ];

	int nSampleByte = DEF_SAMPLEBYTE;
	int nChannel    = DEF_CHANNEL;

	long nSecondToSample = frequency * nChannel;

	FILE *fp, *fpOut;

	// LABELファイル読み込み
	fprintf(stderr, "read_labeldata.\n");
	labels = read_arealabel(labelfile);

	// 音声ファイルオープン
	fp = fopen(wavefile, "rb");
	if(fp == NULL) {
		fprintf(stderr, "File Open Error: %s\n", wavefile);
		return -1;
	}

	// 事前にバッファ確保
	unsigned char *buf = (unsigned char *)malloc( (long)(get_max_duration(labels) * nSecondToSample) * nSampleByte );
	if(buf == NULL) {
		fprintf(stderr, "memory alloocation error.\n");
		return -1;
	}

	AreaLabels::iterator ite;
	i = 0;
	for(ite=labels.begin(); ite!=labels.end(); ite++) {
		long start_byte, byte_length, nread;
		AREALABEL lab = *ite;

		i++;

		if(fSwapBytes) {
			sprintf(outputfilename, "%s%03d.be", output_basename, i);
		} else {
			sprintf(outputfilename, "%s%03d.le", output_basename, i);
		}
		start_byte = (long)(lab.start * nSecondToSample) * nSampleByte;
		byte_length   = (long)((lab.end - lab.start) * nSecondToSample) * nSampleByte;

		fseek(fp, start_byte, SEEK_SET);

		nread = fread(buf, sizeof(unsigned char), byte_length, fp);

		fpOut = fopen(outputfilename, "wb");
		if(fpOut == NULL) {
			fprintf(stderr, "Outputfile open error: %s\n", outputfilename);
			free(buf);
			return -1;
		}

		if(fSwapBytes) swapbytes_as_int16(buf, byte_length);
		fwrite(buf, sizeof(unsigned char), nread, fpOut);

		fclose(fpOut);

		printf("%s : %f -> %f\n", outputfilename, lab.start, lab.end);
	}

	fclose(fp);
	free(buf);

	return 0;
}

/**
 * Main function
 */
int main(int argc, char* argv[])
{
	char *wavefile, *labelfile, *output_basename;
	long frequency = DEF_FREQ;
	int fSwapBytes = 0;
	int idx = 1;

	if(argc <= 3){
		fprintf(stderr, "usage: %s [-x] wavefile labelfile output_basename [frequency] \n", argv[0]);
		return 1;
	}
	if( strcmp(argv[idx], "-x") == 0 ) {
		++idx;
		fSwapBytes = 1;
	}
	wavefile  = argv[idx++];
	labelfile = argv[idx++];
	output_basename = argv[idx++];
	if(argv[idx]) frequency = atol(argv[idx++]);

	return wavecut_by_label(wavefile, labelfile, output_basename, frequency, fSwapBytes);
}
