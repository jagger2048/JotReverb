// fender's reverb based on FDN algorithm
// in-> to mono -> predelay-> prelpf -> FDN -> steteo output
#include "DelayLine.hpp"
class FenderReverb:public FDN
{
public:
	FenderReverb:public FDN();
	~FenderReverb:public FDN();

private:

};

FenderReverb:public FDN::FenderReverb:public FDN()
{
}

FenderReverb:public FDN::~FenderReverb:public FDN()
{
}

//  biquad
class Biquad {
public:
	double coeffs[2][3] = { 0 };
	double state[3] = { 0 };

	int setCoeffs(double *b, double *a);
	int reset();
	double filter(double input);
	//double filter(vector<double> input, vector<double> &output);
	Biquad();
	~Biquad();
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
	// µ¥µãÂË²¨
	state[2] = state[1];
	state[1] = state[0];
	state[0] = input + (-coeffs[1][1]) * state[1] + (-coeffs[1][2]) * state[2];
	// compute the output
	double output = coeffs[0][0] * state[0] + coeffs[0][1] * state[1] + coeffs[0][2] * state[2];
	return output;
}

//double Biquad::filter(vector<double> input, vector<double>& output)
//{
//	for (int n = 0; n != input.size(); ++n) {
//		Biquad::filter(input.at(n), output.at(n));
//	}
//	return 0.0;
//}

class EarlyReverb
{
	// input- > lpf -> delay ->output
	// 5 selectd_delay channels
public:
	EarlyReverb();
	~EarlyReverb();
	//double *lpf_coef;
	double *delay_length;
	double after_lpf_delay = 0;
	DelayLine ER_delay_line;
	Biquad lpf;
	void setLpf(double *_lpf_b, double *_lpf_a) {
		//lpf_coef
		lpf.setCoeffs(_lpf_b, _lpf_a);
	}
	void setDelay(double *_delay_length) {
		delay_length = new double[5];
		for (size_t i = 0; i < 5; i++)
		{
			delay_length[i] = _delay_length[i];
		}
	}

	void init(double *_lpf,double *_delay_length) {
		setLpf();
		setDelay(_delay_length);
	}
	double getEarlyDelay() {
		return after_lpf_delay;
	}
	double run_by_sample(double data) {
		after_lpf_delay = ER_delay_line.delay_by_sample( lpf.filter(data) );
		// select the delay sample
		double output_temp = 0;
		for (size_t i = 0; i < 5; i++)
		{
			output_temp += ER_delay_line.getSample(delay_length[i]);
		}
		return output_temp / 5.0;
	}
private:

};

EarlyReverb::EarlyReverb()
{
}

EarlyReverb::~EarlyReverb()
{
}
