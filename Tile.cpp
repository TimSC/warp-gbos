
#include "Tile.h"

Tile::Tile()
{
	latmin=0.0;latmax=1.0;lonmin=0.0;lonmax=1.0;
	sx=256;sy=256;
}

Tile::~Tile()
{

}

void Tile::Project(double lat, double lon, double &ixOut, double &iyOut)
{
	iyOut = (double)sy - (double)sy * (lat - latmin) / (latmax - latmin);
	ixOut = (double)sx * (lon - lonmin) / (lonmax - lonmin);
	//cout << lat << "," << lon << "," << ixOut << "," << iyOut << endl; 
}

void Tile::Project(double lat, double lon, vector<double> &out)
{
	double iy, ix;
	Project(lat,lon,ix,iy);
	out.clear();
	out.push_back(ix);
	out.push_back(iy);
}

void Tile::UnProject(double x, double y, double &latOut, double &lonOut)
{
	latOut = ((double)sy - y) * (latmax - latmin) / (double)sy  + latmin;
	lonOut = x * (lonmax - lonmin) / (double)sx + lonmin;
}
