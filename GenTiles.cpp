
#include "SlippyMapTile.h"
#include "gbos1936/Gbos1936.h"
#include "StringUtils.h"
#include "Tile.h"
#include "ImgMagick.h"
#include "ImageWarpByFunc.h"
#include "ganzc/LatLong-OSGBconversion.h"
#include "ReadKmlFile.h"
#include "OSTN02Perl.h"
#include "ReadDelimitedFile.h"
#include "GetBounds.h"
#include <time.h>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace po = boost::program_options;

//#include <wand/magick-wand.h>
//#include <wand/drawing-wand.h>
//#include <Magick++.h>
//rsync --size-only -v -r /var/www/os7 timsc@dev.openstreetmap.org:/home/ooc
//rsync --size-only -v -r /var/www/openlayers/os7 timsc@dev.openstreetmap.org:/home/timsc/public_html/openlayers

#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
namespace fs = boost::filesystem;

#include <list>
#include <iostream>
#include <exception>
#include <vector>
#include <map>
using namespace std;
//using namespace Magick;

#define DEFAULT_MAX_TILES 6
#define OUT_TILE_WIDTH 256
#define OUT_TILE_HEIGHT 256

//class OSTN02Perl gConverter;
class HelmertConverter gConverter;

struct RescaleParams
{
public:
	double xscale, yscale, xori, yori;
};

vector<double> RescaleTransform(vector<double> in, void *userPtr)
{
	struct RescaleParams &params = *(struct RescaleParams *)userPtr;
	vector<double> out;
	out.push_back(in[0] * params.xscale - params.xori);
	out.push_back(in[1] * params.yscale - params.yori);
	return out;
}

int GetResizedSubimage(class Tile &src, class Tile &dst, class ImgMagick &imageIn, class ImgMagick &imageOut)
{
	struct RescaleParams params;
	params.xscale = (src.lonmax - src.lonmin) * dst.sx / ((dst.lonmax - dst.lonmin) * src.sx);
	params.yscale = (src.latmax - src.latmin) * dst.sy / ((dst.latmax - dst.latmin) * src.sy);
	params.xori = params.xscale * src.sx * (dst.lonmin - src.lonmin) / (src.lonmax - src.lonmin);
	params.yori = params.yscale * src.sy * (src.latmax - dst.latmax) / (src.latmax - src.latmin);

	class ImageWarpByFunc warp;
	warp.Warp(imageIn, imageOut, RescaleTransform, &params);

	//cout << "scale " << xscale << "\t" << yscale << endl;
	//cout << "ori " << xori << "\t" << yori << endl;

	//double lat, lon, alt;
	//ConvertGbos1936ToWgs84(302000,588000,0.0,lat,lon, alt);
	//cout << "testA " << lat << "," << lon << endl;
	//cout << "test " << (lon - dstW) * dstWidth / (dstE - dstW) << "," << (dstN - lat) * dstHeight / (dstN - dstS) << endl;
	//cout << dst.sx<<","<<dst.sy << endl;
	


	return 1;
}

//************************

class CopyPixelsWithOsMask
{
public:
	int gnorth, gsouth, geast, gwest, boxset;

	CopyPixelsWithOsMask();
	void UpdateBoundingBox(const char *mapref);
	void UpdateBoundingBox(vector<int> &mapref);
	int CheckIfInBox(double lat, double lon);
	void CopyPixels(class ImgMagick &imageIn, class ImgMagick &imageOut, class Tile &tile);
};

CopyPixelsWithOsMask::CopyPixelsWithOsMask()
{
	gnorth = 0;
	gsouth = 0;
	geast = 0;
	gwest = 0;
	boxset = 0;
}

void CopyPixelsWithOsMask::UpdateBoundingBox(const char *mapref)
{
	int dEasting=0, dNorthing=0;
	OSGBGridRefToRefCoords(mapref,dEasting,dNorthing);

	if(!boxset || gsouth > dNorthing) gsouth = dNorthing;
	if(!boxset || gnorth < dNorthing) gnorth = dNorthing;
	if(!boxset || geast < dEasting) geast = dEasting;
	if(!boxset || gwest > dEasting) gwest = dEasting;
	boxset = 1;
	//double lat=-1.0, lon=-1.0, alt = -1.0;
	//ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
}

