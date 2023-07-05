void profile(const char* filename, const char* output_name){
	//100番台のみ考える
	TCanvas* canvas1[10];
	for (int i=1; i<10; i++){
		canvas1[i] = new TCanvas(Form("canvas%d", i), Form("canvas%d", i), 800, 600);
	}
	TFile* file = new TFile(filename);
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

	const int Nhist = 10;
	TH1D* hist1_P[Nhist];
	TH1D* hist1_N[Nhist];


	for (int i=1; i<Nhist; i++){
		if (i==5){continue;}

		hist1_P[i] = new TH1D(Form("hist10%d_P",i), Form("10%d P-side", i), 1024, 0, 1023);
		hist1_P[i]->GetXaxis()->SetTitle("Ch");
		hist1_P[i]->GetYaxis()->SetTitle("Counts");
		hist1_P[i]->GetYaxis()->SetRangeUser(0,1e5);
		
		hist1_N[i] = new TH1D(Form("hist10%d_N",i), Form("10%d N-side", i), 1024, 0, 1023);
		hist1_N[i]->GetXaxis()->SetTitle("Ch");
		hist1_N[i]->GetYaxis()->SetTitle("Counts");
		hist1_N[i]->GetYaxis()->SetRangeUser(0,1e5);
	}

	int ASIC_num_PB[8] = {6, 4, 2, 0, 7, 5, 3, 1};
	int ASIC_num_PA[8] = {0, 2, 4, 6, 1, 3, 5, 7};
	
	int Entry = tree->GetEntries();
	for(int i = 0; i<Entry; i++){
		tree->GetEntry(i);
		for(int j=0; j<ch->size(); j++){
			if(gbt->at(j)==1){//port1 102(PA)	
				if(adc->at(j)>1 && 0<=ch->at(j) && ch->at(j) <=127 && elink->at(j)<=7){ 
					hist1_N[2]->Fill(ch->at(j)+128*ASIC_num_PA[elink->at(j)]);
				}
			}
			else if(gbt->at(j)==2){//port2 106(PB)	
				if(adc->at(j)>1 && 0<=ch->at(j) && ch->at(j) <=127 && elink->at(j)<=7){ 
					hist1_N[6]->Fill(ch->at(j)+128*ASIC_num_PB[elink->at(j)]);
				}
			}
		}
	}
	
	TFile* output = new TFile(output_name, "RECREATE");
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
		hist1_N[i]->Draw();
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
}
