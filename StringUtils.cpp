
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include "StringUtils.h"

string PackBeforeString(string baseString, string pack, int minLength)
{
	string out = "";
	unsigned int baseStrLen = baseString.size();
	for(unsigned int i=0;i<minLength - baseStrLen;i+=pack.size())
		out += pack;
	out += baseString;
	return out;
}

string IntToString(int input)
{
	ostringstream out;
	out << input;
	return out.str();
}

string RemoveTrailingSpaces(string str)
{
	// http://sarathc.wordpress.com/2007/01/31/how-to-trim-leading-or-trailing-spaces-of-string-in-c/

     // Code for Trim trailing Spaces only 
     size_t endpos = str.find_last_not_of(" \t"); // Find the first character position from reverse af 
     if( string::npos != endpos ) 
         str = str.substr( 0, endpos+1 ); 
     
	return str;
}

string RemoveLeadingSpaces(string str)
{
	// http://sarathc.wordpress.com/2007/01/31/how-to-trim-leading-or-trailing-spaces-of-string-in-c/
     size_t startpos = str.find_first_not_of(" \t"); // Find the first character position after excluding leading blank spaces 
     if( string::npos != startpos ) 
         str = str.substr( startpos ); 
	//cout << str << endl;
	return str;
}

string RemoveTrailingWhitespace(string str)
{
	int len = str.size();
	if(len == 0) return str;
	unsigned int i = 0;
	for(i=len-1;i>=0;i--)
	{
		const char *chr = &str.c_str()[i];
		if(*chr != ' '
			&& *chr != '\t'
			&& *chr != '\n')
		{
			break;
		}
	}
	//cout << i << "\t" << len << endl;

	str = str.substr( 0, i+1 ); 

	return str;
}


string RemoveLeadingWhitespace(string str)
{
	int len = str.size();
	if(len == 0) return str;
	int i = 0;
	for(i=0;i<len;i++)
	{
		const char *chr = &str.c_str()[i];
		if(*chr != ' '
			&& *chr != '\t'
			&& *chr != '\n')
		{
			break;
		}
	}
	//cout << i << "\t" << len << endl;

	str = str.substr( i ); 

	return str;
}

string RemoveWhitespace(string str)
{
	return RemoveTrailingWhitespace(RemoveLeadingWhitespace(str));
}

string FilenameRemoveFrameNumAndExtension(string filename)
{
	//cout << "ChangeNumberInFilename input " << filename.c_str() << " " << newNum << endl;
	
	//Remove extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '.')
		{
			extensionOffset = i;
			i = -1;
		}
	}
	if(extensionOffset < 0) //There is no extension
		extensionOffset = len;
	
	//Count numberals before extension
	int startOfNumerals = -1;
	for(int i=extensionOffset-1;i>=0;i--)
	{
		if(filename[i] < '0' || filename[i] > '9')
		{
			startOfNumerals = i + 1;
			i = -1;
		}
	}
	if(startOfNumerals < 0) startOfNumerals = 0;

	//cout << extensionOffset << "\t" << startOfNumerals << endl;
	
	//Build filename
	ostringstream frameFilename;
	for(int i=0;i<startOfNumerals;i++)
	{
		frameFilename << filename[i];
	}

	//frameFilename << filePrefix;
	//int zeroPackedLength = extensionOffset - startOfNumerals;
	//if(zeroPackedLength > 1)
	//	frameFilename << setw(zeroPackedLength) << setfill('0');

	//Add frame number
	//frameFilename << newNum;	

	//Add extension
	//for(int i=extensionOffset;i<len;i++)
	//	frameFilename << filename[i];
		
	return frameFilename.str();
	
	//cout << "ChangeNumberInFilename output " << filename.c_str() << endl;
}

string GetFileExtension(string filename)
{
	//Determine length of extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '.')
		{
			extensionOffset = i+1;
			i = -1;
		}
	}
	if(extensionOffset < 0) //There is no extension
		extensionOffset = len;

	string out;
	out = &(filename.c_str()[extensionOffset]);
	return out;
}

string RemoveFileExtension(string filename)
{
	//Determine length of extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '/') //If found path separator, stop search
		{
			extensionOffset = -1;
			i = -1;
		}

		if(filename[i] == '.')
		{
			extensionOffset = i+1;
			i = -1;
		}
	}

	if(extensionOffset < 0) //There is no extension
	{
		return filename;
		//extensionOffset = len;
	}
	if(extensionOffset > 0) extensionOffset -= 1; //Don't return the . character
	
	return filename.substr(0, extensionOffset);
}