void CopyPixelsWithOsMask::UpdateBoundingBox(vector<int> &mapref)
{
	int dEasting=0, dNorthing=0;
	if (mapref.size()!=2) {cout<<"Error: Mapref needs 2 coordinates"<<endl;throw 3000;}
	dEasting = mapref[0];
	dNorthing = mapref[1];

	if(!boxset || gsouth > dNorthing) gsouth = dNorthing;
	if(!boxset || gnorth < dNorthing) gnorth = dNorthing;
	if(!boxset || geast < dEasting) geast = dEasting;
	if(!boxset || gwest > dEasting) gwest = dEasting;
	boxset = 1;
	//double lat=-1.0, lon=-1.0, alt = -1.0;
	//ConvertGbos1936ToWgs84(dEasting, dNorthing,0.0, lat, lon, alt);
}

int CopyPixelsWithOsMask::CheckIfInBox(double lat, double lon)
{
	double pnorth,peast,palt;
	gConverter.ConvertWgs84ToGbos1936(lat,lon, 0.0, peast,pnorth,palt);
	if(pnorth < gsouth) return 0;
	if(pnorth > gnorth) return 0;
	if(peast < gwest) return 0;
	if(peast > geast) return 0;

	return 1;
}

void CopyPixelsWithOsMask::CopyPixels(class ImgMagick &imageIn, class ImgMagick &imageOut, class Tile &tile)
{
	int width = imageIn.GetWidth();
	int height = imageIn.GetHeight();
	int chans = imageIn.GetNumChannels();

	//Check corners (quicker to check then exhaustively check each pixel)
	double lat, lon;
	tile.UnProject(0, 0, lat, lon);
	int cornerCheck = this->CheckIfInBox(lat, lon);
	tile.UnProject(width, 0, lat, lon);
	cornerCheck = cornerCheck && this->CheckIfInBox(lat, lon);
	tile.UnProject(width, height, lat, lon);
	cornerCheck = cornerCheck && this->CheckIfInBox(lat, lon);
	tile.UnProject(0, height, lat, lon);
	cornerCheck = cornerCheck && this->CheckIfInBox(lat, lon);
	//cout << cornerCheck << endl;
	if(cornerCheck || boxset == 0) //Entire tile is in mask
	{
		
		//Copy every pixel
		for(int i=0;i<width;i++)
		for(int j=0;j<height;j++)
		for(int k=0;k<chans;k++)
		{
			double val = imageIn.GetPix(i,j,k);
			imageOut.SetPix(i,j,k,val);
		}
	}
	else
	{
		//Check each pixel and copy if inside mask
		for(int i=0;i<width;i++)
		for(int j=0;j<height;j++)
		for(int k=0;k<chans;k++)
		{
		
			tile.UnProject(i, j, lat, lon);
			int inMask = this->CheckIfInBox(lat, lon);
			if(!inMask) continue;

			double val = imageIn.GetPix(i,j,k);
			imageOut.SetPix(i,j,k,val);
		}
	}

}

void ImageToBlack(class ImgMagick &image)
{
	int width = image.GetWidth();
	int height = image.GetHeight();
	int chans = image.GetNumChannels();

	for(int i=0;i<width;i++)
	for(int j=0;j<height;j++)
	for(int k=0;k<chans;k++)
	{
		image.SetPix(i,j,k,0);
	}
}

//************************************

class SourceKml
{
public:
	class Tile tile;
	class ImgMagick image;
	vector<vector<int> > bounds;
	string kmlFilename;
	string imgFilename;
	clock_t lastAccess;
	int maxZoomVisible;

	SourceKml() {lastAccess = 0;maxZoomVisible=-1;}
	SourceKml(const SourceKml &a) {operator=(a);}
	virtual ~SourceKml() {}
	SourceKml& operator=(const SourceKml& a)
	{
		tile = a.tile;
		image = a.image;
		bounds = a.bounds;
		imgFilename = a.imgFilename;
		kmlFilename = a.kmlFilename;
		lastAccess = a.lastAccess;
		maxZoomVisible = a.maxZoomVisible;
		return *this;
	}
};

//*****************************************

void DrawMarkerPix(class ImgMagick &img, int x, int y, double r, double g, double b)
{
	if(x < 0 || x >= img.GetWidth()) return;
	if(y < 0 || y >= img.GetHeight()) return;
	img.SetPix(x, y, 0, r);
	img.SetPix(x, y, 1, g);
	img.SetPix(x, y, 2, b);
}

void DrawMarker(class ImgMagick &img, double x, double y)
{
	//Draw to tile
	for(int i=-1;i<=1;i++)
	for(int j=-1;j<=1;j++)
	{
		if(i != 0 || j != 0)
			DrawMarkerPix(img,x+(double)i+0.5,y+(double)j+0.5,0.0,0.0,0.0);
		else
			DrawMarkerPix(img,x+(double)i+0.5,y+(double)j+0.5,255.0,0.0,255.0);
	}	
}

