#ifndef IMAGE_WARP_BY_FUNC_H
#define IMAGE_WARP_BY_FUNC_H

#include "ImgMagick.h"
#include <vector>
using namespace std;

class ImageWarpByFunc
{
public:
	ImageWarpByFunc();
	virtual ~ImageWarpByFunc();
	int Warp(class ImgMagick &in,
		class ImgMagick &out,
		vector<double> (*transform)(vector<double> in, void *userPtr),
		void *userPtr);

	int xsize, ysize;
};

#endif //IMAGE_WARP_BY_FUNC_H

