
//./warp --in /home/tim/osm/hor_common/11011_05.jpg --points 11011_05.csv --out 11011_05 --width 2048 --height 2048 --fit 2 --vis
//./warp --in /home/tim/osm/os/PortCarlisle2small.jpg --points portcar.csv --out portcar --width 2048 --height 2048 --fit 1 --vis
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
#include "TransformPoly.h"
#include "WriteKml.h"
#include "ErrorHandling.h"
#include "ImageWarpByFunc.h"
#include "ReadDelimitedFile.h"
#include "ganzc/LatLong-OSGBconversion.h"
#include "Gbos1936.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

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

//****************************************************

vector<double> PolyProjectWithPtr(vector<double> in, void *userPtr)
{
	vector<double> *poly = (vector<double> *)userPtr;
	vector<double> pout = PolyProject(in, *poly);
	//cout << in[0] << "\t" << in[1] << "\t" << pout[0] << "\t" << pout[1] << endl;
	return pout;
}

void AddPointPoly(class Tile &tile,class PolyProjection &polyEst, double lat, double lon, double x, double y)
{
	//cout << "AddPointPoly " << lat << "," << lon << "," << x << "," << y << endl;
	vector<double> p;
	tile.Project(lat,lon,p);
	polyEst.AddPoint(x,y,p);
}

void SplitGbosRef(string in, string &zone, long &easting, long &northing)
{
	//cout << in << endl;
	zone = in.substr(0,2);
	if(in.length() != 8) ThrowError<logic_error>("Unexpected length of map reference",__LINE__,__FILE__);
	easting = atoi(in.substr(2,3).c_str())*100;
	northing = atoi(in.substr(5,3).c_str())*100;
	//cout << zone << "," << easting << "," << northing << endl;
}

//**************************************************