//***************************************

boost::mutex accessTimeLock;

int RequestTileLoading(vector<class SourceKml> &src,class SourceKml &toLoad, int maxTilesLoaded)
{
	//Touch this timer to prevent it being unloaded straight away
	accessTimeLock.lock(); //Reset timer
	toLoad.lastAccess = clock();
	accessTimeLock.unlock();

	if(toLoad.image.Ready())
	{
		return 2; //Don't do anything if tile already in memory
	}

	//Count how many tiles in memory
	int count = 0;
	for(unsigned int i=0;i<src.size();i++)
		if(src[i].image.Ready()) count ++;
	cout << "Currently " << count << " tiles in memory" << endl;

	clock_t lowTime = 0; int lowTimeSet = 0; int lowIndex = -1;
	while(count >= maxTilesLoaded)
	{
		if(count >= maxTilesLoaded + 2)
		{
			cout << "Failing..... too many tiles loaded in memory" << endl;
			return 0;
		}

		//Need to unload a tile. Find tile that has not been recently used.
		//cout << "count " << count << ", src size " << src.size() << endl;
		accessTimeLock.lock();
		for(unsigned int i=0;i<src.size();i++)
		{
			//cout << "accessed " << src[i].imgFilename << " at " << (int)src[i].lastAccess << ", ready" << src[i].image.Ready() << endl;
			if((src[i].lastAccess < lowTime || lowTimeSet == 0) && src[i].image.Ready())
			{
				lowTime = src[i].lastAccess;
				lowIndex = i;
				//cout << i << "," << (int)src[i].lastAccess << "," << lowTime << "," << (bool)(src[i].lastAccess < lowTime) << endl;
				//cout << "lowIndex " << lowIndex << endl;
				lowTimeSet = 1;
			}
		}
		accessTimeLock.unlock();
			
		//Unload tile
		if(lowIndex != -1)
		{
			cout << "Unloading " << src[lowIndex].imgFilename << endl;
			src[lowIndex].image.Clear();
		}
		else
		{
			cout << "Failed to find a tile to unload" << endl;
			return 0;
		}	

		//Recount
		count = 0;
		for(unsigned int i=0;i<src.size();i++)
			if(src[i].image.Ready()) count ++;
	}
		
	//Load required tile into mem
	cout << "Loading " << toLoad.imgFilename <<"..."; cout.flush();
	int ret = toLoad.image.Open(toLoad.imgFilename.c_str());
	if(ret < 0) {cout << "Failed to open image " << toLoad.imgFilename << endl;return 0;}
	else cout << "done" << endl;
	toLoad.tile.sx = toLoad.image.GetWidth();
	toLoad.tile.sy = toLoad.image.GetHeight();

	return 1;
}



//***************************************

boost::mutex statusLock;
volatile int gThreadsRunning = 0;

class TileJob
{
public:
	int zoom, verbose;
	int tileLon, tileLat;
	string outFilename;
	string outFolder0, outFolder1, outFolder2;
	vector<unsigned int> kmlSrc;
	class Tile dst;
	int complete, running, failed;
	int enableTileLoading, maxTilesLoaded;
	vector<class SourceKml> *srcPtr;
	class TileJob *originalObj;

	TileJob();
	TileJob& operator=(const TileJob& a);
	int Render();
	void operator()()
	{
		statusLock.lock();
		this->originalObj->running = 1;
		statusLock.unlock();

		try{Render();}
		catch (int e) {if (verbose>=1) cout << "Error: render failed ("<<e<<")"<< endl;}

		statusLock.lock();
		this->originalObj->running = 0;
		this->originalObj->complete = 1;
		gThreadsRunning --;
		statusLock.unlock();
	}
};

TileJob::TileJob()
{
	zoom = 0; verbose = 2;
	tileLon = 0; tileLat = 0;
	complete = 0;
	running = 0;
	failed = 0;
	enableTileLoading = 0;
	maxTilesLoaded = DEFAULT_MAX_TILES;
	srcPtr=NULL;
	originalObj = this;
}

TileJob& TileJob::operator=(const TileJob& a)
{
	zoom = a.zoom; verbose = a.verbose;
	tileLon = a.tileLon; tileLat = a.tileLat;
	complete = a.complete;
	running = a.running;
	failed = a.failed;
	enableTileLoading = a.enableTileLoading;
	maxTilesLoaded = a.maxTilesLoaded;
	srcPtr=a.srcPtr;
	originalObj = a.originalObj;	
	return *this;
}

