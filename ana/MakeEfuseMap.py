import json
import os

json_file = open("./EfuseMap.json", mode="w")
trim_file_dir = "./E16_module_test_GSI/ftrim_files_all/"

j = 1
filename = []

tmp_f = os.listdir(trim_file_dir)
trim_files = [f for f in tmp_f  if os.path.isfile(os.path.join(trim_file_dir, f))]

dict={}

for i in range(10):
	if i<8:
		if i%2==0:
			str = "B"
		elif i%2!=0:
			str = "A"
		if j==5:
			j+=1
		filename.append("./E16_module_test_GSI/E16-10{0}_P{1}/E16-10{0}_P{1}.txt".format(j, str))
		j+=1
	elif i==8:
		filename.append("./E16_module_test_GSI/E16-206_PA/E16_206_PA.txt")
	elif i==9:
		filename.append("./E16_module_test_GSI/E16-207_PA/E16-207_PA.txt")	



for i in range(10):
	with open(filename[i]) as f:
		RowList = f.readlines()

	RowCount = -1

	for Row in RowList:
		RowCount+=1
		#print(i)
		if (Row[0:3] != "FEB"):
			continue
		#print(Row)
		Sep_5space = "     "
		ColumnList = Row.split(Sep_5space)
	
		Sep_KeyAndValue = ": "
		FEB = ColumnList[0].split(Sep_KeyAndValue)
		Polarity = ColumnList[1].split(Sep_KeyAndValue)
		ASIC_No = ColumnList[2].split(Sep_KeyAndValue)
		ASIC_Addr = ColumnList[3].split(Sep_KeyAndValue)
		ASIC_Efuse = ColumnList[4].split(Sep_KeyAndValue)
		ASIC_Efuse_str = ColumnList[5].split(Sep_KeyAndValue)
	
		VrefRow = RowList[RowCount+47]
		#print(VrefRow)
		Sep_Tab1 = "	"
		VrefColumnList = VrefRow.split(Sep_Tab1)
		#for c in VrefColumnList:
		#	print(c)
		
		#add trim file path
		for f in trim_files:
			if f[f.find("XA"):f.find("XA")+28] == ASIC_Efuse_str[1][0:28]:
				trim_file_path = trim_file_dir + f
				break

		tmp = {ASIC_Efuse_str[1][0:28]:{
			"Pol":Polarity[1],
			"ASIC_No":int(ASIC_No[1]),
			"ASIC_Addr":int(ASIC_Addr[1]),
			"Vref_N":int(VrefColumnList[1][2:4]),
			"Vref_P":int(VrefColumnList[3]),
			"Vref_T":0b111111&int(VrefColumnList[4]),
			"Thr2_glb":int(VrefColumnList[5][0:2]),
			"Module_ID":filename[i][-10:-4],
			"trim_file":trim_file_path
			}
		}
	
		dict.update(tmp)

		#print("")

#print(dict)

with open("./EfuseMap.json", mode="w") as json_file:
	json.dump(dict, json_file, indent=4)

