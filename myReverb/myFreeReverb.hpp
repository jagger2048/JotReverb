#include "DelayLine.hpp"
#include <iostream>
#include <vector>
//using namespace std;
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
	DelayLine DELAY;
	double run_by_sample(double data_in, double &data_out);;
	double run_by_sample(double data_in);;


	double run_by_frame(std::vector<double> data_in, std::vector<double> &data_out);
	int init(double f, double d, unsigned int delay_len);;

	LBCF();
	~LBCF();

private:

};
class AP
{
	// delay_line based allpass filter
	// ref:https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
public:
	DelayLine delay_line;
	double g = 0.5;
	unsigned int N = 1;
	double init(double gain, unsigned int delay_line_length);;
	double run_by_sample(double data_in, double &data_out);;
	double run_by_sample(double data_in);;
	double run_by_frame(std::vector<double> data_in, std::vector<double> &data_out);;
	AP();
	~AP();

private:

};
class myFreeReverb
{
public:

	int init();
	double run_by_sample(double data_in);
	void run_by_frame(std::vector<double> data_in, std::vector<double> &data_out);;
	void setDamp(double damp_val) {
		for (size_t n = 0; n != 8; ++n) {
			damp[n] = damp_val;
		}
	}
	void setFeedback(double feedback_val) {
		for (size_t n = 0; n != 8; ++n) {
			feedback[n] = feedback_val;
		}
	}
	void setApplassGain(double ap_gain) {
		allpass_gain = ap_gain;
	}
	myFreeReverb();
	~myFreeReverb();

private:
	// d = damp = initial_damp * scale_damp = 0.5 *0.4=0.2
	double damp[8] = { 0 };
	// f = roomsize= initial_room * sacle_room + offset_room = 0.5*0.28 + 0.7 ; f <0.98
	double feedback[8] = { 0 };
	double allpass_gain = 0.5;
	double comb_sum_gain = 0.7;//0.7

	unsigned int ap_coef[4] = { 225,556,441,341 };
	//unsigned int ap_coef[4] = { 325,656,541,441 };
	//unsigned int comb_coef[8] = { 1557,1617,1491,1422,1277,1356,1188,1116 };
	//unsigned int comb_coef[8] = { 1957,2017,1891,1822,1677,1756,1588,1516 };
	unsigned int comb_coef[8] = { 2957,3017,2891,2822,2677,2756,3588,3516 };
	LBCF parallel_comb[8];
	AP cascaded_ap[4];

};

inline int myFreeReverb::init()
{
	// initialize the low_feedback comb filter and allpass filter
	setDamp(0.88); // 0.84
	setFeedback(0.2);// 0.2
	setApplassGain(0.6);

	for (size_t nAp = 0; nAp != 4; nAp++) {
		cascaded_ap[nAp].init(allpass_gain, ap_coef[nAp]);
	}
	for (size_t nComb = 0; nComb != 8; nComb++) {
		//parallel_comb[nComb].init(0.84, 0.2, comb_coef[nComb]);
		parallel_comb[nComb].init( damp[nComb],feedback[nComb], comb_coef[nComb] );
	}

	return 0;
}

inline double myFreeReverb::run_by_sample(double data_in) {
	double sum_comb = 0;
	for (size_t nComb = 0; nComb != 8; nComb++) {
		sum_comb += parallel_comb[nComb].run_by_sample(data_in);
	}
	double sum_ap = sum_comb * comb_sum_gain;
	for (size_t nAp = 0; nAp != 4; nAp++) {
		sum_ap = cascaded_ap[nAp].run_by_sample(sum_ap);
	}
	return sum_ap;
}

inline void myFreeReverb::run_by_frame(std::vector<double> data_in, std::vector<double>& data_out) {

	if (data_in.size() != data_out.size()) {
		std::cout << "Invail vector length\n";
		//return -1;
	}
	for (size_t n = 0; n != data_in.size(); ++n) {
		data_out.at(n) = run_by_sample(data_in.at(n));
	}
}

myFreeReverb::myFreeReverb()
{
}

myFreeReverb::~myFreeReverb()
{
}


inline double LBCF::run_by_sample(double data_in, double & data_out) {
	DELAY.delay_by_sample(data_in + lpf_fb, data_out);					// update the delay line
	lpf_fb = lpf_f * (1 - lpf_d) * DELAY.getEnd() + lpf_d * lpf_cache;
	lpf_cache = lpf_fb;													// update the lpf cache
	return 0;
}

inline double LBCF::run_by_sample(double data_in) {
	double data_out = 0;
	DELAY.delay_by_sample(data_in + lpf_fb, data_out);					// update the delay line
	lpf_fb = lpf_f * (1 - lpf_d) * DELAY.getEnd() + lpf_d * lpf_cache;
	lpf_cache = lpf_fb;													// update the lpf cache
	return data_out;
}

inline double LBCF::run_by_frame(std::vector<double> data_in, std::vector<double>& data_out) {
	for (size_t n = 0; n != data_in.size(); ++n) {
		run_by_sample(data_in.at(n), data_out.at(n));
	}
	return 0;
}

inline int LBCF::init(double f, double d, unsigned int delay_len) {
	DELAY.init(delay_len);
	lpf_f = f;
	lpf_d = d;
	N = delay_len;
	return 0;
}

LBCF::LBCF()
{
}

LBCF::~LBCF()
{
}
inline double AP::init(double gain, unsigned int delay_line_length) {
	g = gain;
	N = delay_line_length;
	delay_line.init(N);
	return 0;
}
inline double AP::run_by_sample(double data_in, double & data_out) {
	data_out = g * data_in + delay_line.getEnd();
	delay_line.delay_by_sample(data_in + -g * data_out);// push data into the delay line
	return 0;
}
inline double AP::run_by_sample(double data_in) {
	double data_out = 0;
	data_out = g * data_in + delay_line.getEnd();
	delay_line.delay_by_sample(data_in + -g * data_out);// push data into the delay line
	return data_out;
}
inline double AP::run_by_frame(std::vector<double> data_in, std::vector<double>& data_out) {
	for (size_t n = 0; n != data_in.size(); ++n) {
		run_by_sample(data_in.at(n), data_out.at(n));
	}
	return 0;
}
AP::AP()
{
}

AP::~AP()
{
}