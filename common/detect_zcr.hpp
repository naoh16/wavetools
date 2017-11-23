/**
 * ラベルファイル操作用のユーティリティ
 *
 * Copyright (C) 2017 Sunao HARA, All rights reserved.
 */
#ifndef _DETECT_ZCR_H_
#define _DETECT_ZCR_H_

#ifndef UNICODE
#  define UNICODE
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <string.h>

typedef class CDetectZcr CDetectZcr, *LPCDetectZcr;
typedef class CDetectHandler CDetectHandler, *LPCDetectHandler;
typedef class CRingFrameBuffer CRingFrameBuffer, *LPCRingFrameBuffer;

class CDetectHandler
{
public:
    virtual ~CDetectHandler(){};
	virtual void onBegin(unsigned long pos){};
	virtual void onData(short *data, int len){};
	virtual void onAllData(short *data, int len){};
	virtual void onEnd(unsigned long pos){};
};

class CDetectZcr
{
private:
	LPCDetectHandler   m_handler;
	LPCRingFrameBuffer  m_waveBuffer;
	LPCRingFrameBuffer  m_zcrBuffer;

	int m_frameWidth;
	int m_frameShift;

	unsigned long m_frameCounter;

	double m_zcrThresholdPerSample;

	short m_ampThreshold;
	int m_zcrThreshold;
	int m_headMarginSample;
	int m_tailMarginSample;

	int m_flagSendStart;
	int m_longFrameThreshold;
	int m_tailMarginCount;
public:
	CDetectZcr(LPCDetectHandler handler, int nFrameWidth, int nFrameShift);
	virtual ~CDetectZcr();

	LPCDetectHandler getHandler(){return m_handler;};

	unsigned long getFrameCounter() { return m_frameCounter; };
	void resetFrameCounter(void){ m_frameCounter = 0UL; };

	void setAmplitudeThreshold(short thre){m_ampThreshold = thre;};
	void setZcrThresholdPerSample(double thre){m_zcrThresholdPerSample = thre;};
	void setMarginSample(int head, int tail);

	void putData(short *data, int len);
	void flushData();
private:
	void updateZerocrossBuffer(short *data, int len);

};

class CRingFrameBuffer
{
private:
	int   m_samplePerBytes;
	long  m_bufferBytes;
	int   m_frameBytes;
	int   m_frameWidthMax;
	int   m_frameShiftBytes;

	char* m_data;
	char* m_frameData;

	long  m_absoluteReadPos;
	int   m_posRead, m_posWrite;

	char* m_outBuffer;
	int   m_outBufferLength;
public:
	CRingFrameBuffer(long nBufferLength, int nFrameWidth, int nFrameShift, size_t nSamplePerBytes);
	virtual ~CRingFrameBuffer();

	// read frame
	int   hasNext();
	char* readFrame();
	void  skipFrame();

	// setter/getter
	void setFrameWidth(int width);
	int  getFrameWidth(){ return m_frameBytes/m_samplePerBytes; };
	void setReadPos(int pos){ m_posRead = pos; };
	int  getReadPos(){ return (m_posRead - m_frameShiftBytes) % m_bufferBytes; };
	long getAbsoluteReadPos(){ return m_absoluteReadPos; };

	// I/O
	int   read(char *dest, int length, int nBackPosition, int forwardPosition=0);
	char* read(int length, int nBackPosition, int forwardPosition=0);
	void  write(char *src, int length);

};

#endif // !defined(_DETECT_ZCR_H_)
