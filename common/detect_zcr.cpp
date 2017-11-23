/**
 *
 * Example:
 * CDetectZcr detector = CDetectZcr(handler, 400, 160);
 *
 */
#include "detect_zcr.hpp"

#include <stdlib.h>

CDetectZcr::CDetectZcr(LPCDetectHandler handler, int nFrameWidth, int nFrameShift)
	:m_frameWidth(nFrameWidth), m_frameShift(nFrameShift), m_frameCounter(0),
	 m_ampThreshold(2000), m_flagSendStart(-1)
{
	m_handler = (LPCDetectHandler)handler;

	setZcrThresholdPerSample(60.0/16000.0);
	setMarginSample(3600, 4800);
//	setMarginSample(5000, 5000);

	m_waveBuffer = new CRingFrameBuffer(nFrameShift*10000, nFrameWidth, nFrameShift, sizeof(short));
	m_zcrBuffer  = new CRingFrameBuffer(nFrameShift*10000, m_headMarginSample, nFrameShift, sizeof(char));
}

CDetectZcr::~CDetectZcr()
{
	delete m_waveBuffer;
	delete m_zcrBuffer;
}

void CDetectZcr::setMarginSample(int head, int tail)
{
	m_headMarginSample = head;
	m_tailMarginSample = tail;

	m_zcrThreshold = (int)(m_headMarginSample * m_zcrThresholdPerSample);
	m_longFrameThreshold = (unsigned int)((20.0*16000.0 - head - tail) / m_frameShift - 1);

}

void CDetectZcr::updateZerocrossBuffer(short *data, int len)
{
	int i;
	char zcr_sign;
	static int flag_presample_sign = 1;
	static int is_trig = 0;
	for(i=0; i<len; i++) {
		if(abs(data[i]) > m_ampThreshold) is_trig = 1;
		if(is_trig) {
			if(flag_presample_sign > 0 && data[i] < 0) {
				zcr_sign = 1;
				flag_presample_sign = -1;
				is_trig = 0;
			}
			else if(flag_presample_sign < 0 && data[i] > 0) {
				zcr_sign = 1;
				flag_presample_sign = 1;
				is_trig = 0;
			}
			else {
				zcr_sign = 0;
			}
		} else {
			zcr_sign = 0;
		}
//			flag_presample_sign = (data[i]>0)?1:-1;
		m_zcrBuffer->write(&zcr_sign, 1);
	}
}

void CDetectZcr::putData(short *data, int len)
{
	// とりあえずバッファに格納
	m_waveBuffer->write((char *)data, len);

	// データも出力
	if(m_handler) m_handler->onAllData(data, len);

	// ゼロクロスの位置を算出
	updateZerocrossBuffer(data, len);

	// ゼロクロスによる発話検出
	{
		int i;

//		printf("\n");
		while(m_zcrBuffer->hasNext()) {
			// ゼロクロスカウント
			int zcr_count = 0;
			char *zcr_buf = m_zcrBuffer->readFrame();
			int frame_width = m_zcrBuffer->getFrameWidth();
			for(i=0; i<frame_width; i++) {
//				if(zcr_buf[i] > 0) ++zcr_count;
				zcr_count += zcr_buf[i];
			}
			++m_frameCounter;

//			printf("%d/", zcr_count);

			// 始端検出
			if(m_flagSendStart == -1 && zcr_count > m_zcrThreshold) {
				// 送信開始
				m_flagSendStart = 0;
				if(m_handler) {
					// 0秒未満になってしまうことへの対応 2007.11.21
					unsigned long pos = m_frameCounter*m_frameShift - m_headMarginSample;
					m_handler->onBegin( (pos>0)?pos:0 );
					m_handler->onData((short *)(m_waveBuffer->read(m_headMarginSample, m_headMarginSample, 1)), m_headMarginSample);
				}
				m_zcrBuffer->setFrameWidth(m_tailMarginSample);
				m_zcrThreshold = (int)(m_tailMarginSample * m_zcrThresholdPerSample);
				// triger
				m_tailMarginCount = m_tailMarginSample;
			}

			// 送信中
			if(m_flagSendStart > -1) {
//				if(m_handler)m_handler->onData((short *)m_waveBuffer->read(m_frameShift, 0, 1), m_frameShift);
				if(m_handler)m_handler->onData((short *)m_waveBuffer->read(m_frameShift, 0), m_frameShift);

				m_flagSendStart++;
				// 終端検出
				if(zcr_count > m_zcrThreshold) {
					//retriger
					m_tailMarginCount = m_tailMarginSample;
//					printf("(%d>%d)", zcr_count, m_zcrThreshold);
//					printf(".");
				} else {
					m_tailMarginCount -= m_frameShift;
				}

				// 終了条件の判別
				// Juliusの制約で20秒以上は勝手に区切られてしまうことに対応 2007.10.09
				if( (m_tailMarginCount < 0) || (m_flagSendStart >= m_longFrameThreshold) ) {
					// 送信終了
					if(m_handler) {
//							printf("%d,", m_flagSendStart);
//							if(m_flagSendStart > 5) {
							m_handler->onEnd(m_frameCounter*m_frameShift + m_tailMarginSample);
//							} else {
//								m_handler->onEnd(0);
//							}
					}
					m_flagSendStart = -1;
					m_zcrBuffer->setFrameWidth(m_headMarginSample);
					m_zcrThreshold = (int)(m_headMarginSample * m_zcrThresholdPerSample);
				}
			}
			// 音声バッファもスキップしとく
			m_waveBuffer->skipFrame();

		}
	}
}