int TileJob::Render()
{

	if(srcPtr==NULL) {cout << "Error: Bad srcPtr pointer" << endl; throw(4000);}
	vector<class SourceKml> &src = *srcPtr;

	class ImgMagick outImg;
	outImg.SetNumChannels(3);
	outImg.SetWidth(this->dst.sx);
	outImg.SetHeight(this->dst.sy);
	ImageToBlack(outImg);

	class ImgMagick tile;
	tile.SetNumChannels(3);
	tile.SetWidth(256);
	tile.SetHeight(256);
	ImageToBlack(tile);

	//Magick::Image image(src.GetWidth(),src.GetHeight(),"RGB",CharPixel,src.GetInternalDataConst());
	
	//cout << size.width() << "," << size.height() << endl;

	//For each KML source
	int countSrcTilesUsed = 0;
	for(unsigned int t=0;t<this->kmlSrc.size();t++)
	{
	class SourceKml &srcKml = src[this->kmlSrc[t]];

	
	countSrcTilesUsed ++;
	if(countSrcTilesUsed == 1)
	{
		if(verbose>=2) cout << "Generating tile " << this->tileLon << "," << this->tileLat << endl;
		if(verbose>=2) cout << "Tile area " << this->dst.latmin << "," << this->dst.lonmin << "," << this->dst.latmax << "," << this->dst.lonmax << endl;
	}
	if(verbose>=2) cout << "Copying pixels from " << srcKml.imgFilename << endl;

	//Check map is in memory
	if(!srcKml.image.Ready())
	{
		if(enableTileLoading)
		{
			int status = RequestTileLoading(src,srcKml,this->maxTilesLoaded);
			if (status == 0)
			{
				statusLock.lock();
				this->originalObj->complete = 1;
				this->originalObj->running = 0;
				this->originalObj->failed = 1;
				gThreadsRunning --;
				statusLock.unlock();
				if(verbose>=1) cout << "Error: Could not load tile"<< endl;
				throw(1000);
			} 
		}
		else
		{
			if(verbose>=1) cout << "Error:Loading dep not satisfied"<< endl;
			statusLock.lock();
			this->originalObj->complete = 1;
			this->originalObj->running = 0;
			this->originalObj->failed = 1;
			gThreadsRunning --;
			statusLock.unlock();
			throw(2000);
		}
	}

	//Update access timer for this tile
	accessTimeLock.lock();
	srcKml.lastAccess = clock();
	accessTimeLock.unlock();

	//Get location in source image containing area of interest
	double xmin = (double)srcKml.tile.sx * (this->dst.lonmin - srcKml.tile.lonmin) / (srcKml.tile.lonmax - srcKml.tile.lonmin);
	double xmax = (double)srcKml.tile.sx * (this->dst.lonmax - srcKml.tile.lonmin) / (srcKml.tile.lonmax - srcKml.tile.lonmin);
	double ymin = (double)srcKml.tile.sy * (srcKml.tile.latmax - this->dst.latmax) / (srcKml.tile.latmax - srcKml.tile.latmin);	
	double ymax = (double)srcKml.tile.sy * (srcKml.tile.latmax - this->dst.latmin) / (srcKml.tile.latmax - srcKml.tile.latmin);	
	xmin = (int)(xmin - 100);
	xmax = (int)(xmax + 100);
	ymin = (int)(ymin - 100);
	ymax = (int)(ymax + 100);
	if(xmin < 0) xmin = 0;
	if(ymin < 0) ymin = 0;
	if(xmax > srcKml.tile.sx) xmax = srcKml.tile.sx;
	if(ymax > srcKml.tile.sy) ymax = srcKml.tile.sy;

	//cout << "a" << endl;
	//GetSubimage(image,src,temp,inter);
	//temp.write("temp.png");

	//cout << "b" << endl;
	GetResizedSubimage(srcKml.tile,this->dst,srcKml.image,tile);
	//GetResizedSubimage(inter,dst,temp);
	//cout << "c" << endl;
	
	class CopyPixelsWithOsMask mask;
	for(unsigned int i=0;i<srcKml.bounds.size();i++)
		mask.UpdateBoundingBox(srcKml.bounds[i]);
	mask.CopyPixels(tile, outImg, this->dst);

	} //End of copy KML source

	//Get OS grid positions of tile corners
	#if 0
	int drawGridCorners = 0;	
	if(drawGridCorners)
	{
	double eaMax=-1.0, noMax=-1.0, heMax=-1.0;
	double eaMin=-1.0, noMin=-1.0, heMin=-1.0;
	gConverter.ConvertWgs84ToGbos1936(job.dst.latmax, job.dst.lonmax, 0.0,eaMax, noMax, heMax);
	gConverter.ConvertWgs84ToGbos1936(job.dst.latmin, job.dst.lonmin, 0.0,eaMin, noMin, heMin);
	//cout << eaMax << "\t" << noMax << "\t" << heMax << endl;
	//cout << eaMin << "\t" << noMin << "\t" << heMin << endl;

	//Draw grid corners on tile
	for(int ea = floor(eaMin/1000.0);ea < ceil(eaMax/1000.0);ea+=1)
	for(int no = floor(noMin/1000.0);no < ceil(noMax/1000.0);no+=1)
	{
	//cout << ea << "," << no << endl;
		//Convert grid ref to lat lon
		double cornerLat = -1.0, cornerLon = -1.0, cornerHeight = -1.0;
		gConverter.ConvertGbos1936ToWgs84(ea*1000, no*1000, 0.0,cornerLat, cornerLon, cornerHeight);

		//Convert to pixel position
		double cornerx=-1.0, cornery=-1.0;
		job.dst.Project(cornerLat, cornerLon, cornerx, cornery);

		DrawMarker(outImg, cornerx, cornery);
	}
	}
	#endif

	//outImg = tile;

		if ( !fs::exists( this->outFolder0 ) )
			fs::create_directory( this->outFolder0 );

		if ( !fs::exists( this->outFolder1 ) )
			fs::create_directory( this->outFolder1 );

		if ( !fs::exists( this->outFolder2 ) )
			fs::create_directory( this->outFolder2 );

	   	outImg.Save(this->outFilename.c_str());
		//temp.syncPixels();
		//image.syncPixels();

	statusLock.lock();
	this->originalObj->complete = 1;
	statusLock.unlock();

	return 1;
}

