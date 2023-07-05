#include "nlohmann/json.hpp"

using json = nlohmann::json;

void read_json(){
	FILE* fp = fopen("./ModuleMap.json", "r");
	json j = json::parse(fp);
	fclose(fp);
	//char test = j["XA-000-08-001-064-063-216-06"]["adc_to_charge"].c_str();
	//cout << test << endl;
	string name = j["106_N_ASIC_No1"]["adc_to_charge"];
	const char* value = name.c_str();
	cout << value << endl;
	cout << typeid(int).name() << endl;
	cout << typeid(char).name() << endl;
}

