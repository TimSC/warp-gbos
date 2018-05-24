
#include "ReadDelimitedFile.h"
#include <stdlib.h>
#include <string.h>
#include "ErrorHandling.h"
#include "StringUtils.h"

DelimitedFileValue::DelimitedFileValue()
{

}

DelimitedFileValue::DelimitedFileValue(const DelimitedFileValue &a)
{
	operator=(a);
}

DelimitedFileValue::~DelimitedFileValue()
{

}

class DelimitedFileValue &DelimitedFileValue::operator=(const DelimitedFileValue &a)
{
	data = a.data;
	return *this;
}

double DelimitedFileValue::GetVald()
{
	return atof(data.c_str());
}

int DelimitedFileValue::GetVali()
{
	return atoi(data.c_str());
}

const char *DelimitedFileValue::GetVals()
{
	return data.c_str();
}

void DelimitedFileValue::SetVald(double in)
{
	data = ToString<double>(in);
}

void DelimitedFileValue::SetVals(const char *in)
{
	data = in;
}

void DelimitedFileValue::ProcessVal(const char *buffer, int size)
{
	/*cout << "\"";
	for(unsigned int i=0;i<size;i++)
	{
		cout << buffer[i];
	}
	cout << "\"" << endl;*/
	data.append(buffer,size);
}

//**************************************************************

DelimitedFileLine::DelimitedFileLine()
{

}

DelimitedFileLine::DelimitedFileLine(const class DelimitedFile &a)
{
	operator=(a);
}

DelimitedFileLine::~DelimitedFileLine()
{

}

class DelimitedFileLine &DelimitedFileLine::operator=(const class DelimitedFileLine &a)
{
	vals = a.vals;
	return *this;
}

unsigned int DelimitedFileLine::NumVals()
{
	return vals.size();
}

class DelimitedFileValue &DelimitedFileLine::GetVal(int num)
{
	if(num < 0 || (unsigned int)num >= vals.size()) ThrowError<invalid_argument>("Invalid value number",__LINE__,__FILE__);
	return vals[num];
}

class DelimitedFileValue &DelimitedFileLine::operator[](int num)
{
	if(num < 0 || (unsigned int)num >= vals.size()) ThrowError<invalid_argument>("Invalid value number",__LINE__,__FILE__);
	return vals[num];
}

void DelimitedFileLine::ProcessLine(const char *line, string &delimiter)
{
	if(delimiter.size() == 0) { ThrowError<invalid_argument>("Delimiter not set",__LINE__,__FILE__);}
	//cout << line << endl;
	//cout << "search ";
	int pos = 0;
	int prevPos = 0;
	const char *del = strstr(&line[pos], delimiter.c_str());
	while(del != NULL)
	{
		//cout << (del - line) << " " <<  endl;
		pos = (del - line) + 1;
		int delimitedComma = 0;
		if(pos-2 >= 0 && line[pos-2] == '\\') {delimitedComma=1;} //Ignore delimited commas

		if(!delimitedComma)
		{
			class DelimitedFileValue newval;
			newval.ProcessVal(&line[prevPos], pos - prevPos - 1);
			this->vals.push_back(newval);

			prevPos = pos;
		}
		del = strstr(&line[pos], delimiter.c_str());
	}
	//cout << "total len" << strlen(line) << endl;
	
	pos = strlen(line);
	class DelimitedFileValue newval;
	newval.ProcessVal(&line[prevPos], pos - prevPos);
	this->vals.push_back(newval);	
}


//******************************************************************

DelimitedFile::DelimitedFile()
{
	delimiter = ",";
}

DelimitedFile::DelimitedFile(const class DelimitedFile &a)
{
	operator=(a);
}

DelimitedFile::~DelimitedFile()
{

}

class DelimitedFile &DelimitedFile::operator=(const class DelimitedFile &a)
{
	lines = a.lines;
	delimiter = a.delimiter;
	return *this;
}