string RemoveFilePath(string filename)
{
	//cout << "x" << endl;
	//Determine length of extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '/')
		{
			extensionOffset = i+1;
			i = -1;
		}
	}
	//cout<<filename << "," << extensionOffset << endl;
	if(extensionOffset < 0) //There is no extension
		return filename;
	
	return filename.substr(extensionOffset, len-extensionOffset);

}

string GetFilePath(string filename)
{
	//cout << "x" << endl;
	//Determine length of extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '/')
		{
			extensionOffset = i+1;
			i = -1;
		}
	}
	//cout<<filename << "," << extensionOffset << endl;
	if(extensionOffset < 0) //There is no extension
		return ".";
	if(extensionOffset > 0) extensionOffset --;
	
	return filename.substr(0,extensionOffset);
}

string FilenameChangeNumber(string filename, int num)
{
	//Remove extension
	int len = strlen(filename.c_str());
	int extensionOffset = -1;
	for(int i=len;i>=0;i--)
	{
		if(filename[i] == '.')
		{
			extensionOffset = i;
			i = -1;
		}
	}
	if(extensionOffset < 0) //There is no extension
		extensionOffset = len;
	
	//Count numberals before extension
	int startOfNumerals = -1;
	for(int i=extensionOffset-1;i>=0;i--)
	{
		if(filename[i] < '0' || filename[i] > '9')
		{
			startOfNumerals = i + 1;
			i = -1;
		}
	}

	if(num==-1 && startOfNumerals >= 0)
	{
		num = atoi(&filename[startOfNumerals]);
	}

	//cout << extensionOffset << "\t" << startOfNumerals << endl;
	
	//Build filename
	ostringstream frameFilename;
	for(int i=0;i<startOfNumerals;i++)
	{
		frameFilename << filename[i];
	}

	//frameFilename << filePrefix;
	int zeroPackedLength = extensionOffset - startOfNumerals;
	if(zeroPackedLength > 1)
		frameFilename << setw(zeroPackedLength) << setfill('0');

	//Add frame number
	frameFilename << num;	

	//Add extension
	for(int i=extensionOffset;i<len;i++)
		frameFilename << filename[i];

	//cout << frameFilename.str() << endl;
	return frameFilename.str();
}

//#define MAX_LINE_LEN 1024
int GetFileAsString(const char *filename, string &dataOut)
{
	ifstream file;
	dataOut.clear();

	file.open(filename);
  	if ( (file.rdstate() & ifstream::failbit ) != 0 )
	{
    		cerr << "Error opening '"<<filename<<"'\n";	
		return -1;
	}

	//cout << this << " file.eof() = " << file.eof()<< "\tfile.good()=" << file.good() << endl;
	/*char line[MAX_LINE_LEN];
	while(!file.eof() && file.good())
	{
		file.getline(line,MAX_LINE_LEN,'\n');
		line[MAX_LINE_LEN-1] = '\0';

		dataOut += line;
	}*/
	file >> dataOut;

	return 1;
}

vector<string> SplitString(string in, string token)
{
	vector<string> out;

	int pos = in.find(token);
	while(pos >= 0)
	{
		string temp(in.begin(),in.begin()+pos);
		string remain(in.begin()+pos+1, in.end());
		out.push_back(temp);

		in = remain;
		pos = in.find(token);
	}
	out.push_back(in);
	
	return out;
}

string ReplaceAllInstances(string input, string findStr, string replaceString)
{
	int pos = input.find(findStr);
	while(pos >= 0)
	{
		input.replace(pos,findStr.length(),replaceString);
		//cout << input << endl;

		int restartPos = pos+replaceString.length();
		//cout << restartPos << endl;
		pos = input.find(findStr,restartPos);	
	}
	return input;	
}

string EscapeCommas(string input)
{
	/*//cout << input << endl;
	int pos = input.find(",");
	while(pos >= 0)
	{
		input.replace(pos,1,"\\,");
		//cout << input << endl;
		pos = input.find(",",pos+2);
	}
	//cout << input << endl; exit(0);
	return input;*/
	return ReplaceAllInstances(input,",","\\,");
}

string UnescapeCommas(string input)
{
	/*//cout << input << endl;
	int pos = input.find("\\,");
	while(pos >= 0)
	{
		input.replace(pos,2,",");
		//cout << input << endl;
		pos = input.find(",",pos+1);
	}
	//cout << input << endl; exit(0);
	//return input;*/
	return ReplaceAllInstances(input,"\\,",",");
}

