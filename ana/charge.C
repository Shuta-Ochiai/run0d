#include "nlohmann/json.hpp"

using json = nlohmann::json;

double csv_to_arrange(const char* inputfile_csv, int s_i, int a_i){
	ifstream ifs(inputfile_csv);
	if (!ifs.is_open()){
		cerr << "Could not open the file : "
		     << inputfile_csv << endl;
		exit(1);
	}
	
	string delim = ", ";
	double adc_to_charge[128][32];
	int strip_i = 0;
	int adc_i = -1;
	size_t pos = 0;
	string line;
	getline(ifs, line); //skip first row
	while (getline(ifs, line)){
		while((pos=line.find(delim)) != string::npos){
			if(adc_i>=0){
				adc_to_charge[strip_i][adc_i] = stod(line.substr(0, pos));
			}
			line.erase(0, pos+delim.length());
			adc_i += 1;
		}
		adc_i = -1;
		strip_i += 1;
	}
	
	return adc_to_charge[s_i][a_i];
}

void charge(const char* inputfile_root, const char* outputfile_root){
	//100番台のみ考える
	TCanvas* canvas1[10];
	for (int i=1; i<10; i++){
		canvas1[i] = new TCanvas(Form("canvas%d", i), Form("canvas%d", i), 800, 600);
	}
	FILE* fp = fopen("./ModuleMap.json", "r");
	json j = json::parse(fp);
	fclose(fp);
	TFile* file = new TFile(inputfile_root);
	TTree* tree = (TTree*)file->Get("tree");
	unsigned long long int systemTimestamp, endTimestamp;
	vector<unsigned int> *packetNumber=0;
	vector<short> *gbt=0, *elink=0, *ch=0, *adc=0, *tdcL10b=0, *eventMissed=0, *tsMsb=0, *ack=0, *status=0;
		
	tree->SetBranchAddress("systemTimestamp",&systemTimestamp);
	tree->SetBranchAddress("endTimestamp",&endTimestamp);
	tree->SetBranchAddress("packetNumber",&packetNumber);
	tree->SetBranchAddress("gbt", &gbt);
	tree->SetBranchAddress("elink", &elink);
	tree->SetBranchAddress("ch", &ch);
	tree->SetBranchAddress("adc", &adc);
	tree->SetBranchAddress("tdcL10b", &tdcL10b);
	tree->SetBranchAddress("eventMissed", &eventMissed);
	tree->SetBranchAddress("tsMsb", &tsMsb);
	tree->SetBranchAddress("ack", &ack);
	tree->SetBranchAddress("status", &status);

	const int module_No = 10;
	const int asic_No = 8;
	const int strip_ch = 128;
	const int adc_ch = 32;

	// profile
	TH1D* phist1_P[module_No];
	TH1D* phist1_N[module_No];
	// adc
	TH1D* ahist1_P[module_No][asic_No][strip_ch];
	TH1D* ahist1_N[module_No][asic_No][strip_ch];
	// charge
	TH1D* chist1_P[module_No][asic_No][strip_ch];
	TH1D* chist1_N[module_No][asic_No][strip_ch];

	double adc_to_charge_P[module_No][asic_No][strip_ch][adc_ch];
	double adc_to_charge_N[module_No][asic_No][strip_ch][adc_ch];
	
	
	for (int m_i=1; m_i<module_No; m_i++){
		if (m_i==5){continue;}
		//profile hist ************************************************
		phist1_P[m_i] = new TH1D(Form("profile 10%d_P",m_i), Form(" profile 10%d P-side", m_i), 1024, 0, 1023);
		phist1_P[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_P[m_i]->GetYaxis()->SetTitle("Counts");
		phist1_P[m_i]->GetYaxis()->SetRangeUser(0,1e5);
		
		phist1_N[m_i] = new TH1D(Form("profile 10%d_N",m_i), Form("profile 10%d N-side", m_i), 1024, 0, 1023);
		phist1_N[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_N[m_i]->GetYaxis()->SetTitle("Counts");
		phist1_N[m_i]->GetYaxis()->SetRangeUser(0,1e5);
		
		for (int asic_i=0; asic_i<asic_No; asic_i++){
			for (int s_i=0; s_i<strip_ch; s_i++){
				//adc hist **********************************************
				ahist1_P[m_i][asic_i][s_i] = new TH1D(Form("adc 10%d_P ASIC#%d Ch%d",m_i, asic_i, s_i), Form("adc 10%d P-side ASIC#%d Ch%d", m_i, asic_i, s_i), 32, 0, 31);
				ahist1_P[m_i][asic_i][s_i]->GetXaxis()->SetTitle("ADC");
				ahist1_P[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//ahist1_P[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
			
				ahist1_N[m_i][asic_i][s_i] = new TH1D(Form("adc 10%d_N ASIC#%d Ch#%d",m_i, asic_i, s_i), Form("adc 10%d N-side ASIC#%d Ch#%d", m_i, asic_i, s_i), 32, 0, 31);
				ahist1_N[m_i][asic_i][s_i]->GetXaxis()->SetTitle("ADC");
				ahist1_N[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//ahist1_N[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
		
				//charge hist *********************************************
				chist1_P[m_i][asic_i][s_i] = new TH1D(Form("charge 10%d_P ASIC#%d Ch%d",m_i, asic_i, s_i), Form("charge 10%d P-side ASIC#%d Ch%d", m_i, asic_i, s_i), 15, 0, 14);
				chist1_P[m_i][asic_i][s_i]->GetXaxis()->SetTitle("Charge");
				chist1_P[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//chist1_P[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
	
				chist1_N[m_i][asic_i][s_i] = new TH1D(Form("charge 10%d_N ASIC#%d Ch%d",m_i, asic_i, s_i), Form("charge 10%d N-side ASIC#%d Ch%d", m_i, asic_i, s_i), 15, 0 ,14);
				chist1_N[m_i][asic_i][s_i]->GetXaxis()->SetTitle("Charge");
				chist1_N[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//chist1_N[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
				
				for (int adc_i=0; adc_i<adc_ch; adc_i++){
					//adc_to_charge_P[m_i][asic_i][s_i][adc_i] = csv_to_arrange(j[Form("10%d_P_ASIC_No%d", m_i, asic_i)]["adc_to_charge"], s_i, adc_i); 
					if (m_i==6){
						string inputfile_csv = j[Form("10%d_N_ASIC_No%d", m_i, asic_i)]["adc_to_charge"];
						const char* file_csv = inputfile_csv.c_str();
						adc_to_charge_N[m_i][asic_i][s_i][adc_i] = csv_to_arrange(file_csv, s_i, adc_i); 
					}
				}
		}
	}
	}

	int elink_to_asicNo_PB[16] = {6, 4, 2, 0, 7, 5, 3, 1, 0, 2, 4, 6, 1, 3, 5, 7};
	int elink_to_asicNo_PA[16] = {0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 7, 5, 3, 1};
	
	int Entry = tree->GetEntries();
	for(int ent_i = 0; ent_i<Entry; ent_i++){
		tree->GetEntry(ent_i);
		for(int evt_i=0; evt_i<ch->size(); evt_i++){
			if(gbt->at(evt_i)==1){//port1 102(PA) ************************************
				continue;	
				if(adc->at(evt_i)>1 && 0<=ch->at(evt_i) && ch->at(evt_i) <128 && elink->at(evt_i)<8){ 
					phist1_N[2]->Fill(ch->at(evt_i)+128*elink_to_asicNo_PA[elink->at(evt_i)]);
				
					for(int asic_i=0; asic_i<asic_No; asic_i++){
						if(asic_i==elink_to_asicNo_PA[elink->at(evt_i)]){
							for(int s_i=0; s_i<strip_ch; s_i++){
								if(s_i==ch->at(evt_i)){
									//ahist1_N[2][asic_i][s_i]->Fill(adc->at(evt_i));
									//chist1_N[2][asic_i][s_i]->Fill(adc_to_charge[2][asic_i][s_i][adc->at(evt_i)]);
								}
							}
						}		
					}
				}
			}//102 *******************************************************************
		
			else if(gbt->at(evt_i)==2){//port2 106(PB) *******************************
				if(adc->at(evt_i)>1 && 0<=ch->at(evt_i) && ch->at(evt_i) <128 && elink->at(evt_i)<8){ 
					phist1_N[6]->Fill(ch->at(evt_i)+128*elink_to_asicNo_PB[elink->at(evt_i)]);
					
					for(int asic_i=0; asic_i<asic_No; asic_i++){
						if(asic_i==elink_to_asicNo_PB[elink->at(evt_i)]){
							for(int s_i=0; s_i<strip_ch; s_i++){
								if(s_i==ch->at(evt_i)){
									ahist1_N[6][asic_i][s_i]->Fill(adc->at(evt_i));
									chist1_N[6][asic_i][s_i]->Fill(adc_to_charge_N[6][asic_i][s_i][adc->at(evt_i)]);
								}
							}
						}		
					}
					
				}
			}//106 ********************************************************************

		}
	}
	
	TFile* output = new TFile(outputfile_root, "RECREATE");
	TLine* Line[10];
	output->cd();
	for(int i=1; i<10; i++){
		if (i!=2 && i!=6){continue;}
		canvas1[i]->cd();
		//canvas1[i]->Divide(2,1);
		//canvas1[i]->cd(1);
		//hist1_P[i]->Draw();
		//for(int j=1; j<8; j++){
		//	Line[i]->Draw();
		//}
		//canvas1[i]->cd(2);
		phist1_N[i]->Draw();
		for(int i=1; i<8; i++){
			Line[i] = new TLine(128*i, 0, 128*i, 1e5);
			Line[i]->SetLineWidth(3);
			Line[i]->SetLineColor(kViolet);
			Line[i]->SetLineStyle(1);
			Line[i]->Draw();
		}
		canvas1[i]->Write();	
	}
	output->Close();
	chist1_N[6][0][10]->Draw();
}