int main(int argc, char *argv[])
{
	string inputImageFilename = "/home/tim/Desktop/hor_common/11011_01.jpg";
	string inputPointsFilename = "points.csv";
	int polynomialOrder = 5;
	string outputFilename = "map";
	int visualiseErrors = 0;
	int outputWidth = -1;
	int outputHeight = -1;


	/*double Lat = 54.9785 ;
	double Long = -3.1897;
	long OSGBNorthing;
	long OSGBEasting;
	char OSGBZone[4];
	
	cout << "Starting position(Lat, Long):  " << Lat << "   " << Long <<endl;

	LLtoOSGB(Lat, Long, OSGBEasting, OSGBNorthing, OSGBZone);
	cout << setiosflags(ios::showpoint | ios::fixed) << setprecision(6);
	cout << "Calculated OSGB position(Northing, Easting, GridSquare):  ";
	cout << OSGBZone << " " << OSGBEasting << " " << OSGBNorthing << endl;
	
	OSGBtoLL(OSGBNorthing, OSGBEasting, OSGBZone, Lat, Long);
	cout << "Calculated Lat, Long position(Lat, Long):  " << Lat << "   " << Long << endl <<endl;*/

	//Process program options using boost library
	po::variables_map vm;
	po::options_description desc("Allowed options");
	try{
	desc.add_options() ("in,i", po::value<string>(), "input image filename")
		("points,p",po::value<string>(),"points to define transformation")
		("out,o",po::value<string>(),"output name (extension is added automatically)")
		("vis,v","visualisation of error")
		("fit,f",po::value<int>(),"order of polynomial")
		("width,w",po::value<int>(),"output width")
		("height,h",po::value<int>(),"output height")
		("help","help message");

		//("annot-offset",po::value<double>(),"time offset of anvil annotation track")
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {cout << desc << endl; exit(0);}
	if(vm.count("in")) inputImageFilename = vm["in"].as<string>();
	if(vm.count("points")) inputPointsFilename = vm["points"].as<string>();
	if(vm.count("out")) outputFilename = vm["out"].as<string>();
	if(vm.count("vis")) visualiseErrors = 1;
	if(vm.count("fit")) polynomialOrder = vm["fit"].as<int>();
	if(vm.count("width")) outputWidth = vm["width"].as<int>();
	if(vm.count("height")) outputHeight = vm["height"].as<int>();

	//Check basic parameters are set
	//if(inputFilename.size() == 0) 
	//{cout << "Error: input sequence must be specified" << endl << desc << endl; exit(0);}

	}
	catch(exception &e)
	{
		cerr << "error: " << e.what() << endl;
	}

	/*CImg<unsigned char> img;
	cimg::exception_mode()=0;
	try{
	//img.load("/home/tim/Desktop/hor_common/11011_01thumb.jpg");
	img.load(inputImageFilename.c_str());
	//img.resize(img.dimx(),img.dimy(),1,3);
	}
	catch(cimg_library::CImgIOException e)
	{
		cout << desc << endl;
		exit(-1);
	}
	cimg::exception_mode()=1;*/
	class ImgMagick img;
	int openRet = img.Open(inputImageFilename.c_str());
	if(openRet < 0) {cout << "open " << inputImageFilename << " failed" << endl; exit(-1);}

	if(outputWidth == -1) outputWidth = img.GetWidth();
	if(outputHeight == -1) outputHeight = img.GetHeight();
	class Tile tile;
	tile.latmin = 0.29;
	tile.latmax = 1.32;
	tile.lonmin = 0.60;
	tile.lonmax = 0.65;
	tile.sx=outputWidth;
	tile.sy=outputHeight;

	class DelimitedFile pointDef;
	if(pointDef.Open(inputPointsFilename.c_str()) < 0)
	{
		cout << "File not found" << endl;
		cout << desc << endl;
		exit(-1);
	}
	
	//Read points to fix max extend to get a rough tile fit
	double north=0.0, south=0.0, east=0.0, west=0.0; int setBox = 0;
	for(unsigned int i=0;i<pointDef.NumLines();i++)
	{
		class DelimitedFileLine &line = pointDef[i];
		//cout << line.NumVals() << endl;

		if(line.NumVals()==5)
		{
			if(strcmp(line[0].GetVals(),"p")==0)
			{
			double ix = line[1].GetVald();
			double iy = line[2].GetVald();
			if(ix < south || !setBox) south = ix;
			if(ix > north || !setBox) north = ix;
			if(iy < west || !setBox) west = iy;
			if(iy > east || !setBox) east = iy;
			setBox = 1;
			}
		}
		if(line.NumVals()==4) //Read in GB OS national grid data
		{
			if(strcmp(line[0].GetVals(),"os")==0)
			{
			string mapref = line[1].GetVals();

			string zone; long easting=0.0, northing=0.0;
			SplitGbosRef(mapref, zone, easting, northing);
			double lat=-1.0, lon=-1.0;
			OSGBtoLL(northing, easting, zone.c_str(), lat, lon);

			if(lat < south || !setBox) south = lat;
			if(lat > north || !setBox) north = lat;
			if(lon < west || !setBox) west = lon;
			if(lon > east || !setBox) east = lon;
			setBox = 1;			
			}
		}

		if(line.NumVals()==5) //Read in GB OS national grid data
		{	
			if(strcmp(line[0].GetVals(),"gbos1936")==0)
			{
			string easting = line[1].GetVals();
			string northing = line[2].GetVals();

			double lat=-1.0, lon=-1.0, alt = -1.0;
			ConvertGbos1936ToWgs84(atof(easting.c_str()), atof(northing.c_str()),0.0, lat, lon, alt);

			if(lat < south || !setBox) south = lat;
			if(lat > north || !setBox) north = lat;
			if(lon < west || !setBox) west = lon;
			if(lon > east || !setBox) east = lon;
			setBox = 1;			
			}
		}

	}
	cout << "Approx bounding box " << north << "," << south << "," << east << "," << west << endl;
	if(setBox)
	{
		tile.latmin = south;
		tile.latmax = north;
		tile.lonmin = west;
		tile.lonmax = east;
	}

	class PolyProjection polyEst;
	int imageCoordSizex = -1, imageCoordSizey = -1;
	
	//Read points from delimited file into transformation estimator
	for(unsigned int i=0;i<pointDef.NumLines();i++)
	{
		class DelimitedFileLine &line = pointDef[i];
		if(line.NumVals()>=1)
		{
			if(strcmp(line[0].GetVals(), "imagesize")==0 && line.NumVals()>=3)
			{
				imageCoordSizex = line[1].GetVali();
				imageCoordSizey = line[2].GetVali();
				cout << "Read image size is " << imageCoordSizex << "," << imageCoordSizey << endl;
			}
		}
		
		if(line.NumVals()==5) //Read in WGS84 lat lon
		{
			if(strcmp(line[0].GetVals(),"p")==0)
			{
			if(imageCoordSizex >= 1 && imageCoordSizey >= 1)
			{
				double ix = line[1].GetVald();
				double iy = line[2].GetVald();
				double ox = img.GetWidth()*line[3].GetVald()/(double)imageCoordSizex;
				double oy = img.GetHeight()*line[4].GetVald()/(double)imageCoordSizey;
				//cout << ix << "," << iy << "," << ox << "," << oy << endl;
				AddPointPoly(tile, polyEst,ix,iy,ox,oy);
			}
			else
			{
				AddPointPoly(tile, polyEst, line[1].GetVald(),line[2].GetVald(), 
					line[3].GetVald(),line[4].GetVald());		
			}
			}
		}

		if(line.NumVals()==4) //Read in GB OS national grid data
		{
			if(strcmp(line[0].GetVals(),"os")==0)
			{
			string mapref = line[1].GetVals();

			string zone; long easting=0.0, northing=0.0;
			SplitGbosRef(mapref, zone, easting, northing);
			double lat=-1.0, lon=-1.0;
			OSGBtoLL(northing, easting, zone.c_str(), lat, lon);
			cout << mapref << " -> " << lat << "\t" << lon << endl;

			if(imageCoordSizex >= 1 && imageCoordSizey >= 1)
			{
				
				double ox = img.GetWidth()*line[2].GetVald()/(double)imageCoordSizex;
				double oy = img.GetHeight()*line[3].GetVald()/(double)imageCoordSizey;
				//cout << ix << "," << iy << "," << ox << "," << oy << endl;
				AddPointPoly(tile, polyEst,lat,lon,ox,oy);
			}
			else
			{
				AddPointPoly(tile, polyEst, lat,lon, 
					line[2].GetVald(),line[3].GetVald());		
			}
			}
		}

		if(line.NumVals()==5) //Read in GB OS national grid data
		{
			if(strcmp(line[0].GetVals(),"gbos1936")!=0) continue;
			string easting = line[1].GetVals();
			string northing = line[2].GetVals();

			double lat=-1.0, lon=-1.0, alt = -1.0;
			ConvertGbos1936ToWgs84(atof(easting.c_str()), atof(northing.c_str()),0.0, lat, lon, alt);

			if(imageCoordSizex >= 1 && imageCoordSizey >= 1)
			{
				
				double ox = img.GetWidth()*line[3].GetVald()/(double)imageCoordSizex;
				double oy = img.GetHeight()*line[4].GetVald()/(double)imageCoordSizey;
				//cout << ix << "," << iy << "," << ox << "," << oy << endl;
				AddPointPoly(tile, polyEst,lat,lon,ox,oy);
			}
			else
			{
				AddPointPoly(tile, polyEst, lat,lon, 
					line[3].GetVald(),line[4].GetVald());		
			}		
		}

	}

	polyEst.order = polynomialOrder;
	vector<double> poly = polyEst.Estimate();

	//Calculate transformed image size (currently not used)
	double warpMinLat=0.0, warpMaxLat=0.0, warpMinLon=0.0, warpMaxLon=0.0;
	int warpBoxSet = 0;
	for(int i=0;i<100;i++)
	for(int j=0;j<100;j++)
	{
		vector<double>p;
		p.push_back((double)i * (double)img.GetWidth() / (double)(100-1));
 		p.push_back((double)j * (double)img.GetHeight() / (double)(100-1));
		vector<double> pt = PolyProjectWithPtr(p,(void *)&poly);
		double lat=-1.0, lon=-1.0;
		tile.UnProject(pt[0],pt[1],lat,lon);
		//cout << lat << "," << lon << endl;

		if(lat < warpMinLat || !warpBoxSet)warpMinLat = lat;
		if(lat > warpMaxLat || !warpBoxSet)warpMaxLat = lat;
		if(lon < warpMinLon || !warpBoxSet)warpMinLon = lon;
		if(lon > warpMaxLon || !warpBoxSet)warpMaxLon = lon;
		warpBoxSet = 1;
	}

	class ImgMagick endImage;
	endImage.SetNumChannels(3);
	endImage.SetWidth(tile.sx);
	endImage.SetHeight(tile.sy);

	//Transform image
	class ImageWarpByFunc imageWarpByFunc;
	imageWarpByFunc.xsize= 100;
	imageWarpByFunc.ysize= 100;
	imageWarpByFunc.Warp(img, endImage, PolyProjectWithPtr, (void *)&poly);

	/*if(visualiseErrors)
	for(unsigned int i=0;i<polyEst.transformedPoints.size();i++)
	{
		unsigned char col[3] = {255,0,0};
		//cout << polyEst.transformedPoints[i][0] << "," << polyEst.transformedPoints[i][1] << endl;
		endImage.draw_circle(polyEst.transformedPoints[i][0],polyEst.transformedPoints[i][1],3,col);

		vector<double> proj = PolyProject(polyEst.originalPoints[i], poly);
		unsigned char col2[3] = {0,0,255};
		endImage.draw_circle(proj[0],proj[1],3,col2);
		endImage.draw_line(polyEst.transformedPoints[i][0],polyEst.transformedPoints[i][1], proj[0],proj[1], col2);
	}*/

	string mapOutFilename = outputFilename + ".png";
	endImage.Save(mapOutFilename.c_str());

	class WriteKml writeKml;
	writeKml.north = tile.latmax;
	writeKml.south = tile.latmin;
	writeKml.west = tile.lonmin;
	writeKml.east = tile.lonmax;
	string kmlOutFilename = outputFilename + ".kml";
	writeKml.href = mapOutFilename;
	cout << "Writing KML to " << kmlOutFilename << endl;
	writeKml.WriteToFile(kmlOutFilename.c_str());
	
}


