
#include "ImageWarpByFunc.h"
extern "C"
{
#include <xmorph/warp2.h>
}
#include <iostream>


//************************************************************

ImageWarpByFunc::ImageWarpByFunc()
{
	xsize = 10; ysize = 10;
}

ImageWarpByFunc::~ImageWarpByFunc()
{

}

int ImageWarpByFunc::Warp(class ImgMagick &in,
	class ImgMagick &out,
	vector<double> (*transform)(vector<double> in, void *userPtr),
	void *userPtr)
{
	//cout << "Creating raw image buffer" << endl;
	unsigned char *inBuf = (unsigned char *)in.GetInternalDataConst();

	//cout << "Creating raw image buffer" << endl;
	unsigned char *outBuf = (unsigned char *)out.GetInternalDataConst();

	//Calculate mesh transform
	double sx[xsize * ysize], sy[xsize * ysize];
	double ex[xsize * ysize], ey[xsize * ysize];
	MESHLABEL_T slabel[xsize * ysize];
	
	double sxmin = -0.0 * (double)in.GetWidth();
	double sxmax = 1.0 * (double)in.GetWidth();
	double symin = -0.0 * (double)in.GetHeight();
	double symax = 1.0 * (double)in.GetHeight();	

	for(int j=0;j<ysize;j++)
	for(int i=0;i<xsize;i++)
	{
		sx[i + j*xsize]=(double)i * (sxmax - sxmin) / (double)(xsize-1) + sxmin;
 		sy[i + j*xsize]=(double)j * (symax - symin) / (double)(ysize-1) + symin;
	}
	for(int i=0;i<xsize*ysize;i++)
	{
		vector<double> mesh; mesh.push_back(sx[i]);mesh.push_back(sy[i]);
		vector<double> meshProj = transform(mesh, userPtr);
		ex[i] = meshProj[0]; ey[i] = meshProj[1];
		slabel[i] = 0;
	}

	MeshT startMesh;
	startMesh.nx=xsize;
	startMesh.ny=ysize;
	startMesh.x = sx;
	startMesh.y = sy;
	startMesh.label = slabel;
	startMesh.changed = 0;
	startMesh.reference_counting = 0;

	MeshT endMesh;
	endMesh.nx=xsize;
	endMesh.ny=ysize;
	endMesh.x = ex;
	endMesh.y = ey;
	endMesh.label = slabel;
	endMesh.changed = 0;
	endMesh.reference_counting = 0;

	//cout << "Performing warp" << endl;
	warp_image_a_m(inBuf,in.GetWidth(), in.GetHeight(), in.GetNumChannels(), in.GetWidth()*in.GetNumChannels(),in.GetNumChannels(),
		outBuf,out.GetWidth(),out.GetHeight(), out.GetNumChannels(), out.GetWidth()*out.GetNumChannels(),out.GetNumChannels(),
		&startMesh, &endMesh);

	return 1;
}


