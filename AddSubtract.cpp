
#include "OSTN02Perl.h"

int main(int argc, char **argv, char **env)
{


	class OSTN02Perl converter;
	double lat,lon;
	converter.ConvertGbos1936ToWgs84(173000,610000,0.0,lat,lon);
	cout << lat << "," << lon << endl;

	double e,n,h;
	converter.ConvertWgs84ToGbos1936(55.331496581601,-5.58040717725593,0.0,e,n,h);
	cout << e << "," << n << "," << h << endl;	
}