#define MAX_LINE_LEN (1024 * 10)
int DelimitedFile::Open(const char *filename)
{
	lines.clear();

	ifstream file;
	file.open(filename);
  	if ( (file.rdstate() & ifstream::failbit ) != 0 )
	{
    		cout << "Error opening '"<<filename<<"'\n";	
		return -1;
	}

	char line[MAX_LINE_LEN];
	while(!file.eof() && file.good())
	{
		file.getline(line,MAX_LINE_LEN,'\n');
		line[MAX_LINE_LEN-1] = '\0';

		class DelimitedFileLine newline;
		newline.ProcessLine(line, delimiter);

		lines.push_back(newline);
		//cout << line << endl;
	}
	file.close();
	return 1;
}

unsigned int DelimitedFile::NumLines()
{
	return lines.size();
}

class DelimitedFileLine &DelimitedFile::GetLine(int num)
{
	if(num < 0 || num >= (int)NumLines()) ThrowError<invalid_argument>("Invalid line number",__LINE__,__FILE__);
	return lines[num];
}

class DelimitedFileLine &DelimitedFile::operator[](int num)
{
	if(num < 0 || num >= (int)NumLines()) ThrowError<invalid_argument>("Invalid line number",__LINE__,__FILE__);
	return lines[num];
}

vector<double> ExtractColumnFromDelimitedFile(class DelimitedFile &file, int colNum)
{
	vector<double> output;
	unsigned int numLines = file.NumLines();
	for(unsigned int i=0;i<numLines;i++)
	{
		class DelimitedFileLine &line = file.GetLine(i);
		if(colNum < (int)line.NumVals())
			output.push_back(line.GetVal(colNum).GetVald());
		else
			output.push_back(0.0);
	}
	return output;
}

void CopyColumnFromDelimitedFile(class DelimitedFile &file, int colNum, class DelimitedFile &out)
{
	vector<double> output;
	unsigned int numLines = file.NumLines();

	//Ensure output file has same number of lines
	while(out.NumLines() < numLines)
	{
		class DelimitedFileLine templine;
		out.lines.push_back(templine);
	}

	for(unsigned int i=0;i<numLines;i++)
	{
		class DelimitedFileLine &inLine = file.GetLine(i);
		class DelimitedFileLine &outLine = out.GetLine(i);
		
		if(colNum < (int)inLine.NumVals())
		{
			class DelimitedFileValue tempVal;
			tempVal.SetVals(inLine.GetVal(colNum).GetVals());
			outLine.vals.push_back(tempVal);
		}
		else
		{
			class DelimitedFileValue tempVal;
			outLine.vals.push_back(tempVal);
		}
	}
}

void CopyColumnFromDelimitedFileVec(vector<class DelimitedFile> &files, 
	int colNum, vector<class DelimitedFile> &out)
{
	//Ensure output has same size
	while(out.size() < files.size())
	{
		class DelimitedFile temp;
		out.push_back(temp);
	}

	//Copy data
	for(unsigned int i=0;i<files.size();i++)
	{
		CopyColumnFromDelimitedFile(files[i], colNum, out[i]);
	}

}

//***************************************************************


TextFile::TextFile()
{
	
}

TextFile::TextFile(const class TextFile &a)
{
	operator=(a);
}

TextFile::~TextFile()
{
	Close();
}

class TextFile &TextFile::operator=(const class TextFile &a)
{

	return *this;
}

int TextFile::Open(const char *filename)
{
	file.open(filename);
  	if ( (file.rdstate() & ifstream::failbit ) != 0 )
	{
    		cerr << "Error opening '"<<filename<<"'\n";
		return -1;
	}
	return 1;
}

int TextFile::GetLine(string &line)
{
	if(file.eof()) return 0;
	//string line;
	if(!file.eof() && file.good())
	{
		getline(file, line);

		//ProcessLine(line.c_str());
		//cout << line << endl;
		return 1;
	}
	return -1;	
}

