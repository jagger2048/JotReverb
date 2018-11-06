// JotReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include <iostream>
#include <vector>
#include <math.h>
#include "FreeReverb.hpp"
using namespace std;

//#define JR_PROCESS
//#define OUTPUT
#define LOAD_DATA
class JotReverb {
public:
	double fs = 48000;						
	int nFrame = 256;						// the number fo data in per frame
	double Un[4] = { 0.4,0.2,0.6,0.8 };
	double J4[4][4] = { { 0,0,1,0 },{ 0,1,0,0 },{ 1,0,0,0 },{ 0,0,0,1} };
	double gain_b[4] = { 0.25,0.25,0.25,0.25 };
	double gain_c[4] = { 1,1,1,1 };
	double delay_length[4] = { 2205,6615,3087,8820 };
	double dry_wet_mix_rate = 0.5;

	// lpf coef 
	double bi[4] = { 0 };
	double gi[4] = { 0 };
	double Tr_pi = 0.3;
	double Tr_0 = 1.757;

	double An[4][4] = { 0 };				// An = J4 - 1 / 4 * ( Un * Un^T )

private:
	double delay_net[4][8820] = { 0 };		// 8820 = max(delay_length)
	unsigned int delay_pos[4] = { 0 };		// current position of delay line per channle  
	double lpf_cache[4] = { 0 };

public:
	double An_out[4] = { 0 };
	void updateAn();						// compute the matrix An
	double updateLpfCoeff();				// compute the lpf coeff
	int init();
	int delay_by_samples(double in, double &after_delay, unsigned int &cur_delay_pos, unsigned int N, double *delay_line);
	int process(vector<double> data_in, vector<double>&data_out);
};

class DelayLine
{
public:
	void delay_by_sample(double data_to_push, double &after_delay);
	double getEnd() {
		return delay_line[cur_pos];
	};
	int init(unsigned int N) {
		delay_line_length = N;
		delay_line = new double[N];

		for (size_t n = 0; n != N; ++n) {
			delay_line[n] = 0;
		}
		return 0;
	};
	DelayLine(unsigned int N);
	DelayLine() {

	};
	~DelayLine();
private:
	unsigned int cur_pos = 0;
	unsigned int delay_line_length;
	double *delay_line;

};

class LBCF
{
	// Lowpass-feedback comb filter
	// ref: FreeReverb https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
public:
	double lpf_f = 0;
	double lpf_d = 0;
	unsigned int N = 0;
	//
	double lpf_fb = 0;
	double lpf_cache = 0;
	DelayLine DELAY ;
	double run_by_sample(double data_in, double &data_out) {
		DELAY.delay_by_sample(data_in + lpf_fb, data_out);					// update the delay line
		lpf_fb = lpf_f * (1 - lpf_d) * DELAY.getEnd() + lpf_d * lpf_cache;	
		lpf_cache = lpf_fb;													// update the lpf cache
		return 0;
	};
	double run_by_frame(vector<double> data_in,vector<double> &data_out) {
		for (size_t n = 0; n != data_in.size(); ++n) {
			run_by_sample(data_in.at(n), data_out.at(n));
		}
		return 0;
	}
	int init(double f, double d, unsigned int delay_len) {
		DELAY.init(delay_len);
		lpf_f = f;
		lpf_d = d;
		N = delay_len;
		return 0;
	};
	LBCF();
	~LBCF();

private:

};

class AP
{
	// delay_line based allpass filter
public:
	DelayLine delay_line;
	double g = 0;
	unsigned N = 0;
	double init(double gain,unsigned delay_line_length) {
		g = gain;
		N = delay_line_length;
		delay_line.init(N);
		return 0;
	};
	double run_by_sample(double data_in, double &data_out) {
		//double tmp =0;
		delay_line.delay_by_sample( g*data_out + data_in,data_out);
		data_out = -g * data_in + data_out;
		return 0;
	};
	double run_by_frame(vector<double> data_in, vector<double> &data_out) {
		for (size_t n = 0; n != data_in.size(); ++n) {
			run_by_sample(data_in.at(n), data_out.at(n));
		}
		return 0;
	};
	AP();
	~AP();

private:

};

