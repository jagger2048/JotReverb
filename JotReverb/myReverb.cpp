// JotReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include "myFreeReverb.hpp"
#include <iostream>
#include <vector>
#include <math.h>
#include "JotReverb.hpp"
#include "FDN.hpp"
using namespace std;

//#define OUTPUT
#define LOAD_DATA
//#define NORMALIZE


int main()
{
#ifdef LOAD_DATA
	// Load audio data
	AudioFile<double> af;
	af.load("ideal_impulse_48k.wav");
	//af.load("dukou_noReverb.wav");
	af.printSummary();
	vector<double> data;		// load mono data for test
	if (af.getNumChannels() > 1)
	{
		for (size_t nCount = 0; nCount != af.getNumSamplesPerChannel(); ++nCount) {
			data.push_back( (af.samples.at(0).at(nCount) + af.samples.at(1).at(nCount)) *0.5 );
		}
	}
	else
		data.assign( (af.samples.at(0)).begin(), (af.samples.at(0)).end() );	
#endif // LOAD_DATA

	// initialize the reverb unit
	myFreeReverb Fb;
	Fb.init();

	JotReverb Jb;
	Jb.init();

	FDN fn;
	fn.init(4, vector<double>{1, 1, 1, 1}, vector<double>{1, 1, 1, 1}, vector<double>{1, 1, 1, 1});

	// initialize the processing 
	const unsigned int N = 256;							// the number of samples per frame
	vector<double> data_processed;						// a vector wrap the output data
	auto it = data.begin();
	vector<double> dFramePack(N);						// wrap the audio data into Frame
	vector<double> dFrameOut(N);						// wrap the processed data
	// start to process
	cout << "Start to precess\n";

	for (size_t n = 0; n != (data.size() / N) * N; n += N) {
		if (it <= data.end()) {
			//vector<double> dFramePack(it, it + N);
			dFramePack.assign(it, it + N);

			// your processing here
			//Fb.run_by_frame(dFramePack, dFrameOut);

			//Jb.process(dFramePack, dFrameOut);

			for (size_t nn = 0; nn != N; ++nn) {				// output 
				data_processed.push_back(dFrameOut.at(nn));
			}

		}
		it += N;												// update the iterator	
	}

#ifdef OUTPUT
#ifdef NORMALIZE
	 //find max
	double max = 0;
	for (auto mData : data_processed) {
		if (mData > max)
			max = mData;
	}
	// 归一化
	for (auto &mData : data_processed) {
		mData = mData / (max +0.1);
	}
#endif // NORMALIZE

	cout << "Ready to output\n";
	vector<vector<double>> outputBuffer(1);
	outputBuffer.at(0).assign(data_processed.begin(), data_processed.end());
	//outputBuffer.at(1).assign(data_processed.begin(), data_processed.end());
	af.setAudioBuffer(outputBuffer);
	//af.samples.at(0).assign(data_processed.begin(), data_processed.end());
	//af.samples.at(1).assign(data_processed.begin(), data_processed.end());
	af.save("FreeRev_impulse_v1.wav");
#endif
    return 0;
}