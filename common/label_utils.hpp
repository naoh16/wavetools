/**
 * ラベルファイル操作用のユーティリティ
 *
 * Copyright (C) 2017 Sunao HARA, All rights reserved.
 */
#ifndef _LABEL_UTILS_H_
#define _LABEL_UTILS_H_

#ifndef UNICODE
#  define UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#ifdef CHOMP
#undef CHOMP
#endif
#define CHOMP(x) {char *p; if((p=strrchr(x,'\n')) != NULL) *p='\0'; if((p=strrchr(x,'\r')) != NULL) *p='\0';}

/**
 * ラベルファイル構造体
 */
typedef struct _labeldata{
	double      start;
	double      end;
	std::string label;
	std::string data;
} AREALABEL, *LPAREALABEL;

/**
 * マーカーラベルファイル構造体
 */
typedef struct _markerlabeldata{
	double      start;
	std::string label;
	std::string data;
} MARKERLABEL, *LPMARKERLABEL;

typedef std::vector<AREALABEL> AreaLabels;
typedef std::vector<MARKERLABEL> MarkerLabels;

AreaLabels read_arealabel(char* filename);
MarkerLabels read_markerlabel(char* filename);

#endif /* ifndef _LABEL_UTILS_H_ */
