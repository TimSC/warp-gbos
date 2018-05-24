
#ifndef PYOSTN02_H_
#define PYOSTN02_H_

#include <vector>
#include <string.h>
#include <Python.h>
using namespace std;

class PyOstn02
{
public:
	PyOstn02();
	virtual ~PyOstn02();

	int OsgbToWgs84(vector<double> &out, double ea, double no, double ele);
	void GetTransformName(char *buffer, int maxLen)
	{
		strncpy(buffer, "OSTN02 via Geo-Coordinates-OSGB (Python Port)", maxLen);
	}

	void ConvertGbos1936ToWgs84(double ea, double no, double he,
		double &latOut, double &lonOut, double &heOut);
	void ConvertWgs84ToGbos1936(double lat, double lon, double he,
		double &eaOut, double &noOut, double &heOut);

private:
	PyObject *pFunc;
};

#endif //PYOSTN02_H_