void TextFile::Close()
{
	file.close();
}

//*****************************************************************

DelimitedFileStreamCallback::DelimitedFileStreamCallback()
{

}

DelimitedFileStreamCallback::~DelimitedFileStreamCallback()
{

}

void DelimitedFileStreamCallback::Val(int col, const char *buffer, int len)
{

}

//****************************************************************

DelimitedFileStream::DelimitedFileStream()
{
	delimiter = ",";
}

DelimitedFileStream::DelimitedFileStream(const class DelimitedFile &a)
{

}

DelimitedFileStream::~DelimitedFileStream()
{
	file.close();
}

class DelimitedFileStream &DelimitedFileStream::operator=(const class DelimitedFileStream &a)
{
	delimiter = a.delimiter;
	//file = a.file;
	return *this;
}

int DelimitedFileStream::Open(const char *filename)
{
	file.open(filename);
  	if ( (file.rdstate() & ifstream::failbit ) != 0 )
	{
    		cout << "Error opening '"<<filename<<"'\n";	
		return -1;
	}
	return 1;
}

void DelimitedFileStream::ToStart()
{
	//cout << "DelimitedFileStream::ToStart()" << endl;
	file.seekg(0,ios::beg);
	file.clear();
	//cout << this << " file.eof() = " << file.eof()<< "\tfile.good()=" << file.good() << endl;
}

int DelimitedFileStream::GetLine(class DelimitedFileStreamCallback &callback)
{
	//cout << this << " file.eof() = " << file.eof()<< "\tfile.good()=" << file.good() << endl;
	char line[MAX_LINE_LEN];
	if(!file.eof() && file.good())
	{
		file.getline(line,MAX_LINE_LEN,'\n');
		line[MAX_LINE_LEN-1] = '\0';

		ProcessLine(line, callback);
		return 1;
	}
	//cout << this << " file.eof() = " << file.eof()<< "\tfile.good()=" << file.good() << endl;
	file.seekg(0,ios::beg); //This should be in ToStart() function. Bug. Really annoying too.
	file.clear();
	//cout << "zzfile.eof() = " << file.eof()<< "\tfile.good()=" << file.good() << endl;
	return -1;
}

int DelimitedFileStream::GetAllLines(class DelimitedFileStreamCallback &callback)
{
	ToStart();
	int running = 1;
	while(running)
	{
		int ret = GetLine(callback);
		if (ret == 0) running = 0;
		if (ret < 0) return ret;
	}
	return 1;
}

void DelimitedFileStream::ProcessLine(const char *line,
		class DelimitedFileStreamCallback &callback)
{
	//cout << line << endl;
	
	//cout << "search ";
	int pos = 0;
	int prevPos = 0;
	int col = 0;
	const char *del = strstr(&line[pos], delimiter.c_str());
	while(del != NULL)
	{
		//cout << (del - line) << " " <<  endl;
		pos = (del - line) + 1;
		callback.Val(col,&line[prevPos], pos - prevPos - 1);
		col ++;

		prevPos = pos;
		del = strstr(&line[pos], delimiter.c_str());
	}
	//cout << "total len" << strlen(line) << endl;
	
	pos = strlen(line);
	callback.Val(col,&line[prevPos], pos - prevPos);
	col++;
	callback.EndOfLine();
}

//*************************************************************

int SeekInDelimitedFile(class DelimitedFile &file, int firstColValue, class DelimitedFileLine &lineOut)
{
	lineOut.vals.clear();
	int lines = file.NumLines();
	for(int i=0;i<lines;i++)
	{
		class DelimitedFileLine &line = file[i];
		if(line.NumVals() < 1) continue;
		int firstVal = line[0].GetVali();
		if(firstVal == firstColValue)
		{
			//cout << "Found " << firstColValue << endl;
			lineOut = line;
			return 1;
		}
	}
	return -1;
}

