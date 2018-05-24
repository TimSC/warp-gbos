#ifndef WRITE_KML_H
#define WRITE_KML_H

#include <string>
using namespace std;

class WriteKml
{
public:
	double north, south, east,west;
	string folderName, description;
	string overlayName, href;

	WriteKml();
	void Write(string &out);
	void WriteToFile(const char *filename);
};


#endif //WRITE_KML_H

