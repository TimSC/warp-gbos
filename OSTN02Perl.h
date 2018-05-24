
#ifndef OSTN02_PERL_H
#define OSTN02_PERL_H

//Interface to Geo-Coordinates-OSGB-2.04 (in perl)

#include <iostream>
using namespace std;
#include "StringUtils.h"
#include <string.h>

class OSTN02Perl
{
public:
	struct interpreter *my_perl;

	OSTN02Perl();
	virtual ~OSTN02Perl();

	void ConvertGbos1936ToWgs84(double ea, double no, double he,
		double &latOut, double &lonOut, double &heOut);
	void ConvertWgs84ToGbos1936(double lat, double lon, double he,
		double &eaOut, double &noOut, double &heOut);
	void GetTransformName(char *buffer, int maxLen)
	{
		strncpy(buffer, "OSTN02 via Geo-Coordinates-OSGB", maxLen);
	}
};

#endif //OSTN02_PERL_H

