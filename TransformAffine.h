
#ifndef TRANSFORM_AFFINE_H
#define TRANSFORM_AFFINE_H

#include <vector>
using namespace std;

vector<double> AffineProject(vector<double> point, vector<double>pose);
vector<double> AffineUnProject(vector<double> point, vector<double>pose);

class AffineProjection
{
public:
	AffineProjection();
	virtual ~AffineProjection();

	//vector<double> Project(vector<double> point, vector<double>pose) {vector<double> empty; return empty;};
	//vector<double> UnProject(vector<double> point, vector<double>pose) {vector<double> empty; return empty;};

	void Clear();
	void AddPoint(vector<double> original, vector<double> transformed);
	void AddPoint(double ox, double oy, double tx, double ty);
	void AddPoint(double ox, double oy, vector<double> transformed);
	vector<double> Estimate();

	vector<vector<double> > originalPoints, transformedPoints;
	
};

template <class outType, class InType, class InTypeIt> outType CastVector (InType in) {
	outType out;
	for(InTypeIt it = in.begin();it!=in.end();it++)
	{
		out.push_back(*it);
	}
	return out;
}

#endif //TRANSFORM_AFFINE_H

