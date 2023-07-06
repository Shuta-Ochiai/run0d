#include "nlohmann/json.hpp"
using json = nlohmann::json;

//get adc to charge table ************************************************************************
double** create2DAry(const char* inputfile_csv){
	ifstream ifs(inputfile_csv);
	if(!ifs.is_open()){
		cerr << "Could not open the file : "
			<< inputfile_csv << endl;
		exit(1);
	}
	
	double** adc_to_charge = new double*[128];
	//cout << "get row" << endl;
	for(int i=0; i<128; i++){
		adc_to_charge[i] = new double[32];
	}
	//cout << "get column" << endl;
	string delim = ", ";
	string line;
	int strip_i = 0;
	int adc_i = -1;
	size_t pos = 0;
	//cout << "par init " << endl;
	getline(ifs, line); //skip first row
	//cout << "get first line" << endl;
	while(getline(ifs, line)){
		while((pos=line.find(delim)) != string::npos){
			if(adc_i>=0){ //skip first column
				adc_to_charge[strip_i][adc_i] = stod(line.substr(0, pos));
				//cout << adc_to_charge[strip_i][adc_i] << " ";
			}
			line.erase(0, pos+delim.length());
			adc_i += 1;
		}
		//cout << endl;
		adc_i = -1;
		strip_i += 1;
		//cout << strip_i << endl;
	}
	ifs.close();
	//cout << "csv closed" << endl;
	return adc_to_charge;
}
void delete2DAry(double** ary){
	for(int i=0; i<128; i++){
		delete[] ary[i];
		}	
	delete[] ary;
}

