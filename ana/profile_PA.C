void profile_PA(const char* filename, const char* output_name){
	
	TCanvas* canvas = new TCanvas("canvas", "canvas", 800, 600);
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
	
	TH1D* hist = new TH1D("hist", "102 N-side", 1024, 0, 1023);
	hist->GetXaxis()->SetTitle("Ch");
	hist->GetYaxis()->SetTitle("Counts");
	hist->GetYaxis()->SetRangeUser(0,1e5);
	int ASIC_num[8] = {0, 2, 4, 6, 1, 3, 5, 7};
	
	int Entry = tree->GetEntries();
	for(int i = 0; i<Entry; i++){
		
		tree->GetEntry(i);
		for(int j=0; j<ch->size(); j++){
			if(adc->at(j)>1 && 0<=ch->at(j) && ch->at(j) <=127 && elink->at(j)<=7){ 
				hist->Fill(ch->at(j)+128*ASIC_num[elink->at(j)]);
			}
		}
	}
	hist->Draw();
	
	TFile* output = new TFile(output_name, "RECREATE");
	output->cd();
	hist->Write();
	output->Close();
}
