
#ifndef GBOS1936_H
#define GBOS1936_H

#include <string.h>
//Helmert approximation of GBOS1936 to WGS84
//Accurate to 6 metres approx
//Based on that VB Excel OS ProjectionandTransformationCalculations.xls
//Probably based on OS Crown Copyright
//Ported to C++ by Tim Sheerman-Chase 2009

double Helmert_X(double X, double Y, double Z, double DX, double Y_Rot, double Z_Rot, double s);
double Helmert_Y(double X, double Y, double Z, double DY, double X_Rot, double Z_Rot, double s);
double Helmert_Z(double X, double Y, double Z, double DZ,double  X_Rot,double  Y_Rot,double  s);
double XYZ_to_Lat(double X, double Y, double Z, double a, double b);
double XYZ_to_Long(double X,double Y);
double XYZ_to_H(double X,double  Y,double  Z,double  a,double  b);
double Lat_Long_H_to_X(double PHI, double LAM, double H, double a, double b);
double Lat_Long_H_to_Y(double PHI, double LAM, double H, double a, double b);
double Lat_H_to_Z(double PHI, double LAM, double H, double a, double b);
double E_N_to_Lat(double East, double North, double a, double b, double e0, double n0, double f0, double PHI0, double LAM0);
double E_N_to_Long(double East, double North, double a, double b, double e0, double n0, double f0, double PHI0, double LAM0);

void ConvertGbos1936ToWgs84(double ea, double no, double he,
	double &latOut, double &lonOut, double &heOut);
void ConvertWgs84ToGbos1936(double lat, double lon, double he,
	double &eaOut, double &noOut, double &heOut);
int TestGbos1936();

class HelmertConverter
{
public:
	void ConvertGbos1936ToWgs84(double ea, double no, double he,
		double &latOut, double &lonOut, double &heOut);
	void ConvertWgs84ToGbos1936(double lat, double lon, double he,
		double &eaOut, double &noOut, double &heOut);
	void GetTransformName(char *buffer, int maxLen)
	{
		strncpy(buffer, "Helmert Gbos1936", maxLen);
	}
};


#endif //GBOS1936_H