//***************************************

int main(int argc, char ** argv)
{

	
	//Image imageInOut("step.jpg");
	//imageInOut.crop( Geometry(255,255,0,0) );
	//imageInOut.write("test2.png");

	/*class Tile test;
	vector<string> test2;
	string test3;
	int retOpen = ReadKmlFile("/home/tim/dev/os7files/rect/73.kml", test, test3, test2);

	exit(0);*/

	//cout << long2tile(-3.68, zoom) << "," << lat2tile(54.8333,zoom) << endl;
	//cout << long2tile(-3.04, zoom) << "," << lat2tile(55.2446,zoom) << endl;

	vector<class SourceKml> src;
	class DelimitedFile boundsFile;

	string outFolder = "out";
	unsigned int minZoom = 10;
	unsigned int maxZoom = 14;
	int maxTilesLoaded = DEFAULT_MAX_TILES;
	int targetNumThreads = boost::thread::hardware_concurrency();
	string boundsFilename = "bounds.csv";
	vector<string> inputFiles ;
	string execFilename = "gentiles";
	if(argc >= 1) execFilename = argv[0];

	//Process program options using boost library
	po::variables_map vm;
	po::options_description desc("Allowed options");
	try{
	po::positional_options_description pd;
        pd.add("positional", -1); 

	desc.add_options() ("minzoom", po::value<int>(), "Minimum zoom level")
		("maxzoom", po::value<int>(), "Maximum zoom level")
		("output", po::value<string>(), "Output folder")
		("bounds", po::value<string>(), "Bounds filename")
		("maxloaded", po::value<int>(), "Maximum number of tiles in memory")
		("threads", po::value<int>(), "Maximum number of threads")
		("positional", po::value<std::vector<std::string> >(), "Input KML files")
		("help","help message");

		//("annot-offset",po::value<double>(),"time offset of anvil annotation track")
	 po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().positional(pd).run();
	po::store(parsed, vm);
	po::notify(vm);

	if(vm.count("help")) {cout << desc << endl; exit(0);}
	if(vm.count("minzoom")) minZoom = vm["minzoom"].as<int>();
	if(vm.count("maxzoom")) maxZoom = vm["maxzoom"].as<int>();
	if(vm.count("maxloaded")) maxTilesLoaded = vm["maxloaded"].as<int>();
	if(vm.count("threads")) targetNumThreads = vm["threads"].as<int>();
	if(vm.count("output")) outFolder = vm["output"].as<string>();
	if(vm.count("bounds")) boundsFilename = vm["bounds"].as<string>();

	if (vm.count("positional"))
	{
		inputFiles = vm["positional"].as< vector<string> >();
		//for(unsigned int i=0;i<inputFiles.size();i++)
		//	cout << "input file " << inputFiles[i] << endl;
	}

	}
	catch(exception &e)
	{
		cerr << "error: " << e.what() << endl;
	}

	if(inputFiles.size()==0)
	{
		cout << "No input KML files were specified" << endl;
		cout << "Usage: " << execFilename << " [options] one.kml [two.kml ...]" << endl;
		cout << desc << endl; exit(0);
	}

	if(minZoom > maxZoom)
	{
		cout << "Error max zoom is lower than min zoom" << endl;
		cout << desc << endl; exit(0);
	}

	int boundsOpen = boundsFile.Open(boundsFilename.c_str());
	if(boundsOpen < 1) cout << "Could not read "<<boundsOpen<<" file" << endl;
	class Tile sourceBBox; int sourceBBoxSet = 0;

	//***************************************************
	//** For each input file, parse KML into local mem
	//***************************************************

	for(unsigned int i=0;i<inputFiles.size();i++)
	{
		class SourceKml temp;
		cout << "Source file '" << inputFiles[i] << "'" << endl;
		string filePath = GetFilePath(inputFiles[i]);

		src.push_back(temp);
		class SourceKml &last = src[src.size()-1];
		string imgFilename;
		last.kmlFilename = inputFiles[i];
		int ret = ReadKmlFile(last.kmlFilename.c_str(), last.tile, imgFilename);
		last.imgFilename = filePath;
		last.imgFilename += "/";
		last.imgFilename += imgFilename;
		if(ret < 1) {cout << "Kml "<<last.kmlFilename<<" not found"; exit(0);}
		cout << last.tile.latmin << "," << last.tile.lonmin << "," << last.tile.latmax << "," << last.tile.lonmax << endl;
		cout << "image filename '" << last.imgFilename << "'" << endl;

		//Update source bounding box
		if(sourceBBox.latmin > last.tile.latmin || !sourceBBoxSet) sourceBBox.latmin = last.tile.latmin;
		if(sourceBBox.latmax < last.tile.latmax || !sourceBBoxSet) sourceBBox.latmax = last.tile.latmax;
		if(sourceBBox.lonmin > last.tile.lonmin || !sourceBBoxSet) sourceBBox.lonmin = last.tile.lonmin;
		if(sourceBBox.lonmax < last.tile.lonmax || !sourceBBoxSet) sourceBBox.lonmax = last.tile.lonmax;
		sourceBBoxSet = 1;

		//int ret = last.image.Open(last.imgFilename.c_str());
		//if(ret < 0){cout << "Filed to open image" << endl;exit(0);}
		//last.bounds.push_back("NR550000");
		//last.bounds.push_back("NR950450");
		string filenameNoPath = RemoveFilePath(inputFiles[i].c_str());
		vector<vector<int> > boundsTemp;
		GetBounds(boundsFile,filenameNoPath.c_str(),boundsTemp);
		
		cout << "bounds (" << boundsTemp.size() << ")";
		for(unsigned int j=0;j<boundsTemp.size();j++)
		{
			cout << "(";
			//cout << "["<<boundsTemp[j].size()<<"] ";
			for(unsigned int k=0;k<boundsTemp[j].size();k++) cout << boundsTemp[j][k]<<" ";
			cout << ")";
		}
		cout << endl;
		last.bounds = boundsTemp;
	}

	cout << "Input files bounding box:" << endl;
	cout << sourceBBox.latmin << "," << sourceBBox.lonmin << "," << sourceBBox.latmax << "," << sourceBBox.lonmax << endl;

	//Read the zoom limits from a file
	class DelimitedFile zoomLimitsFile;
	if(zoomLimitsFile.Open("zoomlimits.txt") < 0)
	{
		cout << "Zoom limits file not found" << endl;
		exit(-1);
	}
	map<string,int> zoomLimitMax, zoomLimitMin;
	for(unsigned int i=0;i<zoomLimitsFile.lines.size();i++)
	{
		class DelimitedFileLine &line = zoomLimitsFile.lines[i];
		//cout << line[0].GetVals() << endl;
		if(line.NumVals() >= 2) zoomLimitMin[line[0].GetVals()] = line[1].GetVali();
		if(line.NumVals() >= 3) zoomLimitMax[line[0].GetVals()] = line[2].GetVali();
	}

	//Set zoom levels in the respective kml sources
	for(unsigned int t=0;t<src.size();t++)
	{
		class SourceKml &srcKml = src[t];
		boost::filesystem::path p(srcKml.kmlFilename.c_str()); 
		string chkStr = p.filename();

		map<string,int>::iterator it = zoomLimitMax.find(chkStr);
		if(it!=zoomLimitMax.end())
		{
			cout << "Max zoom for " << chkStr << " is " << it->second << endl;
			srcKml.maxZoomVisible = it->second;
		}
	}

	//************************************
	//** Calculate tile jobs to do
	//************************************
	vector<class TileJob> jobs;
	for(unsigned int zoom=minZoom;zoom<=maxZoom;zoom++)
	{
	int srcWtile = long2tile(sourceBBox.lonmin, zoom);
	int srcEtile = long2tile(sourceBBox.lonmax, zoom);
	int srcStile = lat2tile(sourceBBox.latmin,zoom);
	int srcNtile = lat2tile(sourceBBox.latmax,zoom);
	cout << "Planning zoom "<<zoom<<", tiles covered " << srcStile << "," << srcWtile << "," << srcNtile << "," << srcEtile << endl;

	for(int tileLon = srcWtile; tileLon <= srcEtile; tileLon ++)
	for(int tileLat = srcNtile; tileLat <= srcStile; tileLat ++)
	{
		class TileJob job;

		string outFilename = outFolder;
		string outFolder0 = outFilename;
		outFilename += "/";
		outFilename += IntToString(zoom);
		string outFolder1 = outFilename;
		outFilename += "/";
		outFilename += IntToString(tileLon);
		string outFolder2 = outFilename;
		outFilename += "/";
		outFilename += IntToString(tileLat);
		outFilename += ".jpg";

		int skipExistingTiles = 1;
		if ( fs::exists( outFilename ) && skipExistingTiles)
		{
			//cout << "Already exists: " << outFilename << endl;
			continue;
		}

		job.zoom = zoom;
		job.tileLon = tileLon;
		job.tileLat = tileLat;
		job.outFilename = outFilename;
		job.outFolder0 = outFolder0;
		job.outFolder1 = outFolder1;
		job.outFolder2 = outFolder2;
		job.srcPtr = &src;
		job.maxTilesLoaded = maxTilesLoaded;

		job.dst.latmax = tile2lat(job.tileLat, job.zoom);
		job.dst.lonmin = tile2long(job.tileLon, job.zoom);
		job.dst.latmin = tile2lat(job.tileLat+1, job.zoom);
		job.dst.lonmax = tile2long(job.tileLon+1, job.zoom);
		job.dst.sx = OUT_TILE_WIDTH;
		job.dst.sy = OUT_TILE_HEIGHT;

		//cout << "Planning tile " << tileLat << "," << tileLon << "," << zoom << endl;
		//For each KML source
		//cout << "test"<<src.size()<< endl;
		for(unsigned int t=0;t<src.size();t++)
		{
			class SourceKml &srcKml = src[t];

			//cout << srcKml.maxZoomVisible << endl;
			if(srcKml.maxZoomVisible >= 0 && srcKml.maxZoomVisible < (int)zoom)
				continue;

			//Check kml src actually contains useful info for this destination tile
			int noOverlap = 0;
			if(srcKml.tile.lonmax < job.dst.lonmin) noOverlap = 1;
			if(srcKml.tile.lonmin > job.dst.lonmax) noOverlap = 1;
			if(srcKml.tile.latmax < job.dst.latmin) noOverlap = 1;
			if(srcKml.tile.latmin > job.dst.latmax) noOverlap = 1;
			//cout << noOverlap << endl;
			if(!noOverlap)
			{
				job.kmlSrc.push_back(t);
				//cout << "source found: " << t << endl;
			}
		}
		if(job.kmlSrc.size() == 0) continue; //Don't bother with empty tiles

		//Add job to list
		jobs.push_back(job);
		
		//Ensure pointer is set
		//jobs[jobs.size()-1].originalObj = &jobs[jobs.size()-1];
		//cout << jobs[0].originalObj->complete << ",";
		//cout << jobs[1].originalObj->complete << ",";
	}	
	}

	for (unsigned int i=0;i<jobs.size();i++)
	{
		jobs[i].originalObj = &jobs[i];
	}

	//*******************************
	//** Start rendering loop
	//*******************************

	int running = 1;
	int renderCount = 0;
	while (running)
	{
	//Get the most complex dep
	unsigned int maxDepNum = 0;
	unsigned int countRemaining = 0;
	vector<unsigned int> maxDep;
	for(unsigned int jobNum=0;jobNum<jobs.size();jobNum++)
	{	
		class TileJob &job = jobs[jobNum];
		statusLock.lock();
		int complete = job.originalObj->complete;
		//cout << complete << "," ;
		statusLock.unlock();
		if (complete) continue;
		countRemaining ++;
		if(job.kmlSrc.size() > maxDepNum)
		{
			maxDepNum = job.kmlSrc.size();
			maxDep = job.kmlSrc;
		}
	}

	cout << "Most complex dep (";
	for (unsigned int j=0;j<maxDep.size();j++) cout << maxDep[j] << " ";
	cout << ")"<<endl;

	int tilesLoadable = ((int)maxDep.size() <= maxTilesLoaded);

	//What jobs are satistifed by this dep?
	vector<class TileJob *> jobsInDep;
	for(unsigned int jobNum=0;jobNum<jobs.size();jobNum++)
	{
		int skipJob = 0;
		class TileJob &job = jobs[jobNum];
		if (job.originalObj->complete) continue;
		if(tilesLoadable)
		{
			//When we can fit the source data into memory, be less strict in checking here
			for (unsigned int i=0;i<job.kmlSrc.size();i++)
			{
			int match = 0;
			for (unsigned int j=0;j<maxDep.size();j++)
			{
				if(job.kmlSrc[i] == maxDep[j]) match = 1;
			}
			if (match == 0) skipJob = 1;
			}
		}
		else //For slow loading, use strict dep checking
			if(job.kmlSrc != maxDep) skipJob = 1;
		if(!skipJob) jobsInDep.push_back(&job);
	}

	cout << "Tiles satisfied by this dep: " << jobsInDep.size() << endl;

	if(countRemaining==0)
	{
		cout << "All done!" << endl;
		running = 0;
		continue;
	}

	cout << "tilesLoadable " << tilesLoadable << endl;
	if(tilesLoadable)
	{
		//Load appropriate tiles for this dep
		for(unsigned int t=0;t<maxDep.size();t++)
		{
			class SourceKml &toLoad = src[maxDep[t]];
			int status = RequestTileLoading(src,toLoad,maxTilesLoaded);
			if (status == 0) exit(0);		
		}
	}

	//********************************
	//***  Produce tiles for this dep
	//********************************
	
	for(unsigned int jobNum=0;jobNum<jobsInDep.size();jobNum++)
	{

	class TileJob &job = *jobsInDep[jobNum];
	renderCount++;
	cout << "Render Tile " << renderCount << " of " << jobs.size() << " (in dep "<<jobNum<<" of "<<jobsInDep.size() << ")"<<endl;
	cout << "Depends on KML srcs ";
	for (unsigned int j=0;j<job.kmlSrc.size();j++) cout << job.kmlSrc[j] << " ";
	cout << endl;

	if(tilesLoadable) //Do processing in parallel
	{
		job.enableTileLoading = 0;
		job.verbose = 1;

		boost::thread test = boost::thread(job);

		//Increment number of running threads
		statusLock.lock();
		gThreadsRunning ++;
		statusLock.unlock();

		//Count how many running
		statusLock.lock();
		int localThreadRunning = gThreadsRunning;
		statusLock.unlock();
		//cout << "threadsRunning: " << localThreadRunning << endl;

		//Wait if we have enough running threads
		while (localThreadRunning >= targetNumThreads)
		{
		
			boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(100);
			boost::thread::sleep(timeout);
		
			//Check if any have stopped
			statusLock.lock();
			localThreadRunning = gThreadsRunning;
			statusLock.unlock();
		}
	}
	else //Do processing in serial
	{
		job.enableTileLoading = 1;
		job();
		job.enableTileLoading = 0;
	}


	} //End of job loop

	//Wait for things to finish
	cout << "Waiting for threads to finish..."; cout.flush();
	statusLock.lock();
	int localThreadRunning = gThreadsRunning;
	statusLock.unlock();
	while (localThreadRunning > 0)
	{
		boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(100);
		boost::thread::sleep(timeout);
		
		//Check if any have stopped
		statusLock.lock();
		localThreadRunning = gThreadsRunning;
		statusLock.unlock();
	}
	cout << "done" << endl;

	} //End of dep depth loop

	//Count failures
	int countFail = 0;
	for(unsigned int jobNum=0;jobNum<jobs.size();jobNum++)
	{
		class TileJob &job = jobs[jobNum];
		if(job.originalObj->failed) countFail ++;
	}
	cout << "Number of tiles failed: " << countFail << endl;
}

