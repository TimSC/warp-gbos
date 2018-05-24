
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <sstream>
#include <vector>
using namespace std;

string PackBeforeString(string baseString, string pack, int minLength);
string IntToString(int input);
string RemoveTrailingSpaces(string str);
string RemoveLeadingSpaces(string str);
string RemoveTrailingWhitespace(string str);
string RemoveLeadingWhitespace(string str);
string RemoveWhitespace(string str);
string FilenameRemoveFrameNumAndExtension(string filename);
string FilenameChangeNumber(string filename, int num);
string GetFileExtension(string filename);
string RemoveFileExtension(string filename);
string RemoveFilePath(string filename);
string GetFilePath(string filename);
int GetFileAsString(const char *filename, string &dataOut);
vector<string> SplitString(string in, string token);

string ReplaceAllInstances(string input, string findStr, string replaceString);
string EscapeCommas(string input);
string UnescapeCommas(string input);

template <class T> string ToString(T input)
{
	ostringstream out;
	out << input;
	return out.str();
}

#endif //STRING_UTILS_H

