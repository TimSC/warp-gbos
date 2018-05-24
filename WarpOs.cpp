
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
#include "ganzc/LatLong-OSGBconversion.h" //Poor accuracy?
#include "gbos1936/Gbos1936.h"
#ifdef WITH_OSTN02_PERL
#include "OSTN02Perl.h"
#endif //WITH_OSTN02_PERL
#ifdef WITH_OSTN02_PYTHON
#include "PyOstn02.h"
#endif //WITH_OSTN02_PYTHON
#include "Tile.h"
#include <boost/program_options.hpp>
#include <math.h>
#include "StringUtils.h"
namespace po = boost::program_options;

//class HelmertConverter gConverter;
#ifdef WITH_OSTN02_PYTHON
class PyOstn02 gConverter;
#else
#ifdef WITH_OSTN02_PERL
class OSTN02Perl gConverter;
#else
class HelmertConverter gConverter;
#endif //WITH_OSTN02_PERL
#endif //WITH_OSTN02_PYTHON


class HelmertConverter gFallbackConverter;

//****************************************************

class PolyProjectArgs
{
public:
	vector<double> imgToRefPoly;
	class Tile *ptile;
	int refSystemOsgb;
	int mercatorOut;
	int order;

	PolyProjectArgs()
	{
		ptile = 0;
		refSystemOsgb = 1;
		mercatorOut = 0;
		order = 2;
	}
};

void ConvertCoord(double latin,double lonin,double altin,double &latout,double &lonout,double &altout)
{
	try
	{
		gConverter.ConvertGbos1936ToWgs84(latin, lonin,altin, latout, lonout, altout);
	}
	catch(int e)
	{
		cout << "Using HelmertConverter as fallback" << endl;
		gFallbackConverter.ConvertGbos1936ToWgs84(latin, lonin,altin, latout, lonout, altout);
	}
}

vector<double> ProjRefToOutImg(vector<double> ref, int refSystemOsgb, class Tile &tile, void *userPtr)
{
	class PolyProjectArgs *args = (class PolyProjectArgs *)userPtr;
	double lat=0.0, lon=0.0, alt=0.0;
	if(refSystemOsgb && args->mercatorOut)
	{
		//cout << "1 " << ref[0]<< "," << ref[1] << endl;
		ConvertCoord(ref[0], ref[1],0.0, lat, lon, alt);
		/*try
		{
		gConverter.ConvertGbos1936ToWgs84(ref[0], ref[1],0.0, lat, lon, alt);
		}
		catch(int e)
		{
			cout << "Using HelmertConverter as fallback" << endl;
			gFallbackConverter.ConvertGbos1936ToWgs84(ref[0], ref[1],0.0, lat, lon, alt);
		}*/
	}
	if(!refSystemOsgb && args->mercatorOut)
	{
		lat = ref[0]; 
		lon = ref[1]; 
		alt = 0.0;
		//cout << "2\t" << lat << "\t" << lon << endl;
	}
	if(refSystemOsgb && !args->mercatorOut)
	{
		lat = ref[1]; 
		lon = ref[0]; 
		alt = 0.0;
		//cout << "3\t" << lat << "\t" << lon << endl;
	}
	if(!refSystemOsgb && !args->mercatorOut)
	{
		cout << "Error: not implemented" << endl;
		exit(0);
	}

	vector<double> pout;
	tile.Project(lat, lon, pout);
	//if(ref[0] >= 333000 && ref[1] <= 550000)
	{
		//cout << "corner " << ref[0] << "," << ref[1] << "\t" << pout[0] << "," << pout[1] << endl;
	}
	
	return pout;
}