AP::AP()
{
}

AP::~AP()
{
}


int main()
{
#ifdef LOAD_DATA
	// Load audio data
	AudioFile<double> af;
	af.load("audioCut_2.wav");
	af.printSummary();
	vector<double> data_left((af.samples.at(0)).begin(), (af.samples.at(0)).end());	// load the left channel data for this test
#endif // LOAD_DATA
	const unsigned int N = 256;							// the number of samples per frame
	vector<double> data_processed;						// a vector wrap the output data



	// 测试成品

	Freeverb FB;
	//FB.
	// test the FreeReverb
	double f = 0.84;
	double d = 0.2;
	double N1 = 1157;
	DelayLine delay1(N1);

	auto it = data_left.begin();
	vector<double> dFramePack(N);								// wrap the audio data into Frame
	// cache
	LBCF F1;
	F1.init(0.84, 0.2, 1557);
	AP AP1;
	AP1.init(0.5, 225);
	for (size_t n = 0; n != (data_left.size() / N) * N; n += N) {
		if (it <= data_left.end()) {
			vector<double> dFramePack(it, it + N);
			vector<double> dFrameOut(N);
			vector<vector<double>> LBCF_OUT(1);
			LBCF_OUT.at(0).assign(N, 0);

			// your processing here
			//for (size_t i = 0; i != N; ++i) {
			//	delay1.delay_by_sample(dFramePack[i] + lpf_fb, dFrameOut[i]);
			//	lpf_fb = f * (1 - d) * delay1.getEnd() + d * lpf_cache;
			//	lpf_cache = lpf_fb;
			//}
			F1.run_by_frame(dFramePack, LBCF_OUT.at(0));

		
			AP1.run_by_frame(dFrameOut, dFrameOut);


			for (size_t nn = 0; nn != N; ++nn) {				// output 
				data_processed.push_back(dFrameOut.at(nn));
			}

		}
		it += N;												// update the iterator	
	}


#ifdef JR_PROCESS
														// Initialize and set the Jot reverberator's parameters 
	JotReverb JR;
	JR.init();

	auto it = data_left.begin();
	vector<double> dFramePack(N);							// wrap the audio data into Frame
	for (size_t n = 0; n != (data_left.size()/N) * N; n+=N) {
		if (it <= data_left.end()) {
			vector<double> dFramePack(it, it + N);
			vector<double> dFrameOut(N);
			JR.process(dFramePack, dFrameOut);				// process the frame data
			for (size_t nn = 0; nn != N;++nn) {				// output 
				data_processed.push_back(dFrameOut.at(nn));
			}

		}
		it += N;											// update the iterator	
	}
	cout<< "Processed successful\n";

#endif
#ifdef OUTPUT
	// find max
	double max = 0;
	for (auto mData : data_processed) {
		if (mData > max)
			max = mData;
	}
	// 归一化
	for (auto &mData : data_processed) {
		mData = mData / (max +0.01);
	}

	vector<vector<double>> outputBuffer(2);
	outputBuffer.at(0).assign(data_processed.begin(), data_processed.end());

	outputBuffer.at(1).assign(data_processed.begin(), data_processed.end());
	af.setAudioBuffer(outputBuffer);
	//af.samples.at(0).assign(data_processed.begin(), data_processed.end());
	//af.samples.at(1).assign(data_processed.begin(), data_processed.end());
	af.save("test_lpfb_all3.wav");
#endif
    return 0;
}

void JotReverb::updateAn()
{
	/*
		An = J4 - 2/n *(Un*Un');
	*/
	unsigned int nChannel = 4;
	double sum_temp = 0;
	for (size_t nCount = 0; nCount != nChannel; ++nCount) {
		sum_temp += Un[nCount] * Un[nCount];											// compute the matix Un*Un'
	}
	for (size_t nCloumn = 0; nCloumn != nChannel; ++nCloumn) {
		for (size_t nRow = 0; nRow != nChannel; ++nRow) {
			An[nRow][nCloumn] = J4[nRow][nCloumn] - 2 / (double)nChannel * sum_temp;	// An = J4 - 2/n *(Un*Un');
		}
	}
}

