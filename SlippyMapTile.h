
#ifndef SLIPPY_MAP_TILE_H
#define SLIPPY_MAP_TILE_H

#include <math.h>

//Source: http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
//Tiles are 256 × 256 pixel PNG files 

int long2tile(double lon, int z) 
{ 
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z))); 
}

int lat2tile(double lat, int z)
{ 
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z))); 
}

double tile2long(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}

double tile2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

#endif //SLIPPY_MAP_TILE_H

