// fender's reverb based on FDN algorithm
// in-> to mono -> predelay-> prelpf -> FDN -> steteo output
#pragma once
#include <vector>
#include "myJotReverb.h"
//  biquad
class Biquad {
public:
	double coeffs[2][3] = { 0 };
	double state[3] = { 0 };

	int setCoeffs(double *b, double *a);
	int setCoeffs(double **_ba) {
		for (size_t i = 0; i < 2; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				coeffs[i][j] = _ba[i][j];
			}
		}
	}
	int reset();
	double filter(double input);
	//double filter(vector<double> input, vector<double> &output);
	Biquad() {};
	~Biquad() {};
};

int Biquad::setCoeffs(double * b, double * a)
{
	for (int n = 0; n != 3; ++n) {
		coeffs[0][n] = *(b + n);
		coeffs[1][n] = *(a + n);
	}
	return 0;
}

int Biquad::reset()
{
	state[3] = { 0 };
	return 0;
}

double Biquad::filter(double input)
{
	// 单点滤波
	state[2] = state[1];
	state[1] = state[0];
	state[0] = input + (-coeffs[1][1]) * state[1] + (-coeffs[1][2]) * state[2];
	// compute the output
	double output = coeffs[0][0] * state[0] + coeffs[0][1] * state[1] + coeffs[0][2] * state[2];
	return output;
}


class EarlyReverb
{
	// input- > lpf -> delay ->output
	// 5 selectd_delay channels
public:
	EarlyReverb();
	~EarlyReverb();
	//double *lpf_coef;
	unsigned int total_length= 2000;
	double *delay_length;
	double after_lpf_delay = 0;
	double predelay_dw = 0.4;		// predelay dry wet rate

	DelayLine *ER_delay_line;
	Biquad lpf;
	void setLpf(double *_lpf_b, double *_lpf_a) {
		//lpf_coef
		lpf.setCoeffs(_lpf_b, _lpf_a);
	}
	void setDelay(unsigned int _total_length,double *_delay_length) {
		total_length = _total_length;
		ER_delay_line = new DelayLine;
		ER_delay_line->init(total_length);

		delay_length = new double[5];
		for (size_t i = 0; i < 5; i++)
		{
			// 5 channels
			delay_length[i] = _delay_length[i];
		}
	}

	void ER_init(double *_lpf_b,double *_lpf_a,unsigned int _total_length,double *_delay_length) {
		setLpf(_lpf_b,_lpf_a);
		setDelay(_total_length,_delay_length);
	}
	double getEarlyDelay() {
		return after_lpf_delay;
	}
	double run_by_sample(double data, double &_after_lpf_delay,double &_select_delay) {

		// pre dry wet rate is set to 0.5
		after_lpf_delay = ER_delay_line->delay_by_sample( data * predelay_dw + lpf.filter(data) );

		// select the delay sample
		double output_temp = 0;
		for (size_t i = 0; i < 5; i++)
		{
			output_temp += ER_delay_line->getSample(delay_length[i]);
		}
		// output_temp should pass a allpass filter
		// To be implement ......

		_after_lpf_delay = after_lpf_delay;
		_select_delay = output_temp;

		return data;
	}

private:

};

EarlyReverb::EarlyReverb()
{
}

EarlyReverb::~EarlyReverb()
{
}

class FenderRev :public FDN
{
public:
	EarlyReverb ER;
	double *fr_lpf_b;
	double *fr_lpf_a;
	double fr_dry_wet_rate = 0.5;

	double * lpf_cache;
	double *after_lpf;
	double CH[2][8] = { {1,1,-1,-1,1,1,-1,-1 },{1,-1,1,-1,1,-1,1,-1 } };

	double DCB_pre_in[2] = {};
	double DCB_out[2]{};

