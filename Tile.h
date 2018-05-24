
#ifndef TILE_H
#define TILE_H

#include <vector>
using namespace std;

class Tile
{
public:
	double latmin, latmax, lonmin,lonmax;
	int sx, sy;
	Tile();
	virtual ~Tile();
	void Project(double lat, double lon, double &ixOut, double &iyOut);
	void Project(double lat, double lon, vector<double> &out);
	void UnProject(double x, double y, double &latOut, double &lonOut);
};

#endif ///TILE_H

