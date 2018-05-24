
#include <Python.h>
#include <iostream>
#include <vector>
using namespace std;
#include "PyOstn02.h"

PyOstn02::PyOstn02()
{
	// Initialize the Python Interpreter
	Py_Initialize();

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"pyostn02\")"); 

	PyObject *pName, *pModule, *pDict;

	// Build the name object
	pName = PyString_FromString("transform");

	// Load the module object
	pModule = PyImport_Import(pName);
	if (!pModule)
	{cout << "Error importing module" << endl;PyErr_Print();throw(0);}
	Py_DECREF(pName);

	pDict = PyModule_GetDict(pModule);
	Py_DECREF(pModule);

	this->pFunc = PyDict_GetItemString(pDict, "TransformOsgbToWgs84");
	if(!this->pFunc)
	{cout << "No such function in dict" << endl;PyErr_Print();throw(1);}
}

PyOstn02::~PyOstn02()
{
	if(this->pFunc) Py_DECREF(this->pFunc);

	// Finish the Python Interpreter
	Py_Finalize();	
}

int PyOstn02::OsgbToWgs84(vector<double> &out, double ea, double no, double ele=0.0)
{
	out.clear();

	PyObject * easting = PyFloat_FromDouble(ea);
	PyObject * northing = PyFloat_FromDouble(no);
	PyObject * elev = PyFloat_FromDouble(ele);
	PyObject * pArgs = PyTuple_New(3);
        PyTuple_SetItem(pArgs, 0, easting);   
	PyTuple_SetItem(pArgs, 1, northing);
	PyTuple_SetItem(pArgs, 2, elev);
	
	//if (PyCallable_Check(pFunc))     
        PyObject *ret = PyObject_CallObject(pFunc, pArgs);
	Py_DECREF(easting);
	Py_DECREF(northing);
	Py_DECREF(elev);
	Py_DECREF(pArgs);	
	
	if (!ret) {PyErr_Print(); return -3;}
	if (ret == Py_None) {return -4;}
	if (!PyTuple_Check(ret)) {cout << "Error: expecting tuple" << endl; return -1;}
	if (PyTuple_Size(ret) < 3) {cout << "Error: Tuple too small" << endl; return -2;}

	out.push_back(PyFloat_AsDouble(PyTuple_GetItem(ret,0)));
	out.push_back(PyFloat_AsDouble(PyTuple_GetItem(ret,1)));
	out.push_back(PyFloat_AsDouble(PyTuple_GetItem(ret,2)));
	Py_DECREF(ret);

	return 1;
}

void PyOstn02::ConvertGbos1936ToWgs84(double ea, double no, double he,
	double &latOut, double &lonOut, double &heOut)
{
	latOut = 0.0; lonOut = 0.0; heOut = 0.0;
	vector<double> out;
	int ret = OsgbToWgs84(out, ea, no, he);
	if(ret <= 0)
	{
		cout << "Conversion failed at " << ea << "," << no << "," << he << endl;
		throw(0);
	}
	latOut = out[0];
	lonOut = out[1];
	heOut = out[2];
	return;
}

void PyOstn02::ConvertWgs84ToGbos1936(double lat, double lon, double he,
	double &eaOut, double &noOut, double &heOut)
{
	cout << "function not implemented" << endl;
	throw(0);
}

/*int main()
{
	class PyOstn02 con;
	vector<double> out;
	con.OsgbToWgs84(out,276000,900000);
	cout << "out " << out[0] << "," << out[1] << "," << out[2] << endl;
}*/

