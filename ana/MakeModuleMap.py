import json

csv_file_dir = ""

dict={}

# Nside:elink0-7, Pside:elink8-15
asicNo_to_Addr_PB=[7, 6, 5, 4, 3, 2, 1, 0, 1, 0, 3, 2, 5, 4, 7, 6]
asicNo_to_Addr_PA=[1, 0 ,3, 2, 5, 4, 7, 6, 7, 6, 5, 4, 3, 2, 1, 0]

with open("./EfuseMap.json") as f:
	try:
		d = json.load(f)
	except FileNotFoundErro as err:
		print(err)

j = 1

for i in range(10):
	if i<8:
		if j==5:
			j+=1
		if i%2 == 0:#PB type
			for a_i in range(0, 8):
				for key in d.keys():
					if(d[key]["Module_ID"]=="10{}_PB".format(j) and d[key]["Pol"]=="P-side" and d[key]["ASIC_No"]==a_i):
						tmp = {"10{}_P_ASIC_No{}".format(j, a_i):
							{
							"ASIC_Addr":asicNo_to_Addr_PB[a_i+8],
							"Efuse":key,
							"adc_to_charge":"path"
							}
						}
				dict.update(tmp)

			for a_i in range(0, 8):
				for key in d.keys():
					if(d[key]["Module_ID"]=="10{}_PB".format(j) and d[key]["Pol"]=="N-side" and d[key]["ASIC_No"]==a_i):
						if(j==6):
							path = "../pscan_106_202306212131_ALineOnly_FMOFF/pscan_{}_addr{}_ADCtoCharge.csv".format(key, d[key]["ASIC_Addr"])
						else: path = "path"
						tmp = {"10{}_N_ASIC_No{}".format(j,a_i):
							{
							"ASIC_Addr":asicNo_to_Addr_PB[a_i],
							"Efuse":key,	
							"adc_to_charge":path
							}
						}
				dict.update(tmp)

		elif i%2 != 0:#PA type
			for a_i in range(0, 8):
				for key in d.keys():
					if(d[key]["Module_ID"]=="10{}_PA".format(j) and d[key]["Pol"]=="P-side" and d[key]["ASIC_No"]==a_i):
						tmp = {"10{}_P_ASIC_No{}".format(j, a_i):
							{
							"ASIC_Addr":asicNo_to_Addr_PA[a_i+8],
							"Efuse":key,
							"adc_to_charge":"path"
							}
						}
				dict.update(tmp)

			for a_i in range(0, 8):
				for key in d.keys():
					if(d[key]["Module_ID"]=="10{}_PA".format(j) and d[key]["Pol"]=="N-side" and d[key]["ASIC_No"]==a_i):
						tmp = {"10{}_N_ASIC_No{}".format(j,a_i):
							{
							"ASIC_Addr":asicNo_to_Addr_PA[a_i],
							"Efuse":key,
							"adc_to_charge":"path"
							}
						}
				dict.update(tmp)
		j+=1

	elif i==8:
		for a_i in range(0, 8):
			for key in d.keys():
				if(d[key]["Module_ID"]=="206_PA" and d[key]["Pol"]=="P-side" and d[key]["ASIC_No"]==a_i):
					tmp = {"206_P_ASIC_No{}".format(a_i):
						{
						"ASIC_Addr":asicNo_to_Addr_PA[a_i+8],
						"Efuse":key,
						"adc_to_charge":"path"
						}
					}
			dict.update(tmp)

		for a_i in range(0, 8):
			for key in d.keys():
				if(d[key]["Module_ID"]=="206_PA" and d[key]["Pol"]=="N-side" and d[key]["ASIC_No"]==a_i):
					tmp = {"206_N_ASIC_No{}".format(a_i):
						{
						"ASIC_Addr":asicNo_to_Addr_PA[a_i],
						"Efuse":key,
						"adc_to_charge":"path"
						}
					}
			dict.update(tmp)
	elif i==9:
		for a_i in range(0, 8):
			for key in d.keys():
				if(d[key]["Module_ID"]=="207_PA" and d[key]["Pol"]=="P-side" and d[key]["ASIC_No"]==a_i):
					tmp = {"207_P_ASIC_No{}".format(a_i):
						{
						"ASIC_Addr":asicNo_to_Addr_PA[a_i+8],
						"Efuse":key,
						"adc_to_charge":"path"
						}
					}
			dict.update(tmp)
		for a_i in range(0, 8):
			for key in d.keys():
				if(d[key]["Module_ID"]=="207_PA" and d[key]["Pol"]=="P-side" and d[key]["ASIC_No"]==a_i):
					tmp = {"207_N_ASIC_No{}".format(a_i):
						{
						"ASIC_Addr":asicNo_to_Addr_PA[a_i],
						"Efuse":key,
						"adc_to_charge":"path"
						}
					}
			dict.update(tmp)

with open("./ModuleMap.json", mode="w") as json_file:
		json.dump(dict, json_file, indent=4)
