#pragma once
/*
Simple delayline for reverb
*/
#pragma once
#include <math.h>
#include <iostream>
class FixedDelayLine
{
public:
	void initDelay(int _delayInSamples, double _fs)
	{
		fs = _fs;
		setDelayLengthInSamples(_delayInSamples);

		for (int i = 0; i < maxDelayTime; i++)
			delay[i] = 0.0f;
	}

	void setDelayTime(float _delayInSeconds)
	{

		if (_delayInSeconds < 0) {
			std::cout << "Time cannot be negigive\n";
		}
		delayInSeconds = _delayInSeconds;
		delayInSamples = ceil(fs * delayInSeconds);

		if (delayInSamples > maxDelayTime)
			delayInSamples = maxDelayTime;
	}

	void setDelayLengthInSamples(int lengthInSamples)
	{
		delayInSamples = lengthInSamples;

		if (delayInSamples > maxDelayTime)
			delayInSamples = maxDelayTime;
	}

	void tick(float input)
	{
		delay[pos] = input;
		pos = (pos + 1) % (delayInSamples);
	}

	float getOutput()
	{
		int readPos = pos - delayInSamples;

		if (readPos < 0)
			readPos += delayInSamples;

		return delay[readPos];
	}

private:
	static const int maxDelayTime = 2000;
	float delayInSeconds;
	unsigned int delayInSamples = 0;
	unsigned int pos = 0;

	double fs = 48000;
	float delay[maxDelayTime] = {};
};