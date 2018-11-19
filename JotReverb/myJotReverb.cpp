#include "stdafx.h"
#include "FDN.h"
#include "myJotReverb.h"

inline int mJotReverb::updateLpfCoeff() {
	bi = new double[nChannel];
	gi = new double[nChannel];
	double alpha = Tr_pi / Tr_0;
	double T = 1 / fs;

	for (size_t i = 0; i != 4; ++i) {
		// mi = delay_length[i]
		gi[i] = pow(10, (-3 * delay_length[i] * T / Tr_0));			//  lpf coef	
		bi[i] = 1 - 2.0 / (1 + pow(gi[i], (1 - 1 / alpha)));		//  lpf coef
	}
	lpf_cache = new double[nChannel] {0};
	return 0;
}

inline int mJotReverb::default_init() {
	init_fdn(
		4,												// number of channels
		new double[4]{ 0.25, 0.25, 0.25, 0.25 },		// b
		new double[4]{ 1, 1, 1, 1 },					// c
		new double[4]{ 1, 1, 1, 1 },					// g
		new unsigned int[4]{ 3089, 3187, 3323, 3407 }	// delay line length
	);
	after_lpf = new double[nChannel];
	updateLpfCoeff();
	return 0;
}

inline double mJotReverb::run_by_sample(double data_in) {
	double *after_lpf = new double[nChannel];
	for (size_t n = 0; n < nChannel; n++)
	{
		delay_line[n].delay_by_sample(Bn[n] * data_in + Gn[n] * sum_of_an[n], after_delay[n]);
		after_lpf[nChannel] = after_delay[n] * gi[n] * (1 - bi[n]) + bi[n] * lpf_cache[n];

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
		output_temp += after_delay[n] * Cn[n];
	}
	return output_temp + 0.5*data_in;

}

inline mJotReverb::mJotReverb() {}

inline mJotReverb::~mJotReverb() {
	delete[] after_lpf;
	delete[] lpf_cache;
	delete[] bi;
	delete[] gi;
}