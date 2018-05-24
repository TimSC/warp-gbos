
#include "ReadDelimitedFile.h"
#include "ganzc/LatLong-OSGBconversion.h"
#include <string.h>
#include <stdlib.h>

#define SHEET_WIDTH 40000	
#define SHEET_HEIGHT 45000

int BoundsSearchForString(class DelimitedFile &boundsFile, const char *filename, vector<vector<int> > &boundsOut)
{
	boundsOut.clear();
	for(unsigned int i=0;i<boundsFile.NumLines();i++)
	{
		class DelimitedFileLine &line = boundsFile.GetLine(i);
		if(line.NumVals() < 3) continue;
		if(strcmp(line[0].GetVals(),filename)!=0) continue;

		//Read details from this line
		vector<int> coord;
		for(unsigned int j=1;j<line.NumVals();j++)
		{
			//cout << strlen(line[j].GetVals()) << "," << line[j].GetVals() << endl;
			string valStr = line[j].GetVals();
			if (valStr.length() == 0) continue;
			int isNumber = 1;
			char firstLetter = valStr.substr(0,1).c_str()[0];
			if((firstLetter >= 'a' && firstLetter <= 'z') || (firstLetter >= 'A' && firstLetter <= 'Z'))
				isNumber = 0;
			
			if(isNumber)
			{
				coord.push_back(line[j].GetVali());
				if (coord.size() == 2)
				{
					boundsOut.push_back(coord);
					cout << "coord size: " << coord.size() << endl;
					coord.clear();
				}
			}
			else
			{
				int dEasting=0, dNorthing=0;
				OSGBGridRefToRefCoords(valStr.c_str(),dEasting,dNorthing);
				coord.clear();
				coord.push_back(dEasting);
				coord.push_back(dNorthing);
				boundsOut.push_back(coord);
				coord.clear();
			}
		}
		return boundsOut.size();
	}
	return boundsOut.size();
}

int GetBoundsFromFile(class DelimitedFile &boundsFile, const char *filename, vector<vector<int> > &boundsOut)
{
	boundsOut.clear();

	//Try to find bounds by sheet name first
	//Remove trailing ".kml"
	string sheetname = filename;
	int kmlEnd = (sheetname.substr(sheetname.length()-4,4)==".kml");
	if (sheetname.substr(sheetname.length()-4,4)==".KML") kmlEnd = 1;
	if (kmlEnd)
		sheetname = sheetname.substr(0,sheetname.length()-4);

	BoundsSearchForString(boundsFile, sheetname.c_str(), boundsOut);
	if (boundsOut.size() > 0)
		return boundsOut.size();

	//Remove leading 0 (if any)
	int zeroTrimming = 1;
	while (zeroTrimming)
	{
		if(sheetname.length() > 0 && sheetname.substr(0,1)=="0")
		{
			sheetname = sheetname.substr(1,sheetname.length()-1);
		}
		else {zeroTrimming = 0; continue;}
		//cout << "Check sheet name: " << sheetname << endl;

		BoundsSearchForString(boundsFile, sheetname.c_str(), boundsOut);
		if (boundsOut.size() > 0)
		{	//cout << "here" << boundsOut[0].size() << endl;
			return boundsOut.size();
		}
	}

	//Fall back to searching by filename
	return BoundsSearchForString(boundsFile, filename, boundsOut);
}


int GetBounds(class DelimitedFile &boundsFile, const char *filename, vector<vector<int> > &boundsOut)
{
	boundsOut.clear();

	GetBoundsFromFile(boundsFile, filename, boundsOut);
	if (boundsOut.size() == 1)
	{
		//If we only have one point, assume it is the corner of a full OS sheet
		//cout << "Bottom left at: " << boundsOut[0][0] << "," << boundsOut[0][1] << endl;
		vector<int> topRight;
		topRight.push_back(boundsOut[0][0]+SHEET_WIDTH);
		topRight.push_back(boundsOut[0][0]+SHEET_HEIGHT);
		boundsOut.push_back(topRight);
	}

	return boundsOut.size();
}

