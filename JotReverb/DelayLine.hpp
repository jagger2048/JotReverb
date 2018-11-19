#pragma once
#include <iostream>

class DelayLine
{
	// Delay line based on circular queue
public:
	//DelayLine(unsigned int N);
	DelayLine() {};
	virtual ~DelayLine();

	virtual void delay_by_sample(double data_to_push, double &after_delay);
	virtual double delay_by_sample(double data_to_push);
	double getEnd();
	int init(unsigned int N);
	double getSample(unsigned int N) {
		return delay_line[(cur_pos + N ) % delay_line_length];
	}
private:
	unsigned int cur_pos = 0;
	unsigned int delay_line_length;
	double *delay_line;

};

void DelayLine::delay_by_sample(double data_to_push, double & after_delay)
{
	// delay_line_length : delay length
	// This functin is based on the circular queue
	after_delay = delay_line[cur_pos];					// pop the dealyed data
	delay_line[cur_pos] = data_to_push;					// push data into the delay line
	cur_pos = (cur_pos + 1) % delay_line_length;		// update the pos

}

inline double DelayLine::delay_by_sample(double data_to_push)
{
	double after_delay = delay_line[cur_pos];			// pop the dealyed data
	delay_line[cur_pos] = data_to_push;					// push data into the delay line
	cur_pos = (cur_pos + 1) % delay_line_length;		// update the pos
	return after_delay;
}

inline double DelayLine::getEnd() {
	return delay_line[cur_pos];
}

inline int DelayLine::init(unsigned int N) {
	delay_line_length = N;
	delay_line = new double[N]();	// initialize the delay lint with 0
									//for (size_t n = 0; n != N; ++n) {
									//	delay_line[n] = 0;
									//}
	return 0;
}

//DelayLine::DelayLine(unsigned int N)
//{
//	init(N);
//}

DelayLine::~DelayLine()
{
	delete[] delay_line;

}