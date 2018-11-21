#include "stdafx.h"
#include "FDN.h"
#include "myJotReverb.h"

inline int mJotReverb::updateLpfCoeff() {
	bi = new double[nChannel] {};
	gi = new double[nChannel] {};
	double alpha = Tr_pi / Tr_0;
	double T = 1 / fs;
	for (size_t i = 0; i != nChannel; ++i) {
		// mi = delay_length[i]
		gi[i] = pow(10, (-3.0 * delay_length[i] * T / Tr_0));			//  lpf coef
		bi[i] = 1 - 2.0 / (1.0 + pow(gi[i], (1.0 - 1.0 / alpha)));		//  lpf coef
	}
	lpf_cache = new double[nChannel] {0};
	return 0;
}

inline int mJotReverb::default_init() {

	nChannel = 4;
	fs = 44100;
	init_fdn(
		4,												// number of channels
		new double[4]{ 1,1,1,1},		// b
		//new double[4]{ 0.5,0.5,0.5,0.5},		// b
		//new double[8]{ 1,1,1,1,1,1,1,1 },		// b
		new double[4]{ 0.9, 0.9, 0.9, 0.9},					// c
		//new double[8]{ 1, 1, 1, 1,1,1,1,1 },					// c
		new double[4]{ 0.9, 0.9, 0.9, 0.9 },		// g
		//new double[8]{ 1, 1, 1, 1,1, 1, 1, 1 },		// g
		new unsigned int[4]{ 3089, 3187, 3323, 3407 }	// delay line length
		//new unsigned int[4]{ 2011,2113,2203,2333 }	// delay line length
		//new unsigned int[8]{ 2011,2113,2203,2333,3089, 3187, 3323, 3407 }	// delay line length
	);
	for (size_t i = 0; i < 4; i++)
	{
		Cascaded_AP[i] = new AP;
	}
	Cascaded_AP[0]->init(0.5, 225);
	Cascaded_AP[1]->init(0.5, 556);
	Cascaded_AP[2]->init(0.5, 441);
	Cascaded_AP[3]->init(0.5, 341);
	after_lpf = new double[nChannel];
	updateLpfCoeff();
	return 0;
}

inline double mJotReverb::run_by_sample(double data_in) {
	for (size_t n = 0; n < nChannel; n++)
	{
		delay_line[n].delay_by_sample(Bn[n] * data_in + Gn[n] * sum_of_an[n], after_delay[n]);
		after_lpf[n] = after_delay[n] * gi[n] * (1.0 - bi[n]) + bi[n] * lpf_cache[n];
		lpf_cache[n] = after_lpf[n]  * 0.99;
	}
	//	update sum_of_an
	for (size_t nRow = 0; nRow < nChannel; nRow++)
	{
		double sum_temp = 0;
		for (size_t nCloumn = 0; nCloumn < nChannel; nCloumn++)
		{
			//sum_temp = sum_temp +after_delay[nCloumn] * An[nRow][nCloumn];
			sum_temp += after_lpf[nCloumn] * An[nRow][nCloumn];
		}
		sum_of_an[nRow] = sum_temp;
	}
	//	output
	double output_temp = 0;
	for (size_t n = 0; n < nChannel; n++)
	{
		output_temp += after_lpf[n] * Cn[n];
	}
	// add a cascaded allpass filter
	//for (size_t i = 0; i < 4; i++)
	//{
		output_temp = Cascaded_AP[1]->run_by_sample(output_temp);
	//}

	return output_temp + 0.5 * data_in;		// feedforward factor is 0.5 
}

void mJotReverb::run_by_frame(std::vector<double> data_in, std::vector<double> &data_out)
{
	for (size_t n = 0; n < data_in.size(); n++)
	{
		data_out.at(n) = run_by_sample(data_in.at(n));
	}
}

inline mJotReverb::mJotReverb() {}

inline mJotReverb::~mJotReverb() {
	delete[] after_lpf;
	delete[] lpf_cache;
	delete[] bi;
	delete[] gi;
}