void charge(const char* inputfile_root){
	//100番台のみ考える
	TCanvas* pcanvas[10];
	for (int i=1; i<10; i++){
		pcanvas[i] = new TCanvas(Form("pcanvas%d", i), Form("pcanvas%d", i), 800, 800);
	}
	ifstream ifs("./ModuleMap.json");
	if (!ifs.is_open()){
		cerr << "Could not open the file : "
			<< "./ModuleMap.json" << endl;
		exit(1);
	}
	json j = json::parse(ifs);
	string inputfile_csv_P[10][8];
	const char* file_csv_P[10][8];
	string inputfile_csv_N[10][8];
	const char* file_csv_N[10][8];
	
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
	TH1D* chist_all = new TH1D("charge for all strip","charge for all strip", 16, 0, 16);
	chist_all->GetXaxis()->SetTitle("Charge[fC]");
	chist_all->GetYaxis()->SetTitle("Counts");

	double charge;
	
	for (int m_i=1; m_i<module_No; m_i++){
		if (m_i==5){continue;}
		//profile hist ************************************************
		phist1_P[m_i] = new TH1D(Form("profile 10%d_P",m_i), Form(" profile 10%d P-side", m_i), 1024, 0, 1023);
		phist1_P[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_P[m_i]->GetYaxis()->SetTitle("Counts");
		//phist1_P[m_i]->GetYaxis()->SetRangeUser(0,1e6);
		
		phist1_N[m_i] = new TH1D(Form("profile 10%d_N",m_i), Form("profile 10%d N-side", m_i), 1024, 0, 1023);
		phist1_N[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_N[m_i]->GetYaxis()->SetTitle("Counts");
		//phist1_N[m_i]->GetYaxis()->SetRangeUser(0,1e6);
		
		for (int asic_i=0; asic_i<asic_No; asic_i++){
			//get adc to charge csv 
			inputfile_csv_P[m_i][asic_i] = 	j[Form("10%d_P_ASIC_No%d", m_i, asic_i)]["adc_to_charge"];
			file_csv_P[m_i][asic_i] = inputfile_csv_P[m_i][asic_i].c_str();
			inputfile_csv_N[m_i][asic_i] = 	j[Form("10%d_N_ASIC_No%d", m_i, asic_i)]["adc_to_charge"];
			file_csv_N[m_i][asic_i] = inputfile_csv_N[m_i][asic_i].c_str();
			
			for (int s_i=0; s_i<strip_ch; s_i++){
				//adc hist **********************************************
				ahist1_P[m_i][asic_i][s_i] = new TH1D(Form("adc 10%d_P ASIC#%d Ch%d",m_i, asic_i, s_i), Form("adc 10%d P-side ASIC#%d Ch%d", m_i, asic_i, s_i), 31, 0, 31);
				ahist1_P[m_i][asic_i][s_i]->GetXaxis()->SetTitle("ADC");
				ahist1_P[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//ahist1_P[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
			
				ahist1_N[m_i][asic_i][s_i] = new TH1D(Form("adc 10%d_N ASIC#%d Ch#%d",m_i, asic_i, s_i), Form("adc 10%d N-side ASIC#%d Ch#%d", m_i, asic_i, s_i), 31, 0, 31);
				ahist1_N[m_i][asic_i][s_i]->GetXaxis()->SetTitle("ADC");
				ahist1_N[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//ahist1_N[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
		
				//charge hist *********************************************
				chist1_P[m_i][asic_i][s_i] = new TH1D(Form("10%d P-side ASIC#%d Ch%d",m_i, asic_i, s_i), Form("charge 10%d P-side ASIC#%d Ch%d", m_i, asic_i, s_i), 16, 0, 16);
				chist1_P[m_i][asic_i][s_i]->GetXaxis()->SetTitle("Charge[fC]");
				chist1_P[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//chist1_P[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
	
				chist1_N[m_i][asic_i][s_i] = new TH1D(Form("10%d N-side ASIC#%d Ch%d",m_i, asic_i, s_i), Form("charge 10%d N-side ASIC#%d Ch%d", m_i, asic_i, s_i), 16, 0 ,16);
				chist1_N[m_i][asic_i][s_i]->GetXaxis()->SetTitle("Charge[fC]");
				chist1_N[m_i][asic_i][s_i]->GetYaxis()->SetTitle("Counts");
				//chist1_N[m_i][asic_i][s_i]->GetYaxis()->SetRangeUser(0,1e5);
			}
		}
	}
	
	double** charge_106N_0 = create2DAry(file_csv_N[6][0]);
	cout << "0" << endl;
	double** charge_106N_1 = create2DAry(file_csv_N[6][1]);
	cout << "1" << endl;
	double** charge_106N_2 = create2DAry(file_csv_N[6][2]);
	cout << "2" << endl;
	double** charge_106N_3 = create2DAry(file_csv_N[6][3]);
	cout << "3" << endl;
	double** charge_106N_4 = create2DAry(file_csv_N[6][4]);
	cout << "4" << endl;
	double** charge_106N_5 = create2DAry(file_csv_N[6][5]);
	cout << "5" << endl;
	double** charge_106N_6 = create2DAry(file_csv_N[6][6]);
	cout << "6" << endl;
	double** charge_106N_7 = create2DAry(file_csv_N[6][7]);
	cout << "7" << endl;

	int asicNo_PB[16] = {6, 4, 2, 0, 7, 5, 3, 1, 0, 2, 4, 6, 1, 3, 5, 7};
	int asicNo_PA[16] = {0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 7, 5, 3, 1};
	
	int Entry = tree->GetEntries();
	for(int ent_i = 0; ent_i<Entry; ent_i++){
		tree->GetEntry(ent_i);
		for(int evt_i=0; evt_i<ch->size(); evt_i++){
			/*if(gbt->at(evt_i)==1){//port1 102(PA) ************************************
				continue;	
				if(adc->at(evt_i)>1 && 0<=ch->at(evt_i) && ch->at(evt_i) <128 && elink->at(evt_i)<8){ 
					phist1_N[2]->Fill(ch->at(evt_i)+128*asicNo_PA[elink->at(evt_i)]);
					//ahist1_N[2][asic_i][s_i]->Fill(adc->at(evt_i));
					//chist1_N[2][asic_i][s_i]->Fill(adc_to_charge[2][asic_i][s_i][adc->at(evt_i)]);		
				}
			}//102 *******************************************************************
			*/
			if(gbt->at(evt_i)==2){//port2 106(PB) *******************************
				if(adc->at(evt_i)>1 && 0<=ch->at(evt_i) && ch->at(evt_i) <128 && elink->at(evt_i)<8){ 
					phist1_N[6]->Fill(ch->at(evt_i)+128*asicNo_PB[elink->at(evt_i)]);	
					ahist1_N[6][asicNo_PB[elink->at(evt_i)]][ch->at(evt_i)]->Fill(adc->at(evt_i));
					if(asicNo_PB[elink->at(evt_i) == 0]){
						charge = charge_106N_0[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 1){
						charge = charge_106N_1[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 2){
						charge = charge_106N_2[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 3){
						charge = charge_106N_3[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 4){
						charge = charge_106N_4[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 5){
						charge = charge_106N_5[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 6){
						charge = charge_106N_6[ch->at(evt_i)][adc->at(evt_i)];
					}
					else if(asicNo_PB[elink->at(evt_i)] == 7){
						charge = charge_106N_7[ch->at(evt_i)][adc->at(evt_i)];
					}
					chist1_N[6][asicNo_PB[elink->at(evt_i)]][ch->at(evt_i)]->Fill(charge);	
					if(asicNo_PB[elink->at(evt_i)]!=1){
						chist_all->Fill(charge);}
				}
			}//106 ********************************************************************

		}
	}
	delete2DAry(charge_106N_0);
	delete2DAry(charge_106N_1);
	delete2DAry(charge_106N_2);
	delete2DAry(charge_106N_3);
	delete2DAry(charge_106N_4);
	delete2DAry(charge_106N_5);
	delete2DAry(charge_106N_6);
	delete2DAry(charge_106N_7);
	
	TString tmp = inputfile_root;
	TString outputfile_root;
	TString adc_pdf;
	TString charge_pdf;

	int ipos = tmp.Last('.');
	tmp.Remove(ipos);
	outputfile_root = tmp + "_profile.root";
	adc_pdf = tmp + "_adc.pdf";
	charge_pdf = tmp + "_charge.pdf";
	
	TFile* output_root = new TFile(outputfile_root, "RECREATE");
	TLine* Line[10];
	output_root->cd();
	for(int i=1; i<10; i++){
		if (i!=2 && i!=6){continue;}
		pcanvas[i]->cd();
		//pcanvas[i]->Divide(2,1);
		//pcanvas[i]->cd(1);
		//phist1_P[i]->Draw();
		//for(int j=1; j<8; j++){
		//	Line[i]->Draw();
		//}
		//pcanvas[i]->cd(2);
		phist1_N[i]->Draw();
		for(int i=1; i<8; i++){
			Line[i] = new TLine(128*i, 0, 128*i, 1e5);
			Line[i]->SetLineWidth(3);
			Line[i]->SetLineColor(kViolet);
			Line[i]->SetLineStyle(1);
			Line[i]->Draw();
		}
		pcanvas[i]->Write();	
	}
	output_root->Close();
	
	TCanvas* c1 = new TCanvas("c1", "c1", 800, 800);
	c1->Print(adc_pdf + "[", "PDF");
	int cnt = 1;
	for (int asic_i=0; asic_i<1; asic_i++){
		for(int s_i=0; s_i<strip_ch; s_i++){
			if(cnt==1){
				c1->Clear();
				c1->Divide(4,4);
			}
			c1->cd(cnt);
			ahist1_N[6][asic_i][s_i]->Draw();
			cnt += 1;
			if (cnt>16){
				c1->Draw();
				c1->Update();
				c1->Print(adc_pdf, "PDF");
				cnt = 1; //reset cnt
			}
		}
	}
	c1->Print(adc_pdf + "]", "PDF");
	
	c1->Print(charge_pdf + "[", "PDF");
	cnt = 1;
	for (int asic_i=0; asic_i<1; asic_i++){
		for(int s_i=0; s_i<strip_ch; s_i++){
			if(cnt==1){
				c1->Clear();
				c1->Divide(4,4);
			}
			c1->cd(cnt);
			chist1_N[6][asic_i][s_i]->Draw();
			cnt += 1;
			if (cnt>16){
				c1->Draw();
				c1->Update();
				c1->Print(charge_pdf, "PDF");
				cnt = 1; //reset cnt
			}
		}
	}
	c1->Clear();
	chist_all->Draw();
	chist_all->Fit("landau");
	c1->Print(charge_pdf, "PDF");
	c1->Print(charge_pdf + "]", "PDF");
	
	
}


