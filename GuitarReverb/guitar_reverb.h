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

//========Allpass filter========//
typedef struct {
	size_t delayLen_;
	float gain_;
	DelayLine* dealyLine;
}
FRAllpass;
FRAllpass* newFRAllpass(size_t delayLen, float gain) {
	FRAllpass* obj = NULL;
	obj = (FRAllpass*)malloc(sizeof(FRAllpass));
	if (obj == NULL)
	{
		return NULL;
	}
	obj->delayLen_ = delayLen;
	obj->gain_ = gain;
	obj->dealyLine = newDelayLine(obj->delayLen_);
	return obj;
}
float runFRAllpass(FRAllpass* obj, float in) {
	// formalua:
	// y(n) = g*y(n-N) - g*x(n)+x(n-N)

	float tmp = getDelayData(obj->dealyLine, obj->delayLen_ - 1);	// get the delayed data in delay line
	float out = in *( - obj->gain_) + tmp;
	//float out = in *( - 1) + tmp;
	tick(obj->dealyLine, in + obj->gain_ * out);
	return out;
}
int freeFRAllpass(FRAllpass* obj) {
	if (obj != NULL)
	{
		freeDelayLine(obj->dealyLine);
		free(obj);
		obj = NULL;
		return 0;
	}
	return -1;
}

typedef struct 
{
	LowpassFeedbackCombFilter* LBCF[8];
	FRAllpass* AP[4];

	float feedback_;		// 0.84;
	float damping_;			// 0.2;
	double samplerate_;		// 48000;
	float apGain_;			// 0.5

	unsigned int* lbcfLength_;
	unsigned int* apLength_;

}
FreeReverb;

FreeReverb* newFreeReverb(float damp,float feedback,float apGain, unsigned int* lbcfLength_, unsigned int* apLength_) {
	FreeReverb* obj = NULL;
	obj = (FreeReverb*)malloc(sizeof(FreeReverb));
	if (obj == NULL)
	{
		return NULL;
	}
	obj->lbcfLength_ = (unsigned int*)malloc(sizeof(unsigned int) * 8);
	obj->apLength_ = (unsigned int*)malloc(sizeof(unsigned int) * 4);
	obj->apGain_ = apGain;
	obj->feedback_ = feedback;
	obj->damping_ = damp;

	for (size_t n = 0; n < 8; n++)
	{
		obj->lbcfLength_[n] = lbcfLength_[n];
		obj->LBCF[n] = newLBCF(damp, feedback, obj->lbcfLength_[n]);
	}
	for (size_t n = 0; n < 4; n++)
	{
		obj->apLength_[n] = apLength_[n];
		obj->AP[n] = newFRAllpass(obj->apLength_[n], 0.5);
	}
	return obj;
}
float runFreeReverb(FreeReverb* obj, float in) {
	float out = 0.0f;
	float tmp = 0.0f;
	for (size_t n = 0; n < 8; n++)
	{
		tmp = runLBCF(obj->LBCF[n], in);
		out = tmp + out;
	}
	out *= 0.125;
	//out = in;
	//for (size_t n = 0; n < 4; n++)
	//{
	//	out = runFRAllpass(obj->AP[n], out);
	//}
	return out;
}
int freeFreeReverb(FreeReverb* obj) {
	if (obj != NULL)
	{
		for (size_t n = 0; n < 8; n++)
		{
			freeLBCF(obj->LBCF[n]);
		}
		for (size_t n = 0; n < 4; n++)
		{
			freeFRAllpass(obj->AP[n]);
		}
		free(obj);
		obj = NULL;
		return 0;
	}
	return -1;
}