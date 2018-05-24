//LatLong- OSGB conversion..h
//definitions for lat/long to OSGB and OSGB to lat/lng conversions
#include <string.h>

#ifndef LATLONGCONV
#define LATLONGCONV

void LLtoOSGB(const double Lat, const double Long, 
			  long &OSGBEasting, long &OSGBNorthing, char OSGBGridSquare[3]);
void LLtoOSGBGrid(const double Lat, const double Long, 
			 double &easting, double &northing);

void OSGBtoLL(const double OSGBNorthing, const double OSGBEasting, const char* OSGBZone,
			  double& Lat,  double& Long );
char OSGBLetterDesignator(double Lat);
void CoordsToOSGBSquare(double easting, double northing,  
				  char OSGBGridSquare[3], long &OSGBEasting, long &OSGBNorthing);
void OSGBSquareToRefCoords(const char* OSGBGridSquare,
						   int &RefEasting, int &RefNorthing);

void OSGBGridRefToRefCoords(const char* OSGBpos,
						   int &RefEasting, int &RefNorthing);


class GanzcTransform
{
public:
	void ConvertGbos1936ToWgs84(double ea, double no, double he,
		double &latOut, double &lonOut, double &heOut);
	void ConvertWgs84ToGbos1936(double lat, double lon, double he,
		double &eaOut, double &noOut, double &heOut);
	void GetTransformName(char *buffer, int maxLen)
	{
		strncpy(buffer, "Ganzc Gbos1936", maxLen);
	}
};


#endif

