#include <vector>
// Todo:
// 将延时线换成使用 DelayLine 类
// 重新封装各类参数

class JotReverb {
public:
	double fs = 48000;
	int nFrame = 256;						// the number fo data in per frame
	//double Un[4] = { 0.4,0.2,0.6,0.8 };
	double Un[4] = { 1,1,1,1 };
	//double J4[4][4] = { { 0,0,1,0 },{ 0,1,0,0 },{ 1,0,0,0 },{ 0,0,0,1 } };
	//double J4[4][4] = { { 0,0,0,1 },{ 0,0,1,0 },{ 0,1,0,0 },{ 1,0,0,0 } };
	double J4[4][4] = { { 1,0,0,0 },{ 0,1,0,0 },{ 0,0,1,0 },{ 0,0,0,1 } };
	double gain_b[4] = { 0.25,0.25,0.25,0.25 };
	double gain_c[4] = { 1,1,1,1 };
	double delay_length[4] = { 3089,3187,3323,3407 };
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
	int process(std::vector<double> data_in, std::vector<double>&data_out);
};


void JotReverb::updateAn()
{
	/*
	An = J4 - 2/n *(Un*Un');
	*/
	unsigned int nChannel = 4;
	double sum_temp = 0;
	// compute the matix Un*Un'	,note that Un is a column vector
	// assume the Un has 4 rows
	double temp_matrix[4][4] = { 0 };
	for (size_t nRow = 0; nRow != 4; ++nRow) {
		for (size_t nColumn = 0; nColumn != 4; ++nColumn) {
			temp_matrix[nRow][nColumn] = Un[nColumn] * Un[nColumn];
		}
	}
	for (size_t nRow = 0; nRow != nChannel; ++nRow) {
		for (size_t nColumn = 0; nColumn != nChannel; ++nColumn) {
			An[nRow][nColumn] = J4[nRow][nColumn] - 2 / (double)nChannel * temp_matrix[nRow][nColumn];	// An = J4 - 2/n *(Un*Un');
		}
	}
}

int JotReverb::process(std::vector<double> data_in, std::vector<double>& data_out)
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
			s[nChannel] = after_delay[nChannel] * gi[nChannel] * (1 - bi[nChannel]) + bi[nChannel] * lpf_cache[nChannel];
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

			data_out.at(nCount) = data_in[nCount] * dry_wet_mix_rate + sum_of_c_s;			// mix dry and wet
		}


	}
	return 0;
}

int JotReverb::delay_by_samples(double in, double &after_delay, unsigned int &cur_delay_pos, unsigned int N, double *delay_line)
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
		gi[i] = pow(10, (-3 * delay_length[i] * T / Tr_0));			//  lpf coef	
		bi[i] = 1 - 2.0 / (1 + pow(gi[i], (1 - 1 / alpha)));			//  lpf coef
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