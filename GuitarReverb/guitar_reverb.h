#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "delay_line.h"
typedef struct {
	float damp_;		//	damp = initialDamp * sacleDamp = 0.5*0.4 = 0.2
	float f_;		//  lowpass scale-factor , f = roomsize = initialRoom * scaleRoom + offsetRoom = 0.5*0.28+0.7=0.84
	size_t delayLen_;
	DelayLine* delayLine;
	float lpfOut;
}
LowpassFeedbackCombFilter; // LBCF

LowpassFeedbackCombFilter* newLBCF(float damp,float f,size_t delayLen) {
	LowpassFeedbackCombFilter* obj = NULL;
	obj = (LowpassFeedbackCombFilter*)malloc(sizeof(LowpassFeedbackCombFilter));
	if (!obj)
	{
		return NULL;
	}
	memset(obj, 0, sizeof(LowpassFeedbackCombFilter));
	obj->damp_ = damp;
	obj->f_ = f;
	obj->delayLen_ = delayLen;
	obj->delayLine = newDelayLine(obj->delayLen_);
	return obj;
}
void setLBCF(LowpassFeedbackCombFilter* obj, float damp, float f, size_t delayLen) {

}
float runLBCF(LowpassFeedbackCombFilter* obj, float in) {
	// update lpfOut
	// unity-gain onepole lpf: y(n) = damp * y(n-1) +(1-damp) * x(n)
	obj->lpfOut = obj->damp_ * obj->lpfOut + getDelayData(obj->delayLine, obj->delayLen_ - 1) * (1 - obj->damp_);

	return tick(obj->delayLine, obj->lpfOut*obj->f_ + in);
}

int freeLBCF(LowpassFeedbackCombFilter* obj) {
	if (obj != NULL)
	{
		free(obj->delayLine);
		free(obj);
		obj = NULL;
		return 0;
	}
	return -1;
}