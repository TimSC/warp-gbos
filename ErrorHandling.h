
#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdexcept>
#include <sstream>
using namespace std;

//Example: ThrowError<logic_error>("Not enough stuff", __LINE__, __FILE__);

template<class T> void ThrowError(const char *errStr, int line, const char *file)
{
	ostringstream out;
	out << errStr;
	out << " in ";
	out << file;
	out << ":";
	out << line;
	T errObj(out.str());
	throw(errObj);
}

template<class T> void ThrowError(const char *err)
{
	T errObj(err);
	throw(errObj);
}

#endif //ERROR_HANDLING_H

