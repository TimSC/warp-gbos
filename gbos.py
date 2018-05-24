
#Quick batch script to generate rectified sheets in GB coordinate system
#PERL5LIB=/home/tim/dev/Geo-Coordinates-OSGB-2.04/lib

import os
import subprocess
import os.path


path="cal-os7th"  # insert the path to the directory of interest
dirList=os.listdir(path)

if 1:
	for fname in dirList:
		if fname[-1] == "~": continue
		if fname[0:8] != "os7sheet": continue
		sheetNum = fname[8:11]
		print sheetNum

		if fname[11:14] == "sub": continue
		#print fname, sheetNum

		if os.path.isfile(sheetNum+".jpg"): continue
		cmd = "nice ./warpos --in ../os7files/source/"+sheetNum+"half.jpg --points cal-os7th/"+fname+" --out "+sheetNum+" --fit 2 --outproj gbos"
		print cmd
		subprocess.call(cmd.split(" "))

if 1:
	for fname in dirList:
		if fname[-1] == "~": continue
		if fname[0:8] != "os7sheet": continue
		if fname[11:14] != "sub" : continue
		sheetNum = fname[8:11]
		print sheetNum

		#print fname, sheetNum
		if os.path.isfile(sheetNum+fname[11:15]+".jpg"): continue
		cmd = "nice ./warpos --in ../os7files/source/"+sheetNum+"half"+fname[11:15]+".jpg --points cal-os7th/"\
		+fname+" --out "+sheetNum+fname[11:15]+" --fit 1 --outproj gbos"
		print cmd
		subprocess.call(cmd.split(" "))
