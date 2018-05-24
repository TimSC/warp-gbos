
#ifndef IMG_MAGICK_H
#define IMG_MAGICK_H

#include "ImgFrameBase.h"
#define IMG_FRAME_TYPE_MAGICKWAND 101

class ImgMagick : public ImgFrameBase
{
public:
	ImgMagick();
	ImgMagick(const ImgMagick &a);
	virtual ~ImgMagick();
	ImgMagick& operator=(const ImgMagick& a);

	virtual int Open(const char *filename);
	virtual int Save(const char *filename);
	virtual void Clear();
	virtual int Ready();

	//Pixel data
	virtual double GetPix(int x, int y, unsigned int channel) const;
	virtual void SetPix(int x, int y, unsigned int channel, double val);

	//Meta data
	virtual int GetWidth() const;
	virtual int GetHeight() const;
	virtual int GetNumChannels() const;
	
	virtual int SetWidth(int val);
	virtual int SetHeight(int val);
	virtual int SetNumChannels(int val);

	virtual int GetInternalImageType() const;
	virtual const void *GetInternalDataConst() const;

protected:
	unsigned char *raw;
	int height;
	int width;
	int channels;
};

#endif //IMG_MAGICK_H
