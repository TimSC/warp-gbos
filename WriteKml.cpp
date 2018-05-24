
#include <iostream>
#include <fstream>
#include <sstream>
#include "WriteKml.h"

string WriteKmlDoubleToString(double input)
{
	ostringstream out;
	out << input;
	return out.str();
}

WriteKml::WriteKml()
{
	north=50.0; south=49.0; east=-1.0; west= 0.0;
	folderName = "WarpFolder";
	overlayName = "WarpMap";
	href = "map.png";
	description = "No description";
}

void WriteKml::Write(string &out)
{
	out.clear();
	out+="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	out+="<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
	out+="  <Folder>\n";
	out+="    <open>1</open>\n";
	out+="    <name>";out+= folderName;out+="</name>\n";
	out+="    <description>";out+=description;out+="</description>\n";
	out+="    <GroundOverlay>\n";
	out+="      <name>";out+=overlayName;out+="</name>\n";
	out+="      <description></description>\n";
	out+="      <Icon>\n";
	out+="        <href>";out+=href;out+="</href>\n";
	out+="      </Icon>\n";
	out+="      <LatLonBox>\n";
	out+="        <north>";out+=WriteKmlDoubleToString(north);out+="</north>\n";
	out+="        <south>";out+=WriteKmlDoubleToString(south);out+="</south>\n";
	out+="        <east>";out+=WriteKmlDoubleToString(east);out+="</east>\n";
	out+="        <west>";out+=WriteKmlDoubleToString(west);out+="</west>\n";
	out+="        <rotation>0</rotation>\n";
	out+="      </LatLonBox>\n";
	out+="    </GroundOverlay>\n";
	out+="  </Folder>\n";
	out+="</kml>\n";
}

void WriteKml::WriteToFile(const char *filename)
{
	ofstream file(filename);
	string data;
	Write(data);
	//cout << data << endl;
	file.write(data.c_str(),data.size());
}

