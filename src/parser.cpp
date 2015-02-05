#include "parser.hpp"
#include <stdexcept>
#include <iostream>

#include "tinyxml2.h"

using namespace std;

tinyxml2::XMLElement* UPnPParser::parser(tinyxml2::XMLElement* node, string device_name, string service_device)
{
	tinyxml2::XMLElement *device = node->FirstChildElement((service_device + "List").c_str())->FirstChildElement();
	string wan_device = device->FirstChildElement((service_device + "Type").c_str())->GetText();
	while(wan_device.find(device_name) == string::npos) {
		device = device->NextSiblingElement();

		if(!device)
			throw runtime_error(device_name + " not found");

		wan_device = device->FirstChildElement((service_device + "Type").c_str())->GetText();
	}

	return device;
}

std::map<string, tinyxml2::XMLElement*> UPnPParser::parse_description(const string &xml_doc)
{
	igd_xml.Parse(xml_doc.c_str());

	tinyxml2::XMLElement *root = igd_xml.RootElement();

	InternetGatewayDevice = root->FirstChildElement("device");

	WANDevice = parser(InternetGatewayDevice, "WANDevice", "device");
	WANConnectionDevice = parser(WANDevice, "WANConnectionDevice", "device");
	WANIPConnection = parser(WANConnectionDevice, "WANIPConnection", "service");

	std::map<string, tinyxml2::XMLElement*> devices = {
			{"WANDevice", WANDevice},
			{"WANConnectionDevice", WANConnectionDevice},
			{"WANIPConnection", WANIPConnection}
	};

	return devices;
}
