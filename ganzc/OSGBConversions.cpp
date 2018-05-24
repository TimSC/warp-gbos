//OSGB Conversion.cpp- test program for lat/long to OSGB and OSGB to lat/long conversions
#include <iostream.h>
#include <iomanip.h>
#include "LatLong-OSGBconversion.h"


void main()
{
//	double Lat = 54.515278;
//	double Long = -1.465486;
//	double Lat = 56.4564444;
//	double Long = -2.9851663;
//	double Lat = 52.201667;
//	double Long = 0.1166667;
//	double Lat = 49;
//	double Long = -2;
	double Lat = 52.701;
	double Long = 3.001003;
	long OSGBNorthing;
	long OSGBEasting;
	char OSGBZone[4];
	
	cout << "Starting position(Lat, Long):  " << Lat << "   " << Long <<endl;

	LLtoOSGB(Lat, Long, OSGBEasting, OSGBNorthing, OSGBZone);
	cout << setiosflags(ios::showpoint | ios::fixed) << setprecision(6);
	cout << "Calculated OSGB position(Northing, Easting, GridSquare):  ";
	cout << OSGBZone << " " << OSGBEasting << " " << OSGBNorthing << endl;
	
	OSGBtoLL(OSGBNorthing, OSGBEasting, OSGBZone, Lat, Long);
	cout << "Calculated Lat, Long position(Lat, Long):  " << Lat << "   " << Long << endl <<endl;
}


