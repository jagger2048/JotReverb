#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct 
{
	size_t bufferMask;
	size_t delayLen_;
	size_t curPos;		// current position
	float* delayLine;
}
DelayLine;

DelayLine* newDelayLine(size_t delayLen) {
	int k = 1;
	while (k<delayLen)
	{
		k <<= 1;
	}
	DelayLine* obj = NULL;
	obj = (DelayLine*)malloc(sizeof(DelayLine));
	if (obj)
	{
		obj->delayLine = (float*)malloc(sizeof(float)*k);
		memset(obj->delayLine, 0, sizeof(float)*k);
		obj->delayLen_ = delayLen;
		obj->bufferMask = k-1;
		obj->curPos = 0;
	}
	return obj;
}

float pushData(DelayLine* obj, float input) {
	float tmp = obj->delayLine[obj->curPos];
	obj->delayLine[obj->curPos] = input;			// push data into delayline
	obj->curPos = (obj->curPos + 1)&obj->bufferMask;	// update current delay position
	return tmp;
}

inline float getDelayData(DelayLine* obj, size_t pos) {
	//	pos: the order of the data you want to get from delayLine
	//	0 <= pos < delayLen, e.g. if pos = 0, it is the last data
	//	you push into the delay line.
	if (pos >= obj->delayLen_ )
	{
		return NULL;			// outoff Max delay length
	}
	return obj->delayLine[(obj->curPos - pos -1 )&obj->bufferMask];
}

float tick(DelayLine* obj, float input) {
	float tmp = getDelayData(obj, obj->delayLen_-1);
	pushData(obj, input);
	return tmp;
}
int freeDelayLine(DelayLine* obj) {
	if (obj==NULL)
	{
		return -1;
	}
	free(obj->delayLine);
	free(obj);
	obj = NULL;
	return 0;
}