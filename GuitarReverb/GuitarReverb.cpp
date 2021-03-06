

#include <stdint.h>
#include <stdio.h>
#include "guitar_reverb.h"
#include "wavfile.h"
#include "dr_wav.h"
#ifndef DR_WAV_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#endif
#include "freeReverb.hpp"
void lbcf_unittest() {

	WAV* wavfile = wavfile_read("dukou_noReverb.wav");
	size_t len = wavfile->totalPCMFrameCount;
	size_t fs = wavfile->sampleRate;
	float* out = (float*)malloc(sizeof(float)*len);

	LBCF lbcf2;
	lbcf2.init(800, fs);
	
	LowpassFeedbackCombFilter* lbcf = newLBCF(0.2, 0.84, 800);
	for (size_t n = 0; n < len; n++)
	{
		if (n ==0)
		{
			//out[n] = runLBCF(lbcf, 1);
			//out[n] = lbcf2.getOutput(wavfile->pDataFloat[0][n]);
			out[n] = lbcf2.getOutput(1);
		}
		else
		{
			//out[n] = runLBCF(lbcf, 0);
			out[n] = lbcf2.getOutput(0);

		}

	}
	wavfile_destory(wavfile);
	wavfile_write_f32("lcpf out-1600 2.wav", &out, len, 1, fs);
	free(out);
	freeLBCF(lbcf);
}

void delay_unittest() {
	DelayLine* d;
	d = newDelayLine(5);
	for (size_t n = 1; n <= 25; n++)
	{
		auto tmp = tick(d, n);
		printf("in: %d  out: %f\n",n,tmp);
		printf("\ndelay line %d :\n",d->curPos);
		for (size_t i = 0; i < d->delayLen_; i++)
		{
			float tmp = getDelayData(d, i);
			printf(" %f ",tmp);
		}
		printf("\n----------------------\n");

	}

	freeDelayLine(d);
}

void allpass_unittest() {
	FRAllpass* ap = newFRAllpass(30, 0.5);
	WAV* wavfile = wavfile_read("dukou_noReverb.wav");
	size_t len = wavfile->totalPCMFrameCount;
	size_t fs = wavfile->sampleRate;

	float* out = (float*)malloc(sizeof(float)*len);

	float* inSeq = (float*)malloc(sizeof(float) * 300);
	for (size_t i = 0; i < 300; i++)
	{
		inSeq[i] = 1;
	}
	FreeAP ap2;
	ap2.init(30, fs);
	float tmp = 0;
	for (size_t n = 0; n < 300; n++)
	{
		//out[n] = runFRAllpass(ap, wavfile->pDataFloat[0][n]);
		//out[n] = ap2.getOutput(wavfile->pDataFloat[0][n]);
		tmp = runFRAllpass(ap, inSeq[n]);
		printf("%d : %f ",n , tmp);
		tmp = ap2.getOutput(inSeq[n]);
		printf(" %f \n", tmp);
	}
	wavfile_destory(wavfile);
	//wavfile_write_f32("allpass 300 0.5 - 1.wav",&out, len, 1, fs);
	freeFRAllpass(ap);
	free(out);
}

void freeReverb_unittest() {
	unsigned int lcbfLength[8] = { 1557,1617,1491,1422,1277,1356,1188,1116 };
	unsigned int apLength[4] = { 225,556,441,341 };
	float damp = 0.2;
	float feedback = 0.64;
	float apGain = 0.5;
	FreeReverb* fr = newFreeReverb(damp, feedback, apGain, lcbfLength, apLength);
	WAV* wavfile = wavfile_read("dukou_noReverb.wav");
	size_t len = wavfile->totalPCMFrameCount;
	size_t fs = wavfile->sampleRate;
	float* out = NULL;
	out = (float*)malloc(sizeof(float)*len);
	printf("len :%d \n", len);
	if (out == NULL)
	{
		printf("error\n");
	}
	Freeverb fr2;
	fr2.setDamping(damp);
	fr2.setFeedback(feedback);
	fr2.setSampleRate(fs);
	float impulse[48000] = {0};
	impulse[0] = 1;
	for (size_t n = 0; n < len; n++)
	{
		//out[n] = runFreeReverb(fr, impulse[n]);
		//out[n] = fr2.process(impulse[n]);

		//out[n] = fr2.process(wavfile->pDataFloat[0][n]);

		if (n == 0)
		{
			//out[n] = fr2.process(1);
			out[n] = runFreeReverb(fr, 1);

		}
		else
		{
			//out[n] = fr2.process(0);
			out[n] = runFreeReverb(fr, 0);
		}
	}

	wavfile_destory(wavfile);
	wavfile_write_f32("freeReverb demo1-i.wav", &out, len, 1, fs);
	freeFreeReverb(fr);
	free(out);


}
int main()
{
	freeReverb_unittest();
	//lbcf_unittest();
	//////allpass_unittest();
	//delay_unittest();

	return 0;
}


