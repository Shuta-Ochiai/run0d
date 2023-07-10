#include "nlohmann/json.hpp"
#include <TMath.h>
using json = nlohmann::json;

const int module_No = 10;
const int asic_No = 8;
const int strip_ch = 128;
const int adc_ch = 32;

//クラスを作ってファイル分割したい
//get adc to charge table ************************************************************************
double**** create4DAry(char Pol){
	ifstream ifs_json("./ModuleMap.json");
	if(!ifs_json.is_open()){
		cerr << "Could not open the file : "
			<< "./ModuleMap.json" << endl;
		exit(1);
	}
	json j = json::parse(ifs_json);
	ifs_json.close();
	
	string inputfile_csv_P[module_No][asic_No];
	string inputfile_csv_N[module_No][asic_No];
	
	//declare 4DAry for converting adc to charge	
	double**** adc_to_charge = new double***[module_No];
	for(int m_i=1; m_i<module_No; m_i++)
	{
		adc_to_charge[m_i] = new double**[asic_No];
		for(int asic_i=0; asic_i<asic_No; asic_i++)
		{
			adc_to_charge[m_i][asic_i] = new double*[strip_ch];
			for(int s_i=0; s_i<strip_ch; s_i++)
			{
				adc_to_charge[m_i][asic_i][s_i] = new double[adc_ch];
			}
		}
	}
	
	string delim = ", ";
	string line;
	int strip_i = 0;
	int adc_i = -1;
	size_t pos = 0;
	
	//initialize 4DAry
	for (int m_i=1; m_i<module_No; m_i++){
		if(m_i==5){continue;};
		for(int asic_i=0; asic_i<asic_No; asic_i++){
			ifstream ifs_csv;
			if(Pol=='P'){
				inputfile_csv_P[m_i][asic_i] = j[Form("10%d_P_ASIC_No%d", m_i, asic_i)]["adc_to_charge"];
				ifs_csv.open(inputfile_csv_P[m_i][asic_i].c_str());
				if(!ifs_csv.is_open()){
					cerr << "Could not open the file : "
						<< inputfile_csv_P[m_i][asic_i] << endl;
					exit(1);
				}
			}
			else if(Pol=='N'){
				inputfile_csv_N[m_i][asic_i] = j[Form("10%d_N_ASIC_No%d", m_i, asic_i)]["adc_to_charge"];
				ifs_csv.open(inputfile_csv_N[m_i][asic_i].c_str());	
				//cout << inputfile_csv_N[m_i][asic_i] << endl;
				if(!ifs_csv.is_open()){
					cerr << "Could not open the file : "
						<< inputfile_csv_N[m_i][asic_i] << endl;
					exit(1);
				}
			}
			getline(ifs_csv, line); //skip first row
			while(getline(ifs_csv, line)){
				while((pos=line.find(delim)) != string::npos){
					if(adc_i>=0){ //skip first column
						adc_to_charge[m_i][asic_i][strip_i][adc_i] = stod(line.substr(0, pos));
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
			ifs_csv.close();
			strip_i=0;

		}
	}	
	return adc_to_charge;
}

void delete4DAry(double**** ary){
	for(int m_i=1; m_i<module_No; m_i++){
		for(int asic_i=0; asic_i<asic_No; asic_i++){
			for(int s_i=0; s_i<strip_ch; s_i++){
				delete[] ary[m_i][asic_i][s_i];
			}	
			delete[] ary[m_i][asic_i];
		}
		delete[] ary[m_i];
	}
	delete[] ary;
}

void charge(const char* inputfile_root){
	//100番台のみ考える
	TCanvas* pcanvas[10];
	for (int i=1; i<10; i++){
		pcanvas[i] = new TCanvas(Form("pcanvas%d", i), Form("pcanvas%d", i), 800, 800);
	}
		
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

	// profile
	TH1D* phist1_P[module_No];
	TH1D* phist1_N[module_No];
	// mean_strip
	TH2D* mean_strip_hist1_P[module_No];
	TH2D* mean_strip_hist1_N[module_No];
	// adc
	TH1D* ahist1_P[module_No][asic_No][strip_ch];
	TH1D* ahist1_N[module_No][asic_No][strip_ch];
	// charge
	TH1D* chist1_P[module_No][asic_No][strip_ch];
	TH1D* chist1_N[module_No][asic_No][strip_ch];
	TH1D* chist_all = new TH1D("charge for all strip","charge for all strip", 16, 0, 16);
	chist_all->GetXaxis()->SetTitle("Charge[fC]");
	chist_all->GetYaxis()->SetTitle("Counts");

	double evt_charge;
	
	for (int m_i=1; m_i<module_No; m_i++){
		if (m_i==5){continue;}
		//profile hist ************************************************
		phist1_P[m_i] = new TH1D(Form("profile 10%d_P",m_i), Form("profile 10%d P-side", m_i), 1024, 0, 1023);
		phist1_P[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_P[m_i]->GetYaxis()->SetTitle("Counts");
		//phist1_P[m_i]->GetYaxis()->SetRangeUser(0,1e6);
		
		phist1_N[m_i] = new TH1D(Form("profile 10%d_N",m_i), Form("profile 10%d N-side", m_i), 1024, 0, 1023);
		phist1_N[m_i]->GetXaxis()->SetTitle("Ch");
		phist1_N[m_i]->GetYaxis()->SetTitle("Counts");
		//phist1_N[m_i]->GetYaxis()->SetRangeUser(0,1e6);
		
		// mean_strip_hist *******************************************
		mean_strip_hist1_P[m_i] = new TH2D(Form("mean_strip 10%d_P", m_i), Form("mean_strip 10%d_P", m_i), 1000, -5, 5, 100, 0, 1);
		mean_strip_hist1_P[m_i]->GetXaxis()->SetTitle("weighted_mean_strip - strip");
		mean_strip_hist1_P[m_i]->GetYaxis()->SetTitle("Charge_for_each_strip/ALL_Charge");
		
		mean_strip_hist1_N[m_i] = new TH2D(Form("mean_strip 10%d_N", m_i), Form("mean_strip 10%d_N", m_i), 1000, -5, 5, 100, 0, 1);
		mean_strip_hist1_N[m_i]->GetXaxis()->SetTitle("weighted_mean_strip - strip");
		mean_strip_hist1_N[m_i]->GetYaxis()->SetTitle("Charge_for_each_strip/ALL_Charge");
		
		
		for (int asic_i=0; asic_i<asic_No; asic_i++){
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
	double**** charge_P = create4DAry('P');
	double**** charge_N = create4DAry('N');

	int asicNo_PB[16] = {6, 4, 2, 0, 7, 5, 3, 1, 0, 2, 4, 6, 1, 3, 5, 7};
	int asicNo_PA[16] = {0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 7, 5, 3, 1};
	
	int evt_asic;
	int evt_strip;
	int evt_adc;

	int tdc_pre=0;
	int strip_pre=0;
	int same_evt_flag = 0;
	double same_evt_strip[1000];
	double same_evt_charge[1000];
	int same_evt_cnt = 0;
	double weighted_strip = 0;
	double same_evt_charge_all = 0;
	double weighted_mean_strip;
	double strip_diff;
	double charge_contri;
	
	
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
					
					evt_asic = asicNo_PB[elink->at(evt_i)];
					evt_strip = ch->at(evt_i);
					evt_adc = adc->at(evt_i);

					phist1_N[6]->Fill(evt_strip+128*evt_asic);	
					ahist1_N[6][evt_asic][evt_strip]->Fill(evt_adc);
					evt_charge = charge_N[6][evt_asic][evt_strip][evt_adc];
					chist1_N[6][evt_asic][evt_strip]->Fill(evt_charge);	
					if(evt_asic!=1){
						chist_all->Fill(evt_charge);
					}

					if(tdcL10b->at(evt_i)==tdc_pre && TMath::Abs(strip_pre-evt_strip)<=1){
						same_evt_strip[same_evt_cnt] = evt_strip;
						same_evt_charge[same_evt_cnt] = evt_charge;
						weighted_strip += evt_strip*evt_charge;
						same_evt_charge_all += evt_charge;
						same_evt_cnt += 1; 
						same_evt_flag = 1;
					}
					if( (same_evt_flag==1) && (tdc_pre!= tdcL10b->at(evt_i)) ){
						weighted_mean_strip = weighted_strip / same_evt_charge_all;
						for(int same_evt_i=0; same_evt_i<same_evt_cnt; same_evt_i++){
							strip_diff = weighted_mean_strip - same_evt_strip[same_evt_i];
							charge_contri = same_evt_charge[same_evt_i] / same_evt_charge_all;
							mean_strip_hist1_N[6]->Fill(strip_diff, charge_contri);
						}
						weighted_strip = 0;
						same_evt_charge_all = 0;
						same_evt_cnt = 0;
						same_evt_flag = 1;
					}
					tdc_pre = tdcL10b->at(evt_i);
					strip_pre = evt_strip;
				}
			}//106 ********************************************************************

		}
	}

	delete4DAry(charge_P);
	delete4DAry(charge_N);
	
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
	c1->Print(adc_pdf + "[");
	int cnt = 1;
	for (int asic_i=0; asic_i<asic_No; asic_i++){
		for(int s_i=0; s_i<strip_ch; s_i++){
			if(cnt==1){
				c1->Clear();
				c1->Divide(4,4);
			}
			c1->cd(cnt);
			ahist1_N[6][asic_i][s_i]->Draw();
			cnt += 1;
			if (cnt>16){
				//c1->Draw();
				//c1->Update();
				c1->Print(adc_pdf);
				cnt = 1; //reset cnt
			}
		}
	}
	c1->Print(adc_pdf + "]");
	
	c1->Print(charge_pdf + "[");
	cnt = 1;
	for (int asic_i=0; asic_i<asic_No; asic_i++){
		for(int s_i=0; s_i<strip_ch; s_i++){
			if(cnt==1){
				c1->Clear();
				c1->Divide(4,4);
			}
			c1->cd(cnt);
			chist1_N[6][asic_i][s_i]->Draw();
			cnt += 1;
			if (cnt>16){
				//c1->Draw();
				//c1->Update();
				c1->Print(charge_pdf);
				cnt = 1; //reset cnt
			}
		}
	}
	c1->Clear();
	chist_all->Draw();
	chist_all->Fit("landau");
	c1->Print(charge_pdf);

	c1->Clear();
	mean_strip_hist1_N[6]->Draw("colz");
	c1->Print(charge_pdf);

	c1->Print(charge_pdf + "]");
	
	
}


