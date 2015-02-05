#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <map>
#include "tinyxml2.h"

class UPnPParser
{
public:
	std::map<std::string, tinyxml2::XMLElement*> parse_description(const std::string&);
	tinyxml2::XMLElement* parser(tinyxml2::XMLElement*, std::string, std::string);

private:
	tinyxml2::XMLDocument igd_xml;
    tinyxml2::XMLElement *InternetGatewayDevice;
	tinyxml2::XMLElement *WANDevice;
	tinyxml2::XMLElement *WANConnectionDevice;
	tinyxml2::XMLElement *WANIPConnection;
};
#endif
