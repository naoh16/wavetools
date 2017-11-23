/**
 * ラベルファイル操作用のユーティリティ
 *
 * Copyright (C) 2017 Sunao HARA, All rights reserved.
 */

#include "label_utils.hpp"

/**
 *  マーカーラベルファイル読み込み
 *
 *  @return 読み込んだ行数(エラー時: -1)
 */
MarkerLabels read_markerlabel(char* filename)
{
	FILE *fp;
	char buf[BUFSIZ];
	char *p;

	MarkerLabels labels;

	fp = fopen(filename, "rt");
	if(fp == NULL) return labels;
//	printf("%s\n", filename);

//	cnt = 0;
	while( fgets(buf, BUFSIZ, fp) != NULL ) {
		MARKERLABEL newlab;
		CHOMP(buf);
		if(buf[0] == '#' || buf[0] == '\0') continue;
		p = strtok(buf, "\t ");
		if(p != NULL) {
			char *label, *data;
			newlab.start = atof(p);
			label = strtok(NULL, "\t \n");
			data  = strtok(NULL, "\t\n");
			newlab.label = (label!=NULL)?label:"";
			newlab.data = (data!=NULL)?data:"";
			labels.push_back(newlab);
//			printf("<%f> <%s> <%s>\n", newlab.start, newlab.label.c_str(), newlab.data.c_str());
//			cnt++;
		}
	}
	fclose(fp);
	return labels;
}

/**
 *  範囲ラベルファイル読み込み
 *
 *  @return 読み込んだ行数(エラー時: -1)
 */
AreaLabels read_arealabel(char* filename)
{
	FILE *fp;
	char buf[BUFSIZ];
	char *p;

	AreaLabels labels;

	fp = fopen(filename, "rt");
	if(fp == NULL) return labels;
	printf("%s\n", filename);

//	cnt = 0;
	while( fgets(buf, BUFSIZ, fp) != NULL ) {
		AREALABEL newlab;
		CHOMP(buf);
		if(buf[0] == '#' || buf[0] == '\0') continue;
		p = strtok(buf, "\t ");
		if(p != NULL) {
			char *label, *data;
			newlab.start = atof(p);
			newlab.end   = atof(strtok(NULL, "\t "));
			label = strtok(NULL, "\t ");
			data  = strtok(NULL, "\t\n");
			newlab.label = (label!=NULL)?label:"";
			newlab.data = (data!=NULL)?data:"";
			labels.push_back(newlab);
//			printf("%f %f %s %s\n", newlab.start, newlab.end, newlab.label.c_str(), newlab.data.c_str());
//			cnt++;
		}
	}
	fclose(fp);
	return labels;
}
