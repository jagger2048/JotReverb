// FDN, feedback delay network unit
#pragma once

#include <vector>
#include "DelayLine.hpp"
class FDN
{
public:
	double *Bn;				// pre gain,b
	double *Cn;
	double *Gn;
	double *Un;
	double **Jn;
	unsigned int* delay_length;
	double **An;			// feedback matrix
	double fs = 48000;
	size_t nChannel = 4;
	DelayLine *delay_line;
	double* sum_of_an;
	double* after_delay;

	////
	void setJn();
	void setUn();
	void setAn();
	void setBn(double *_Bn);
	void setCn(double *_Cn);
	void setGn(double *_Gn);
	void setDelayLine(unsigned int *_delay_length);
	int init_fdn(unsigned int num_of_channels, double *_Bn, double *_Cn, double *_Gn, unsigned int* _delay_length);
	virtual double run_by_sample(double data_in);
	void run_by_frame(std::vector<double> data_in, std::vector<double> &data_out);

	FDN();
	virtual ~FDN();
private:
};


