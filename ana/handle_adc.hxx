#ifndef _handle_adc_HXX
#define _handle_adc_HXX
 
#include <iostream>
#include <fstream>
#include <TH1F.h>

class handle_adc
{
	public:
		const int module_No;
		const int asic_No;
		const int strip_Ch;

		TH1D* phist1_P[10];
		TH1D* phist1_N[10];
		TH1D* ahist1_P[10][8][128]; //[module_No][asic_No][strip_Ch]
		TH1D* ahist1_N[10][8][128];
		TH1D* chist1_P[10][8][128];
		TH1D* chist1_N[10][8][128];

		void csv_to_arrange(const char* file_csv);
};
#endif
