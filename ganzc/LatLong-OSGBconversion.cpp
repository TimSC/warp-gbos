//LatLong- OSGB conversion.cpp
//Lat Long - OSGB, OSGB - Lat Long conversions

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "LatLong-OSGBconversion.h"
#include <string>
using namespace std;


void LLtoOSGB(const double Lat, const double Long, 
			 long &OSGBEasting, long &OSGBNorthing, char OSGBGridSquare[3])
{
//converts lat/long to OSGB coords.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
	//Written by Chuck Gantz- chuck.gantz@globalstar.com

	double a;
	double eccSquared;
	double k0 = 0.9996012717;

	double LongOrigin = -2;
	double LongOriginRad = LongOrigin * deg2rad;
	double LatOrigin = 49;
	double LatOriginRad = LatOrigin * deg2rad;
	double eccPrimeSquared;
	double N, T, C, A, M;
	
	double LatRad = Lat*deg2rad;
	double LongRad = Long*deg2rad;
	double easting, northing;

	double majoraxis = a = 6377563.396;//Airy
	double minoraxis = 6356256.91;//Airy

	eccSquared = (majoraxis * majoraxis - minoraxis * minoraxis) / (majoraxis * majoraxis);

	//only calculate M0 once since it is based on the origin 
	//of the OSGB projection, which is fixed
	static double 	M0 = a*((1		- eccSquared/4 - 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatOriginRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatOriginRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatOriginRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatOriginRad));

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
	T = tan(LatRad)*tan(LatRad);
	C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
	A = cos(LatRad)*(LongRad-LongOriginRad);


	
	M = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
	
	easting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
					+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120));
	easting += 400000.0; //false easting

	northing = (double)(k0*(M-M0+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
				 + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	northing -= 100000.0;//false northing

	CoordsToOSGBSquare(easting, northing, OSGBGridSquare, OSGBEasting, OSGBNorthing);

}

void LLtoOSGBGrid(const double Lat, const double Long, 
			 double &easting, double &northing)
{
//converts lat/long to OSGB coords.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
	//Written by Chuck Gantz- chuck.gantz@globalstar.com

	double a;
	double eccSquared;
	double k0 = 0.9996012717;

	double LongOrigin = -2;
	double LongOriginRad = LongOrigin * deg2rad;
	double LatOrigin = 49;
	double LatOriginRad = LatOrigin * deg2rad;
	double eccPrimeSquared;
	double N, T, C, A, M;
	
	double LatRad = Lat*deg2rad;
	double LongRad = Long*deg2rad;

	double majoraxis = a = 6377563.396;//Airy
	double minoraxis = 6356256.91;//Airy

	eccSquared = (majoraxis * majoraxis - minoraxis * minoraxis) / (majoraxis * majoraxis);

	//only calculate M0 once since it is based on the origin 
	//of the OSGB projection, which is fixed
	static double 	M0 = a*((1		- eccSquared/4 - 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatOriginRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatOriginRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatOriginRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatOriginRad));

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
	T = tan(LatRad)*tan(LatRad);
	C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
	A = cos(LatRad)*(LongRad-LongOriginRad);


	
	M = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
	
	easting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
					+ (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120));
	easting += 400000.0; //false easting

	northing = (double)(k0*(M-M0+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
				 + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	northing -= 100000.0;//false northing

	//CoordsToOSGBSquare(easting, northing, OSGBGridSquare, OSGBEasting, OSGBNorthing);
}


void CoordsToOSGBSquare(double easting, double northing,  
				  char OSGBGridSquare[3], long &OSGBEasting, long &OSGBNorthing)
{
	char GridSquare[] = "VWXYZQRSTULMNOPFGHJKABCDE";
	int posx, posy; //positions in grid

	OSGBEasting = long(easting + 0.5); //round to nearest int
	OSGBNorthing = long(northing + 0.5); //round to nearest int

	//find correct 500km square
	posx = OSGBEasting / 500000L;
	posy = OSGBNorthing / 500000L;
	OSGBGridSquare[0] = GridSquare[posx + posy * 5 + 7];

	//find correct 100km square
	posx = OSGBEasting % 500000L;//remove 500 km square
	posy = OSGBNorthing % 500000L;//remove 500 km square
	posx = posx / 100000L;//find 100 km square
	posy = posy / 100000L;//find 100 km square
	OSGBGridSquare[1] = GridSquare[posx + posy * 5];

	OSGBGridSquare[2] = '\0';//terminate grid ref string with null

	//remainder is northing and easting
	OSGBNorthing = OSGBNorthing % 500000L; 
	OSGBNorthing = OSGBNorthing % 100000L;

	OSGBEasting = OSGBEasting % 500000L;
	OSGBEasting = OSGBEasting % 100000L;
}


void OSGBSquareToRefCoords(const char* OSGBGridSquare,
						   int &RefEasting, int &RefNorthing)
{
	int pos, x_multiplier, y_multiplier;
	char GridSquare[] = "VWXYZQRSTULMNOPFGHJKABCDE";

	//find 500km offset
	char ch = OSGBGridSquare[0];
	switch(ch)
	{
		case 'S': x_multiplier = 0; y_multiplier = 0; break;
		case 'T': x_multiplier = 1; y_multiplier = 0; break;
		case 'N': x_multiplier = 0; y_multiplier = 1; break;
		case 'O': x_multiplier = 1; y_multiplier = 1; break;
		case 'H': x_multiplier = 0; y_multiplier = 2; break;
		case 'J': x_multiplier = 1; y_multiplier = 2; break;
	}
	RefEasting = x_multiplier * 500000L;
	RefNorthing = y_multiplier * 500000L;

//find 100km offset and add to 500km offset to get coordinate of 
//square point is in
	pos = strchr(GridSquare, OSGBGridSquare[1]) - GridSquare;
	RefEasting += ((pos % 5) * 100000L);
	RefNorthing += ((pos / 5) * 100000L);

}

void OSGBtoLL(const double OSGBNorthing, const double OSGBEasting, const char* OSGBZone,
			  double& Lat,  double& Long )
{
//converts OSGB coords to lat/long.  Equations from USGS Bulletin 1532 
//East Longitudes are positive, West longitudes are negative. 
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees. 
	//Written by Chuck Gantz- chuck.gantz@globalstar.com

	double k0 = 0.9996012717;
	double a;
	double eccPrimeSquared;
	double N1, T1, C1, R1, D, M;
	double LongOrigin = -2;
	double LatOrigin = 49;
	double LatOriginRad = LatOrigin * deg2rad;
	double mu, phi1, phi1Rad;
	double x, y;
	int RefEasting, RefNorthing;


	double majoraxis = a = 6377563.396;//Airy
	double minoraxis = 6356256.91;//Airy

	double eccSquared = (majoraxis * majoraxis - minoraxis * minoraxis) / (majoraxis * majoraxis);
	double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));

	//only calculate M0 once since it is based on the origin of the OSGB projection, which is fixed
	static double 	M0 = a*((1	- eccSquared/4		- 3*eccSquared*eccSquared/64	- 5*eccSquared*eccSquared*eccSquared/256)*LatOriginRad 
				- (3*eccSquared/8	+ 3*eccSquared*eccSquared/32	+ 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatOriginRad)
									+ (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatOriginRad) 
									- (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatOriginRad));

	OSGBSquareToRefCoords(OSGBZone, RefEasting, RefNorthing);
	x = OSGBEasting - 400000.0 + RefEasting; //remove 400,000 meter false easing for longitude
	y = OSGBNorthing + 100000.0 + RefNorthing; //remove 100,000 meter false easing for longitude

	eccPrimeSquared = (eccSquared)/(1-eccSquared);

	M = M0 + y / k0;
	mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

	phi1Rad = mu	+ (3*e1/2-27*e1*e1*e1/32)*sin(2*mu) 
				+ (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
				+(151*e1*e1*e1/96)*sin(6*mu);
	phi1 = phi1Rad*rad2deg;

	N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
	T1 = tan(phi1Rad)*tan(phi1Rad);
	C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
	R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
	D = x/(N1*k0);

	Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
					+(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
	Lat = Lat * rad2deg;

	Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
					*D*D*D*D*D/120)/cos(phi1Rad);
	Long = LongOrigin + Long * rad2deg;

}

void OSGBGridRefToRefCoords(const char* OSGBpos,
   int &RefEasting, int &RefNorthing)
{
	//Should validate input ...
	string mapref = OSGBpos;

	//Convert to GBOS grid
	string gridSquare = mapref.substr(0,2);
	string coords = mapref.substr(2,mapref.size()-2);
	int coordLen = coords.size();
	int scaleFactor = pow(10,(5 - coordLen / 2));
	string coordsEasting = coords.substr(0,coords.size()/2);
	string coordsNorthing = coords.substr(coords.size()/2,coords.size()-(coords.size()/2));
	int gridEasting = 0, gridNorthing = 0;
	OSGBSquareToRefCoords(gridSquare.c_str(), gridEasting, gridNorthing);
			
	RefEasting = gridEasting + atoi(coordsEasting.c_str()) * scaleFactor;
	RefNorthing = gridNorthing + atoi(coordsNorthing.c_str()) * scaleFactor;
}

//*******************************

void GanzcTransform::ConvertGbos1936ToWgs84(double ea, double no, double he,
	double &latOut, double &lonOut, double &heOut)
{
	char OSGBGridSquare[3];
	long OSGBEasting, OSGBNorthing;
	CoordsToOSGBSquare(ea, no,  
				  OSGBGridSquare, OSGBEasting, OSGBNorthing);

	double Lat;  double Lon;
	OSGBtoLL(OSGBNorthing,OSGBEasting, OSGBGridSquare, Lat,  Lon );

	latOut = Lat;
	lonOut = Lon;
	heOut = 0.0;
}

void GanzcTransform::ConvertWgs84ToGbos1936(double lat, double lon, double he,
	double &eaOut, double &noOut, double &heOut)
{
	double OSGBEasting, OSGBNorthing;
	LLtoOSGBGrid(lat, lon, OSGBEasting, OSGBNorthing);

	eaOut = OSGBEasting;
	noOut = OSGBNorthing;
	heOut = 0.0;
}

