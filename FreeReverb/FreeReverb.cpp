// FreeReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FreeReverb.hpp"
#include "AudioFile.cpp"
#include <iostream>
#include <vector>
using namespace std;
int main()
{
	AudioFile<double> af;
	af.load("dukou_noReverb.wav");
	af.printSummary();
	Freeverb fb;
	fb.setSampleRate(af.getSampleRate());
	//fb.setDamping(0.2);
	//fb.setFeedback(0.5);

	vector<double> output;
	cout << "Ready to process\n";
	for (auto data : af.samples.at(0)) {
		output.push_back( 0.7 *fb.process(data) );
	}
	cout << "Ready to output\n";
	// find max
	double max = 0;
	for (auto mData : output) {
		if (mData > max)
			max = mData;
	}
	// 归一化
	for (auto &mData : output) {
		mData = mData / (max + 0.1);
	}

	vector<vector<double>> out(1);
	out.at(0).assign(output.begin(), output.end());
	af.setAudioBuffer(out);
	af.save(" FreeReveb.wav");
    return 0;
}
	