	int FR_init(double fr) {
		ER.ER_init(new double[3]{ 0.4566722944641210, 0, 0 },		// lpf'b
			new double[3]{ 1, -0.444124594801, 0 },					// lpf'a
			2400,													// total delay length
			new double[5]{ 1901,2011,2113,2203,2333 }				// select in 5 channels
		);
		nChannel = 8;
		fs = fr;
		init_fdn(
			8,												// number of channels
			//new double[8]{ 1,1,1,1,1,1,1,1 },				// b
			new double[8]{ 0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5 },// b
			//new double[8]{ 1, 1, 1, 1,1,1,1,1 },			// c
			new double[8]{
				0.9385798540696482, 0.9569348654119887, 0.9661747902651412, 0.9604810809978065,
				0.9025044632215804, 0.8968737228960622, 0.9298475704611008, 0.9106079155648642
			},			// c
			//new double[8]{ 1, 1, 1, 1,1, 1, 1, 1 },			// g
			new double[8]{ 0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125 },			// g
			new unsigned int[8]{ 2011,2113,2203,2333,3089, 3187, 3323, 3407 }	// delay line length

		);

		// set fdn's an

		An = new double* [nChannel] {};
		for (size_t n = 0; n < nChannel; n++)
		{
			An[n] = new double[nChannel] {};
		}
		for (size_t i = 0; i < nChannel; i++)
		{
			for (size_t j = 0; j < nChannel; j++)
			{
				An[i][j] = -0.25;
				if (j == nChannel - i-1) An[i][j] = 0.75;
			}
		}

		// Y(n)=Bp*Y(n-1)+Bp_x*X(n)，其中 X(n)为延迟之后的输出
		fr_lpf_b = new double[nChannel] { // bp
			0.09416941618611564, 0.1167825571171863, 0.1312036766710140, 0.1258676773153445,
				0.06932681600569191, 0.06648417682286377, 0.09667525158390457, 0.07923379711870723
		};
		fr_lpf_a = new double[nChannel] { // bpx
			0.9058305838138844, 0.8832174428828137, 0.8687963233289859, 0.8741323226846556,
				0.9306731839943081, 0.9335158231771362, 0.9033247484160955, 0.9207662028812927
		};

		lpf_cache = new double[nChannel] {};
		after_lpf = new double[nChannel] {};
		return 0;
	};
	double run_by_sample(double data_in) {

		double after_er_delay = 0, select_delay = 0;
		double original = ER.run_by_sample(data_in, after_er_delay, select_delay);
		for (size_t n = 0; n < nChannel; n++)
		{
			delay_line[n].delay_by_sample(Bn[n] * after_er_delay + Gn[n] * sum_of_an[n], after_delay[n]);
			// Y_n  = b * Y_(n-1) + a * X_n
			after_lpf[n] = fr_lpf_b[n] * after_lpf[n] + fr_lpf_a[n] * after_delay[n];
		}
		//	update sum_of_an
		for (size_t nRow = 0; nRow < nChannel; nRow++)
		{
			double sum_temp = 0;
			for (size_t nCloumn = 0; nCloumn < nChannel; nCloumn++)
			{
				sum_temp += after_lpf[nCloumn] * An[nRow][nCloumn];
			}
			sum_of_an[nRow] = sum_temp;
		}
		//	output
		double output_temp = 0;
		double left = 0, right = 0;
		for (size_t n = 0; n < nChannel; n++)
		{
			left += after_lpf[n] * CH[0][n];
			right += after_lpf[n] * CH[1][n];
			output_temp += after_lpf[n] * Cn[n];
		}
		// after_lpf-> c -> matrix -> stereo output-> DCB -> output 
		// DCB output
		// DCB_out(n) = DCB_in(n) - DCB_in(n - 1) + 0.99* DCB_out(n - 1)
		DCB_out[0] = left - DCB_pre_in[0] + 0.99* DCB_out[0];
		DCB_pre_in[0] = left;
		DCB_out[1] = right - DCB_pre_in[1] + 0.99* DCB_out[1];
		DCB_pre_in[1] = right;

		// output left channel 
		return DCB_out[0] + 0.5*after_er_delay;
		//return DCB_out[1] + 0.5*after_er_delay;

	}
	void run_by_frame(std::vector<double> data_in, std::vector<double>& data_out) {
		for (size_t i = 0; i < data_in.size(); i++)
		{
			data_out.at(i) = run_by_sample(data_in.at(i));
		}
	}
	FenderRev();
	virtual ~FenderRev();

private:

};

FenderRev::FenderRev()
{
}

FenderRev::~FenderRev()
{
	delete [] fr_lpf_b;
	delete[] fr_lpf_a;
}


// early reverb usage.
//EarlyReverb ER;
//ER.init(new double[3]{0.014401440346511,0.028802880693022,0.014401440346511},	// lpf'b
//		new double[3]{1,-1.632993161855452,0.690598923241497},					// lpf'a
//		2400,																	// total delay length
//		new double[5]{1901,2011,2113,2203,2333}									// 5 channels		
//		);	
////ER.ER_init(new double[3]{0.4566722944641210 * 1.2171,0,0},		// lpf'b
////		new double[3]{1,-0.444124594801,0},					// lpf'a
////		2400,																	// total delay length
////		new double[5]{1901,2011,2113,2203,2333}									// 5 channels		
////		);