int JotReverb::process(vector<double> data_in, vector<double>& data_out)
{
	/*
		Description	:	Jot reverberator core processing process	
		Version		:	v.1.0	
		Update time	:	2018-11-2
		Parameter 	
			data_in	: a vector wraped the frame data to process
			data_out: a vector wraped the frame processed data
		Note		: the length of data_in/data_out was set as 256 by default .
		Algorithm reference:	
				1. A REVERBERATOR BASED ON ABSORBENT ALL-PASS FILTERS,Luke Dahl, Jean-Marc Jot
				2. IMPROVEMENT OF JOT’S REVERBERATION ALGORITHM,Norbert TOMA1 Marina ŢOPA2 Ioana SĂRĂCUŢ2
	*/

	double sum_an_bx[4] = { 0 };
	double after_delay[4] = { 0 };
	double s[4] = { 0 };

	for (size_t nCount = 0; nCount != nFrame; ++nCount) {
		double sum_of_c_s = 0;
		for (size_t nChannel = 0; nChannel != 4; ++nChannel) {
			sum_an_bx[nChannel] = An_out[nChannel] + gain_c[nChannel] * data_in[nCount]; 
			// delay
			delay_by_samples(sum_an_bx[nChannel], after_delay[nChannel], delay_pos[nChannel], delay_length[nChannel], delay_net[nChannel]);
			// lpf
			s[nChannel] = after_delay[nChannel] * gi[nChannel]*(1 - bi[nChannel]) + bi[nChannel] * lpf_cache[nChannel];
			// out
			sum_of_c_s += gain_c[nChannel] * s[nChannel];									// sum of c*s in the 4 channels
		}

		// compute the matrix s * An
		for (size_t row = 0; row != 4; ++row) {
			double An_out_temp = 0;
			for (size_t j = 0; j != 4; ++j) {
				An_out_temp += An[row][j] * s[j];
			}
			An_out[row] = An_out_temp;

			data_out.at(nCount) = data_in[nCount]* dry_wet_mix_rate + sum_of_c_s;			// mix dry and wet
		}
		
		
	}
	return 0;
}

int JotReverb::delay_by_samples(double in, double &after_delay, unsigned int &cur_delay_pos, unsigned int N,double *delay_line)
{
	// N : delay length
	// the delay_by_samples functin is based on the circular queue
	after_delay = delay_line[cur_delay_pos];		// pop the dealyed data
	delay_line[cur_delay_pos] = in;					// push data into the delay line
	cur_delay_pos = (cur_delay_pos + 1) % N;		// update the pos
	return 0;
}

double JotReverb::updateLpfCoeff()
{
	double alpha = Tr_pi / Tr_0;
	double T = 1 / fs;

	for (size_t i = 0; i != 4; ++i) {
		// mi = delay_length[i]
		gi[i] =pow( 10 , (-3 * delay_length[i] * T  / Tr_0) );			//  lpf coef	
		bi[i] = 1 - 2.0 / (1 + pow(gi[i], (1 - 1 / alpha)) );			//  lpf coef
	}
	return 0.0;
}

int JotReverb::init()
{
	// set the customize Jot's Reverberator parameters here
	// You can modify gain_b,gain_c,dry_wet_mix_rate,J4,Un,and the delay_length per channel

	// update the inner parameter.
	updateAn();			// compute the matrix An
	updateLpfCoeff();	// compute the lpf coeff
	return 0;
}

void DelayLine::delay_by_sample(double data_to_push, double & after_delay)
{
	// delay_line_length : delay length
	// This functin is based on the circular queue
	after_delay = delay_line[cur_pos];					// pop the dealyed data
	delay_line[cur_pos] = data_to_push;								// push data into the delay line
	cur_pos = (cur_pos + 1) % delay_line_length;	// update the pos

}

DelayLine::DelayLine(unsigned int N)
{
	//delay_line_length = N;
	//delay_line = new double[N];

	//for (size_t n = 0; n != N; ++n) {
	//	delay_line[n] = 0;
	//}
	init(N);
}

DelayLine::~DelayLine()
{
	delete[] delay_line;

}

LBCF::LBCF()
{
}

LBCF::~LBCF()
{
}