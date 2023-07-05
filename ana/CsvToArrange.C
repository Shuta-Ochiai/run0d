void CsvToArrange(const char* csvfile){
	//string csvfile = "./pscan_XA000-08-001-064-059-024-15_addr7_ADCtoCharge.csv";
	ifstream ifs(csvfile);
	if (!ifs.is_open()){
		cerr << "Could not open the file - " 
		     << csvfile << endl;
		exit(1);
	}
	
	string delim = ", ";
	double Adc_to_Charge[128][32];
	int Strip_ch = 0;
	int Adc_ch = -1;
	size_t pos = 0;
	string line;
	getline(ifs, line); //skip first row
	while (getline(ifs, line)){
		while((pos=line.find(delim)) != string::npos){
			if (Adc_ch>=0){
				Adc_to_Charge[Strip_ch][Adc_ch] = stod(line.substr(0, pos));
			}
			line.erase(0, pos+delim.length()); //skip first collmun
			Adc_ch += 1;
		}
		Adc_ch = -1;
		Strip_ch += 1;
	}
	for (int s_ch=0; s_ch<128; s_ch++){
		for(int a_ch=0; a_ch<32; a_ch++){
			cout << "Adc_to_Charge[" << s_ch << "][" << a_ch << "] = " << Adc_to_Charge[s_ch][a_ch] << endl;
		}
	}
	
}
