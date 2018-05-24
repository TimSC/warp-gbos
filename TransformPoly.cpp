
#include "TransformPoly.h"
#include <newmat/newmat.h>
#include "ErrorHandling.h"
#include <iostream>
#include <math.h>
using namespace NEWMAT;
//#define USE_OLD_POLYNOMIAL_EQ
#define HIGHEST_ALLOWED_POLY 100

vector<double> GetCoeff(int order, double x, double y)
{
	vector<double> out;

	#ifdef USE_OLD_POLYNOMIAL_EQ
	//Old equation (<= version 0.4)
	for(int i=0;i<order;i++)
	{
		out.push_back(pow(x,i+1));
		out.push_back(pow(y,i+1));
	}
	out.push_back(1.0);
	#else
	//Correct equation
	for(int i=0;i<=order;i++)
		for(int j=0;j<=order;j++)
			out.push_back(pow(x,i)*pow(y,j));
	#endif

	return out;
}

int CoeffSize(int order)
{
	vector<double> dummy = GetCoeff(order,0.0,0.0);
	return dummy.size();
}

int CalcOrderFitForNumConstraints(int numConstr)
{
	numConstr = numConstr/2;
	for(unsigned int i=1;i<HIGHEST_ALLOWED_POLY;i++)
	{
		if (CoeffSize(i) > numConstr)
			return i - 1;
	}
	return HIGHEST_ALLOWED_POLY;
}

vector<double> PolyProject(vector<double> point, vector<double>pose, int order) 
{
	if(pose.size()==0 || pose.size() % 2)
	{
		cout << "Polynomial coeff matrix size " << pose.size() << endl;
		ThrowError<logic_error>("Affine transform vector unexpected size",__LINE__,__FILE__);
	}
	if(point.size()!=2)
		ThrowError<logic_error>("Point to transform does not have 2 components",__LINE__,__FILE__);

	vector<double> coeff = GetCoeff(order, point[0], point[1]);

	vector<double> result;
	double tot = 0.0;
	for(unsigned int i=0;i<coeff.size();i++)
	{
		tot += coeff[i] * pose[i];
	}
	result.push_back(tot);
	
	tot=0.0;
	int indexOff = (pose.size() / 2);
	for(unsigned int i=0;i<coeff.size();i++)
	{
		tot += coeff[i] * pose[i+indexOff];
	}

	result.push_back(tot);

	return result;
}

/*vector<double> PolyUnProject(vector<double> point, vector<double>pose) 
{	

}*/

//**********************************************************************

PolyProjection::PolyProjection()
{
	order = 2;
}

PolyProjection::~PolyProjection()
{

}

void PolyProjection::Clear()
{
	originalPoints.clear();
	transformedPoints.clear();
}

void PolyProjection::AddPoint(vector<double> original, vector<double> transformed)
{
	originalPoints.push_back(original);
	transformedPoints.push_back(transformed);
}

void PolyProjection::AddPoint(double ox, double oy, double tx, double ty)
{
	vector<double> temp, temp2;
	temp.push_back(ox);temp.push_back(oy);
	temp2.push_back(tx);temp2.push_back(ty);
	AddPoint(temp,temp2);
}

void PolyProjection::AddPoint(double ox, double oy, vector<double> transformed)
{
	vector<double> temp;
	temp.push_back(ox);temp.push_back(oy);
	AddPoint(temp,transformed);
}

void PrintMatrixP(Matrix &m)
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

vector<double> PolyProjection::Estimate()
{
	if(originalPoints.size()!=transformedPoints.size())
		ThrowError<logic_error>("Inconsistent number of points in constraints",__LINE__,__FILE__);
	if(originalPoints.size()==0)
		ThrowError<logic_error>("Cannot estimate transform with no points",__LINE__,__FILE__);
	try
	{
		
		
	Matrix po((CoeffSize(order)),originalPoints.size());
	for(unsigned int i=0;i<originalPoints.size();i++)
	{
	
		vector<double> coeff = GetCoeff(order, originalPoints[i][0], originalPoints[i][1]);	
		for(unsigned int j=0;j<coeff.size();j++)
		{
			po(j+1,i+1) = coeff[j];
		}
	}
	//PrintMatrixP(po);

	Matrix pt(2,transformedPoints.size());
	for(unsigned int i=0;i<transformedPoints.size();i++)
	{
		for(unsigned int j=0;j<transformedPoints[i].size();j++)
			pt(j+1,i+1) = transformedPoints[i][j];
	}
	//PrintMatrixP(pt);

	Matrix invPo = po.t() * (po * po.t()).i(); //Pseudo inverse
	Matrix ptInvPo = pt * invPo;
	//PrintMatrixP(ptInvPo);

	//Convert 2D matrix into 1D for output
	vector<double>out;
	for(int i=0;i<ptInvPo.Nrows();i++)
	{
		for(int j=0;j<ptInvPo.Ncols();j++)
		{
			out.push_back( ptInvPo(i+1,j+1) );
		}
	}

	//Project input to check for accuracy
	cout << "Projection accuracy check" << endl;
	double totalError = 0.0, totalCount = 0.0;
	for(unsigned int i=0;i<originalPoints.size();i++)
	{
		vector<double> proj = PolyProject(originalPoints[i], out, order);
		cout <<originalPoints[i][0] << "," <<originalPoints[i][1] << "\t";
		cout <<transformedPoints[i][0] << ","<<transformedPoints[i][1] << "\t";
		cout <<proj[0] << "," << proj[1] << "\t";
		double diff = pow(pow(transformedPoints[i][0]-proj[0],2.0)+pow(transformedPoints[i][1]-proj[1],2.0),0.5);
		cout << diff << endl;
		totalError += diff; totalCount ++;
	}
	cout << "Average pixel error " << totalError / totalCount << " units" << endl;

	return out;

	}
	catch(Exception) { cout << Exception::what() << endl; }
	vector<double> empty;
	return empty;
}

