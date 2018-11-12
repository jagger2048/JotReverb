// FDN, feedback delay network unit
#include <vector>
//#include "DelayLine.hpp"
class FDN
{
public:
	std::vector<double> Bn;		// pre gain,b
	std::vector<double> Cn;
	std::vector<double> Gn;
	std::vector<double> Un;
	double **Jn;

	double **An;// feedback matrix
	double nChannel = 4;
	void setAn() {
		/*
		An = J4 - 2/n *(Un*Un');
		*/
		// initialize Jn
		Jn = new double *[nChannel];
		for (size_t n = 0; n != nChannel; ++n) {
			Jn[n] = new double[nChannel];
			Jn[n][n] = 1;
		}
		// initialize Un
		std::vector<double> tmp(nChannel, 1);
		Un.assign(tmp.begin(), tmp.end());


		// initialize An
		An = new double* [nChannel];
		for (size_t n = 0; n < nChannel; n++)
		{
			An[n] = new double[nChannel] {0};
		}
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
				An[nRow][nColumn] = Jn[nRow][nColumn] - 2 / (double)nChannel * temp_matrix[nRow][nColumn];	// An = J4 - 2/n *(Un*Un');
			}
		}
	}
	int init(unsigned int num_of_channels, std::vector<double> _Bn, std::vector<double> _Cn, std::vector<double> _Gn) {
		nChannel = num_of_channels;
		Bn.assign(_Bn.begin(), _Bn.end());
		Cn.assign(_Cn.begin(), _Cn.end());
		Gn.assign(_Gn.begin(), _Gn.end());

		setAn();
		return 0;
	};
	FDN();
	~FDN();
	//updateAn();
private:

};

FDN::FDN()
{
}

FDN::~FDN()
{
	delete[] Jn;
	delete[] An;
}
//FDN::updateAn()
//{
//	/*
//	An = J4 - 2/n *(Un*Un');
//	*/
//	double sum_temp = 0;
//	// compute the matix Un*Un'	,note that Un is a column vector
//	// assume the Un has 4 rows
//	double temp_matrix[nChannel][nChannel] = { 0 };
//	for (size_t nRow = 0; nRow != nChannel; ++nRow) {
//		for (size_t nColumn = 0; nColumn != nChannel; ++nColumn) {
//			temp_matrix[nRow][nColumn] = Un[nColumn] * Un[nColumn];
//		}
//	}
//	for (size_t nRow = 0; nRow != nChannel; ++nRow) {
//		for (size_t nColumn = 0; nColumn != nChannel; ++nColumn) {
//			An[nRow][nColumn] = Jn[nRow][nColumn] - 2 / (double)nChannel * temp_matrix[nRow][nColumn];	// An = Jn - 2/n *(Un*Un');
//		}
//	}
//}