/**
 * 処理途中のデータがある場合でも現時点までで範囲を完結させて、
 * handlerのonEndを呼び出す。
 */
void CDetectZcr::flushData()
{
	if(m_flagSendStart > -1) {
		if(m_handler) {
			m_handler->onEnd(m_frameCounter*m_frameShift + m_tailMarginSample);
		}
		m_flagSendStart = -1;
		m_zcrBuffer->setFrameWidth(m_headMarginSample);
		m_zcrThreshold = (int)(m_headMarginSample * m_zcrThresholdPerSample);
	}
}

CRingFrameBuffer::CRingFrameBuffer(long nBufferLength, int nFrameWidth, int nFrameShift, size_t nSamplePerBytes)
	:m_samplePerBytes(nSamplePerBytes),
	 m_absoluteReadPos(0), m_posRead(0), m_posWrite(0)
{
	m_data = (char *)calloc(nBufferLength, nSamplePerBytes);
	m_frameData = (char *)calloc(nFrameWidth, nSamplePerBytes);

	m_bufferBytes     = nBufferLength * nSamplePerBytes;
	m_frameBytes      = nFrameWidth * nSamplePerBytes;
	m_frameShiftBytes = nFrameShift * nSamplePerBytes;

	m_frameWidthMax = nFrameWidth;

	m_outBuffer = NULL;
	m_outBufferLength = 0;
}


CRingFrameBuffer::~CRingFrameBuffer()
{
	if(m_data) free(m_data);
	if(m_frameData) free(m_frameData);
	if(m_outBuffer) free(m_outBuffer);
}

void CRingFrameBuffer::setFrameWidth(int nFrameWidth)
{
	if(m_frameWidthMax < nFrameWidth) {
		m_frameData = (char *)realloc(m_frameData, nFrameWidth*m_samplePerBytes);
		m_frameWidthMax = nFrameWidth;
	}

	m_frameBytes = nFrameWidth * m_samplePerBytes;

}

int CRingFrameBuffer::hasNext()
{
	int posRead, posWrite;

	posRead = m_posRead + m_frameBytes;
	if(m_posRead > m_posWrite) {
		posWrite = m_posWrite + m_bufferBytes;
	} else {
		posWrite = m_posWrite;
	}

//	printf("--%d(%d),%d(%d)--\n", m_posRead, posRead, m_posWrite, posWrite);

	return (posRead < posWrite);
}

void  CRingFrameBuffer::skipFrame()
{
	m_posRead = (m_posRead + m_frameShiftBytes) % m_bufferBytes;
	m_absoluteReadPos += m_frameShiftBytes;
}

char* CRingFrameBuffer::readFrame()
{
//	printf("r");
	if(m_posRead + m_frameBytes > m_bufferBytes) {
		int len1 = m_bufferBytes - m_posRead;
		memcpy(m_frameData, &m_data[m_posRead], len1);
		memcpy(&m_frameData[len1], &m_data[0], m_frameBytes - len1);
	} else {
		memcpy(m_frameData, &m_data[m_posRead], m_frameBytes);
	}

	skipFrame();
	return m_frameData;
}

char* CRingFrameBuffer::read(int length, int nBackPosition, int forwardPosition)
{
	if(m_outBufferLength < length) {
		m_outBufferLength = length;
		m_outBuffer = (char *)realloc(m_outBuffer, length * m_samplePerBytes);
	}
	read(m_outBuffer, length, nBackPosition, forwardPosition);

	return m_outBuffer;
}

int CRingFrameBuffer::read(char *dest, int length, int nBackPosition, int forwardPosition)
{
	int posRead = m_posRead - nBackPosition*m_samplePerBytes - m_frameShiftBytes;
	int bytes = length * m_samplePerBytes;

//	printf("R");
	if(posRead < 0) {
		int startBufPos = m_bufferBytes + posRead;
		int startBufLen = -posRead;
		if(startBufLen > bytes) {
			memcpy(dest, &m_data[startBufPos], bytes);
		} else {
			memcpy(dest, &m_data[startBufPos], startBufLen);
			memcpy(&dest[startBufLen], &m_data[0], bytes - startBufLen);
		}
	} else {
		memcpy(dest, &m_data[posRead], bytes);
	}

	if(forwardPosition) setReadPos(m_posRead + bytes - nBackPosition*m_samplePerBytes);

	return (m_absoluteReadPos - m_frameShiftBytes)/m_samplePerBytes - nBackPosition;
}

void CRingFrameBuffer::write(char *src, int length)
{
//	printf("w");
	int bytes = length * m_samplePerBytes;
	if(m_posWrite + bytes > m_bufferBytes) {
		int len1 = m_bufferBytes - m_posWrite;
		memcpy(&m_data[m_posWrite], src, len1);
		memcpy(&m_data[0], &src[len1], bytes - len1);
	} else {
		memcpy(&m_data[m_posWrite], src, bytes);
	}

	m_posWrite = (m_posWrite + bytes) % m_bufferBytes;
}
