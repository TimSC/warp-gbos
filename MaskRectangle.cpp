
#include "ganzc/LatLong-OSGBconversion.h"
#include "Gbos1936.h"
#include <string>
#include <iostream>
using namespace std;

void UpdateBoundingBox(string &mapref, int &gnorth, int &gsouth, int &geast, int &gwest, int &boxset)
{
	int dEasting=0, dNorthing=0;
	OSGBGridRefToRefCoords(mapref.c_str(),dEasting,dNorthing);

	if(!boxset || gsouth > dNorthing) gsouth = dNorthing;
	if(!boxset || gnorth < dNorthing) gnorth = dNorthing;
	if(!boxset || geast < dEasting) geast = dEasting;
	if(!boxset || gwest > dEasting) gwest = dEasting;
	boxset = 1;
	//double lat=-1.0, lon=-1.0, alt = -1.0;
	//ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
}

int CheckIfInBox(double lat, double lon, double gnorth, double gsouth, double geast, double gwest)
{
	double pnorth,peast,palt;
	ConvertWgs84ToGbos1936(lat,lon, 0.0, peast,pnorth,palt);
	if(pnorth < gsouth) return 0;
	if(pnorth > gnorth) return 0;
	if(peast < gwest) return 0;
	if(peast > geast) return 0;

	return 1;
}

int main()
{
	double north = 55.2446;
	double south = 54.8333;
	double east = -3.04477;
	double west = -3.68423;

	string corner1 = "NX930950";
	string corner2 = "NY330500";
	
	int gnorth = 0, gsouth = 0, geast = 0, gwest = 0, boxset = 0;
	UpdateBoundingBox(corner1, gnorth, gsouth, geast, gwest, boxset);
	UpdateBoundingBox(corner2, gnorth, gsouth, geast, gwest, boxset);

	cout << gnorth<<","<< gsouth<< "\t" << geast<<","<< gwest << endl;
	
	cout << CheckIfInBox(55.2446, -3.04477, gnorth, gsouth, geast, gwest) << endl;
	cout << CheckIfInBox(54.9, -3.3, gnorth, gsouth, geast, gwest) << endl;

	/*double pnorth,peast,palt;
	ConvertWgs84ToGbos1936(south,west, 0.0, pnorth,peast,palt);
	cout << pnorth<<","<<peast<<","<<palt << endl;*/


	/*double cornerAlat,cornerAlon,cornerAalt;
	ConvertGbos1936ToWgs84(gwest, gsouth, 0.0, cornerAlat,cornerAlon,cornerAalt);
	cout << cornerAlat << "," << cornerAlon << "," << cornerAalt << endl;

	double cornerBlat,cornerBlon,cornerBalt;
	ConvertGbos1936ToWgs84(gwest, gsouth, 0.0, cornerBlat,cornerBlon,cornerBalt);
	cout << cornerBlat << "," << cornerBlon << "," << cornerBalt << endl;	*/

	//cout << "tests" << endl;
	//TestGbos1936();
}
