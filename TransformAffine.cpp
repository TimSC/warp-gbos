
#include "TransformAffine.h"
#include <newmat/newmat.h>
#include "ErrorHandling.h"
#include <iostream>
using namespace NEWMAT;

vector<double> AffineProject(vector<double> point, vector<double>pose) 
{
	if(pose.size()!=6)
		ThrowError<logic_error>("Affine transform vector unexpected size",__LINE__,__FILE__);
	if(point.size()!=2)
		ThrowError<logic_error>("Point to transform does not have 2 components",__LINE__,__FILE__);

	vector<double> result; 
	result.push_back(point[0] * pose[0] + point[1] * pose[1] + pose[2]);
	result.push_back(point[0] * pose[3] + point[1] * pose[4] + pose[5]);
	return result;
}

vector<double> AffineUnProject(vector<double> point, vector<double>pose) 
{	
	vector<double> result; 
	if(pose.size()!=6)
		ThrowError<logic_error>("Affine transform vector unexpected size",__LINE__,__FILE__);
	if(point.size()!=2)
		ThrowError<logic_error>("Point to transform does not have 2 components",__LINE__,__FILE__);

	Matrix transform(3,3);
	transform(1,1) = pose[0];
	transform(1,2) = pose[1];
	transform(1,3) = pose[2];
	transform(2,1) = pose[3];
	transform(2,2) = pose[4];
	transform(2,3) = pose[5];
	transform(3,1) = 0.0;
	transform(3,2) = 0.0;
	transform(3,3) = 1.0;

	Matrix pos(3,1);
	pos(1,1) = point[0];
	pos(2,1) = point[1];
	pos(3,1) = 1.0;

	Matrix unprojected = transform.i() * pos;

	result.push_back(unprojected(1,1));
	result.push_back(unprojected(2,1));
	return result;
}

//**********************************************************************

AffineProjection::AffineProjection()
{

}

AffineProjection::~AffineProjection()
{

}

void AffineProjection::Clear()
{
	originalPoints.clear();
	transformedPoints.clear();
}

void AffineProjection::AddPoint(vector<double> original, vector<double> transformed)
{
	originalPoints.push_back(original);
	transformedPoints.push_back(transformed);
}

void AffineProjection::AddPoint(double ox, double oy, double tx, double ty)
{
	vector<double> temp, temp2;
	temp.push_back(ox);temp.push_back(oy);
	temp2.push_back(tx);temp2.push_back(ty);
	AddPoint(temp,temp2);
}

void AffineProjection::AddPoint(double ox, double oy, vector<double> transformed)
{
	vector<double> temp;
	temp.push_back(ox);temp.push_back(oy);
	AddPoint(temp,transformed);
}

void PrintMatrix(Matrix &m)
{
	for(int i=0;i<m.Nrows();i++)
	{
		for(int j=0;j<m.Ncols();j++)
		{
			cout << m(i+1,j+1) << ",";
		}
		cout << endl;
	}
}

vector<double> AffineProjection::Estimate()
{
	if(originalPoints.size()!=transformedPoints.size())
		ThrowError<logic_error>("Inconsistent number of points in constraints",__LINE__,__FILE__);
	if(originalPoints.size()==0)
		ThrowError<logic_error>("Cannot estimate transform with no points",__LINE__,__FILE__);
	int dim = originalPoints[0].size();

	try
	{

	Matrix po(dim+1,originalPoints.size());
	for(unsigned int i=0;i<originalPoints.size();i++)
	{
		for(unsigned int j=0;j<originalPoints[i].size();j++)
			po(j+1,i+1) = originalPoints[i][j];
		po(dim+1,i+1) = 1.0;
	}
	//PrintMatrix(po);

	Matrix pt(dim,transformedPoints.size());
	for(unsigned int i=0;i<transformedPoints.size();i++)
	{
		for(unsigned int j=0;j<transformedPoints[i].size();j++)
			pt(j+1,i+1) = transformedPoints[i][j];
	}
	//PrintMatrix(pt);

	Matrix invPo = po.t() * (po * po.t()).i(); //Pseudo inverse
	Matrix ptInvPo = pt * invPo;
	//PrintMatrix(ptInvPo);

	//Convert 2D matrix into 1D for output
	vector<double>out;
	for(int i=0;i<ptInvPo.Nrows();i++)
	{
		for(int j=0;j<ptInvPo.Ncols();j++)
		{
			out.push_back( ptInvPo(i+1,j+1) );
		}
	}
	return out;
	}
	catch(Exception) { cout << Exception::what() << endl; }
	vector<double> empty;
	return empty;

}

