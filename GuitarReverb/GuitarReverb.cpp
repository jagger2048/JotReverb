

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

	LBCF a1;
	a1.init(1600, fs);
	
	LowpassFeedbackCombFilter* lbcf = newLBCF(0.2, 0.84, 1600);
	for (size_t n = 0; n < len; n++)
	{
		out[n] = runLBCF(lbcf, wavfile->pDataFloat[0][n]);
		//out[n] = a1.getOutput(wavfile->pDataFloat[0][n]);
	}
	wavfile_destory(wavfile);
	wavfile_write_f32("lcpf out33.wav", &out, len, 1, fs);
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

int main()
{
	lbcf_unittest();
	//delay_unittest();
	//int a = (-2) & 7;
	//printf("%d",a );
	return 0;
}