vector<double> PolyProjectWithPtr(vector<double> in, void *userPtr)
{
	class PolyProjectArgs *args = (class PolyProjectArgs *)userPtr;
	vector<double> ref = PolyProject(in, args->imgToRefPoly, args->order);
	if(!args->ptile) throw(0);
	/*double lat=0.0, lon=0.0, alt=0.0;
	ConvertGbos1936ToWgs84(ref[0], ref[1],0.0, lat, lon, alt);
	vector<double> pout;
	//cout << args->ptile->latmin << "\t" << args->ptile->latmax << "\t" << args->ptile->sy << endl;

	args->ptile->Project(lat, lon, pout);
	//cout << ref[0] << "\t" << ref[1] << "\t" << pout[0] << "\t" << pout[1] << endl;
	return pout;*/
	return ProjRefToOutImg(ref, args->refSystemOsgb, *args->ptile, userPtr);

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

void DrawCross(class ImgFrameBase &img, int x, int y, double r, double g, double b)
{
	cout << x << "," << y << endl;
	for(int i=-10;i<=10;i++)
	for(int j=-10;j<=10;j++)
	{
	img.SetPix(x+i,y+j,0,r);
	img.SetPix(x+i,y+j,1,g);
	img.SetPix(x+i,y+j,2,b);
	}
}

//**************************************************

int main(int argc, char *argv[])
{
	string inputImageFilename = "";
	string inputPointsFilename = "os7sheet075.csv";
	int polynomialOrder = -1; //-1 is auto select
	string outputFilename = "map";
	int visualiseErrors = 0;
	int outputWidth = -1;
	int outputHeight = -1;
	int fitOnly = 0;
	int gbosOut = 0;
	int mercatorOut = 1;
	int gbosIn = 1;
	int mercatorIn = 0;
	int forceAspectCoord = 0;
	string outproj = "mercator";
	string inproj = "gbos";
	vector<string> corners;
	
	//Print name of transform to screen
	char transformName[100];
	gConverter.GetTransformName(transformName, 100);
	transformName[99] = '\0';
	cout << "Using transform " << transformName << endl;

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
		("fitonly","calc transform only (no rectify)")
		("aspect","ensure output aspect ratio matches coordinate distances")
		("projin",po::value<string>(),"input projection (mercator, gbos)")
		("projout",po::value<string>(),"output projection (mercator, gbos)")
		("fit,f",po::value<int>(),"order of polynomial")
		("width,w",po::value<int>(),"output width")
		("height,h",po::value<int>(),"output height")
		("corner",po::value<vector<string> >(),"override map corners of final map")
		("help","help message");

		//("annot-offset",po::value<double>(),"time offset of anvil annotation track")
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	/*cout << vm.count("help") << "," << (vm.find("in")!=vm.end()) << endl;
	for(po::variables_map::iterator it = vm.begin();it != vm.end(); it++)
	{
		cout << "in variable map:" << it->first << endl;
	}*/

	if(vm.count("help")) {cout << desc << endl; exit(0);}
	if(vm.count("in")) {inputImageFilename = vm["in"].as<string>();}
	if(vm.count("points")) inputPointsFilename = vm["points"].as<string>();
	if(vm.count("out")) outputFilename = vm["out"].as<string>();
	if(vm.count("vis")) visualiseErrors = 1;
	if(vm.count("fit")) polynomialOrder = vm["fit"].as<int>();
	if(vm.count("width")) outputWidth = vm["width"].as<int>();
	if(vm.count("height")) outputHeight = vm["height"].as<int>();
	if(vm.count("aspect")) forceAspectCoord = 1;
	if(vm.count("fitonly")) fitOnly = 1;
	if(vm.count("corner")) corners = vm["corner"].as<vector<string> >();
	if(vm.count("outproj"))
	{
		outproj = vm["outproj"].as<string>();
		if (outproj == "gbos") {gbosOut = 1;mercatorOut=0;}
	}
	if(vm.count("inproj"))
	{
		inproj = vm["inproj"].as<string>();
		if (inproj == "marcator") {gbosIn = 0;mercatorIn=1;}
	}

	if(inputImageFilename.length() == 0) {cout << "Input image not defined." << endl; cout << desc << endl; exit(0);}

	//Check basic parameters are set
	//if(inputFilename.size() == 0) 
	//{cout << "Error: input sequence must be specified" << endl << desc << endl; exit(0);}

	}
	catch(exception &e)
	{
		cerr << "error: " << e.what() << endl;
	}

	class Tile tile;
	cout << "Loading image..." << endl;
	class ImgMagick img;
	int openRet = img.Open(inputImageFilename.c_str());
	if(openRet < 0) {cout << "open " << inputImageFilename << " failed" << endl; exit(-1);}
	if(outputWidth == -1) outputWidth = img.GetWidth();
	if(outputHeight == -1) outputHeight = img.GetHeight();
	tile.sx=outputWidth;
	tile.sy=outputHeight;

	class DelimitedFile pointDef;
	if(pointDef.Open(inputPointsFilename.c_str()) < 0)
	{
		cout << "File not found" << endl;
		cout << desc << endl;
		exit(-1);
	}
	
	//Read points
	double north=0.0, south=0.0, east=0.0, west=0.0; int setBox = 0;
	class PolyProjection srcImgToRef;

	for(unsigned int i=0;i<pointDef.NumLines();i++)
	{
		class DelimitedFileLine &line = pointDef[i];
		//cout << line.NumVals() << endl;

		if(line.NumVals()==5)
		{
			if(strcmp(line[0].GetVals(),"p")==0)
			{
			double imgX = line[3].GetVald();
			double imgY = line[4].GetVald();
			double ix = line[1].GetVald();
			double iy = line[2].GetVald();


			if(ix < south || !setBox) south = ix;
			if(ix > north || !setBox) north = ix;
			if(iy < west || !setBox) west = iy;
			if(iy > east || !setBox) east = iy;
			
			if(gbosIn)
			{
				cout << "Cannot take lat lon input when using OS as input projection" << endl;	
				exit(0);
			}
			else
				srcImgToRef.AddPoint(imgX,imgY,ix,iy);

			setBox = 1;
			}
		}
		if(line.NumVals()==4) //Read in GB OS national grid data
		{
			if(strcmp(line[0].GetVals(),"os")==0)
			{
			string mapref = line[1].GetVals();
			double imgX = line[2].GetVald();
			double imgY = line[3].GetVald();

			/*string zone; long easting=0.0, northing=0.0;
			SplitGbosRef(mapref, zone, easting, northing);
			srcImgToRef.AddPoint(imgX,imgY,easting,northing);

			double lat=-1.0, lon=-1.0;
			OSGBtoLL(northing, easting, zone.c_str(), lat, lon); //Poor accuracy?*/

			//Convert to GBOS grid
			/*string gridSquare = mapref.substr(0,2);
			string coords = mapref.substr(2,mapref.size()-2);
			int coordLen = coords.size();
			int scaleFactor = pow(10,(5 - coordLen / 2));
			string coordsEasting = coords.substr(0,coords.size()/2);
			string coordsNorthing = coords.substr(coords.size()/2,coords.size()-(coords.size()/2));
			int gridEasting = 0, gridNorthing = 0;
			OSGBSquareToRefCoords(gridSquare.c_str(), gridEasting, gridNorthing);
			
			double dEasting = gridEasting + atoi(coordsEasting.c_str()) * scaleFactor;
			double dNorthing = gridNorthing + atoi(coordsNorthing.c_str()) * scaleFactor;*/

			int dEasting=0, dNorthing=0;
			OSGBGridRefToRefCoords(mapref.c_str(),dEasting,dNorthing);

			//Add point to transform constraints
			double lat=-1.0, lon=-1.0, alt = -1.0;
			//cout << "x " << mapref << endl;
			//gConverter.ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
			ConvertCoord(dEasting, dNorthing,0.0, lat, lon, alt);
			//cout << "y " << dEasting<< "," << dNorthing << "->"<<lat<< "," << lon << endl;
			
			if(gbosIn)
				srcImgToRef.AddPoint(imgX,imgY,dEasting,dNorthing);
			else
				srcImgToRef.AddPoint(imgX,imgY,lat,lon);

			if(mercatorOut)
			{
				if(lat < south || !setBox) south = lat;
				if(lat > north || !setBox) north = lat;
				if(lon < west || !setBox) west = lon;
				if(lon > east || !setBox) east = lon;
				setBox = 1;
			}
			if(gbosOut)
			{
				if(dNorthing < south || !setBox) south = dNorthing;
				if(dNorthing > north || !setBox) north = dNorthing;
				if(dEasting < west || !setBox) west = dEasting;
				if(dEasting > east || !setBox) east = dEasting;
				setBox = 1;
			}
			}
		}

		if(line.NumVals()==5) //Read in GB OS national grid data
		{	
			if(strcmp(line[0].GetVals(),"gbos1936")==0)
			{
			double dEasting = line[1].GetVald();
			double dNorthing = line[2].GetVald();
			double imgX = line[3].GetVald();
			double imgY = line[4].GetVald();

			double lat=-1.0, lon=-1.0, alt = -1.0;
			//gConverter.ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
			ConvertCoord(dEasting, dNorthing,0.0, lat, lon, alt);

			cout << "conv " << dEasting<< "," << dNorthing << "->"<<lat<< "," << lon << endl;

			//Add point to transform constraints
			if(gbosIn)
				srcImgToRef.AddPoint(imgX,imgY,dEasting,dNorthing);
			else
				srcImgToRef.AddPoint(imgX,imgY,lat,lon);

			/*char gridSquare[3]; long OSGBEasting=0, OSGBNorthing=0;
			CoordsToOSGBSquare(dEasting, dNorthing,  
				  gridSquare, OSGBEasting, OSGBNorthing);
			cout << gridSquare << OSGBEasting << "," << OSGBNorthing << endl;*/

			if(mercatorOut)
			{
				if(lat < south || !setBox) south = lat;
				if(lat > north || !setBox) north = lat;
				if(lon < west || !setBox) west = lon;
				if(lon > east || !setBox) east = lon;
				setBox = 1;
			}
			if(gbosOut)
			{
				if(dNorthing < south || !setBox) south = dNorthing;
				if(dNorthing > north || !setBox) north = dNorthing;
				if(dEasting < west || !setBox) west = dEasting;
				if(dEasting > east || !setBox) east = dEasting;
				setBox = 1;
			}
			//cout << "gbos " << dEasting << "," << dNorthing << "\t" << lat << "," << lon << endl;
			
			}
		}

	}

	if(corners.size()==0)
	{
	cout << "Approx bounding box N " << north << ", S " << south << ", E " << east << ", W " << west << endl;
	}
	else
	{
		setBox = 0;
		for(unsigned int i=0;i<corners.size();i++)
		{
			int dEasting=0, dNorthing=0;
			OSGBGridRefToRefCoords(corners[i].c_str(),dEasting,dNorthing);

			//Add point to transform constraints
			double lat=-1.0, lon=-1.0, alt = -1.0;
			//gConverter.ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
			ConvertCoord(dEasting, dNorthing,0.0, lat, lon, alt);

			if(mercatorOut)
			{
				if(lat < south || !setBox) south = lat;
				if(lat > north || !setBox) north = lat;
				if(lon < west || !setBox) west = lon;
				if(lon > east || !setBox) east = lon;
				setBox = 1;
			}
			if(gbosOut)
			{
				if(dNorthing < south || !setBox) south = dNorthing;
				if(dNorthing > north || !setBox) north = dNorthing;
				if(dEasting < west || !setBox) west = dEasting;
				if(dEasting > east || !setBox) east = dEasting;
				setBox = 1;
			}
			setBox = 1;
		}
		cout << "Manually set boundary " << north << "," << south << "," << east << "," << west << endl;
	}
	tile.latmin = south;
	tile.latmax = north;
	tile.lonmin = west;
	tile.lonmax = east;	

	//Auto determine polynomial order
	if(polynomialOrder == -1)
	{
		polynomialOrder = CalcOrderFitForNumConstraints(2*srcImgToRef.originalPoints.size());
		cout << "Using " << polynomialOrder << "th order polynomial ("<<srcImgToRef.originalPoints.size()<<" points)" << endl;
		cout << "Need to determine " << CoeffSize(polynomialOrder) << " coeffs" << endl;
	}

	srcImgToRef.order = polynomialOrder;
	vector<double> poly = srcImgToRef.Estimate();

	double coordWidth = tile.lonmax - tile.lonmin;
	double coordHeight = tile.latmax - tile.latmin;
	//Get aspect ratio based on coordinates
	if(forceAspectCoord)
	{	
		if ( tile.sx < coordWidth * tile.sy / coordHeight) tile.sx = coordWidth * tile.sy / coordHeight;
		if ( tile.sy < coordHeight * tile.sx / coordWidth) tile.sy = coordHeight * tile.sx / coordWidth;
	}
	cout << "Output image size " << tile.sx << " by " << tile.sy << " px" << endl;
	cout << "Output coordinates size " << coordWidth << " by " << coordHeight << endl;

	if(fitOnly) return 0;

	class ImgMagick endImage;
	endImage.SetNumChannels(3);
	endImage.SetWidth(tile.sx);
	endImage.SetHeight(tile.sy);

	//vector<double> test;
	//test.push_back(333000);
	//test.push_back(550000);
	//vector<double> test2 = ProjRefToOutImg(test, tile);
	//cout << test2[0] << "," << test2[1] << endl;
	//exit(0);

	//Transform image
	class ImageWarpByFunc imageWarpByFunc;
	imageWarpByFunc.xsize= 100;
	imageWarpByFunc.ysize= 100;
	class PolyProjectArgs args;
	args.imgToRefPoly = poly;
	args.refSystemOsgb = gbosIn;
	args.ptile = &tile;
	args.order = polynomialOrder;
	args.mercatorOut = mercatorOut;
	cout << "Warping image..." << endl;
	imageWarpByFunc.Warp(img, endImage, PolyProjectWithPtr, (void *)&args);

	if(visualiseErrors)
	for(unsigned int i=0;i<srcImgToRef.transformedPoints.size();i++)
	{
		//unsigned char col[3] = {255,0,0};
		//cout << polyEst.transformedPoints[i][0] << "," << polyEst.transformedPoints[i][1] << endl;
		DrawCross(endImage,srcImgToRef.transformedPoints[i][0], srcImgToRef.transformedPoints[i][1], 255,0,0);

		//endImage.draw_circle(polyEst.transformedPoints[i][0],polyEst.transformedPoints[i][1],3,col);

		//vector<double> proj = ProjRefToOutImg(polyEst.originalPoints[i], tile);
		DrawCross(endImage,srcImgToRef.originalPoints[i][0], srcImgToRef.originalPoints[i][1], 0,0,255);
	}

	cout << "Saving image..." << endl;
	string mapOutFilename = outputFilename + ".jpg";
	endImage.Save(mapOutFilename.c_str());

	string mapOutFileNoPath; //Don't save the path into the kml output
	mapOutFileNoPath = RemoveFilePath(mapOutFilename);

	if(mercatorOut)
	{
		class WriteKml writeKml;
		writeKml.north = tile.latmax;
		writeKml.south = tile.latmin;
		writeKml.west = tile.lonmin;
		writeKml.east = tile.lonmax;
		string kmlOutFilename = outputFilename + ".kml";
		writeKml.href = mapOutFileNoPath;
		cout << "Writing KML to " << kmlOutFilename << endl;
		writeKml.WriteToFile(kmlOutFilename.c_str());
	}
}


