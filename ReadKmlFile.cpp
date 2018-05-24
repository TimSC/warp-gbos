
#include "ReadKmlFile.h"

//pkg-config libxml++-2.6 --cflags --libs
#include <libxml++/libxml++.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;

void print_indentation(unsigned int indentation)
{
  //for(unsigned int i = 0; i < indentation; ++i)
  //  std::cout << " ";
}

class ProcessKml
{
public:
	string imgFilename;
	class Tile tile;

	ProcessKml() {};
	void ProcessNode(const xmlpp::Node* node, vector<string> nodeNames, unsigned int indentation = 0);
};

void ProcessKml::ProcessNode(const xmlpp::Node* node, vector<string> nodeNames, unsigned int indentation)
{
  //std::cout << std::endl; //Separate nodes by an empty line.
  
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
    return;
    
  Glib::ustring nodename = node->get_name();

  if(!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
  {
	nodeNames.push_back(node->get_name());
    print_indentation(indentation);
    //std::cout << "Node name = " << node->get_name() << std::endl;
    //std::cout << "Node name = " << nodename << std::endl;
  }
  else if(nodeText) //Let's say when it's text. - e.g. let's say what that white space is.
  {
	//for(unsigned int i=0;i<nodeNames.size();i++)
	//{
	//	cout << nodeNames[i] << ",";
	//}
	//cout << endl;
    print_indentation(indentation);
    //std::cout << "Text Node" << std::endl;
  }

  //Treat the various node types differently: 
  if(nodeText)
  {
    print_indentation(indentation);
    //std::cout << "text = \"" << nodeText->get_content() << "\"" << std::endl;

	vector<string> pathImgFilename;
	pathImgFilename.push_back("kml");
	pathImgFilename.push_back("Folder");
	pathImgFilename.push_back("GroundOverlay");
	pathImgFilename.push_back("Icon");
	pathImgFilename.push_back("href");

	if(pathImgFilename == nodeNames)
	{
		imgFilename = nodeText->get_content();
	}

	vector<string> latLonBoxPath;
	latLonBoxPath.push_back("kml");
	latLonBoxPath.push_back("Folder");
	latLonBoxPath.push_back("GroundOverlay");
	latLonBoxPath.push_back("LatLonBox");
	int match = 1;
	//cout << latLonBoxPath.size() << endl;
	if(nodeNames.size() != 5) match = 0;
	else
	for(unsigned int i=0;i<latLonBoxPath.size() && i<nodeNames.size();i++)
	{
		if(latLonBoxPath[i] != nodeNames[i]) {match = 0; break;}
	}
	//cout << "match" << match << endl;
	if(match)
	{
		double val = atof(nodeText->get_content().c_str());
		if(nodeNames[4] == "west") {tile.lonmin = val; }
		if(nodeNames[4] == "east") {tile.lonmax = val; }
		if(nodeNames[4] == "south") {tile.latmin = val; }
		if(nodeNames[4] == "north") {tile.latmax = val;}
	}


  }
  else if(nodeComment)
  {
    print_indentation(indentation);
    //std::cout << "comment = " << nodeComment->get_content() << std::endl;
  }
  else if(nodeContent)
  {
    print_indentation(indentation);
    //std::cout << "content = " << nodeContent->get_content() << std::endl;
  }
  else if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node))
  {
    //A normal Element node:

    //line() works only for ElementNodes.
    print_indentation(indentation);
    //std::cout << "     line = " << node->get_line() << std::endl;

    //Print attributes:
    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
    {
     // const xmlpp::Attribute* attribute = *iter;
     // print_indentation(indentation);
     // std::cout << "  Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
    }

    const xmlpp::Attribute* attribute = nodeElement->get_attribute("title");
    if(attribute)
    {
      //std::cout << "title found: =" << attribute->get_value() << std::endl;
      
    }
  }
  
  if(!nodeContent)
  {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    {
	
      ProcessNode(*iter, nodeNames, indentation + 2); //recursive
    }
  }
}


int ReadKmlFile(const char *filename, class Tile &tileOut, string &imgFilenameOut)
{
	imgFilenameOut = "";

	try
	{
	xmlpp::DomParser parser;
	//parser.set_validate();
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(filename);

	if(parser)
	{
		//Walk the tree:
		const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.

		class ProcessKml processKml;
		vector<string> empty;
		processKml.ProcessNode(pNode, empty);

		imgFilenameOut = processKml.imgFilename;
		tileOut = processKml.tile;
	}
	}
	catch(const std::exception& ex)
	{
		std::cout << "Exception caught: " << ex.what() << std::endl;
		return -1;
	}

	return 1;
}

