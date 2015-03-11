#include "parser.hpp"
#include <stdexcept>
#include <iostream>

#include "tinyxml2.h"

using namespace std;

UPnPParser::UPnPParser(const string &xml_doc)
{
	igd_xml.Parse(xml_doc.c_str());
}

tinyxml2::XMLElement* UPnPParser::parse(tinyxml2::XMLElement* node, string device_name, string service_device)
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

std::map<string, tinyxml2::XMLElement*> UPnPParser::parse_description()
{
	tinyxml2::XMLElement *root = igd_xml.RootElement();

	InternetGatewayDevice = root->FirstChildElement("device");

	WANDevice = parse(InternetGatewayDevice, "WANDevice", "device");
	WANConnectionDevice = parse(WANDevice, "WANConnectionDevice", "device");
	WANIPConnection = parse(WANConnectionDevice, "WANIPConnection", "service");

	map<string, tinyxml2::XMLElement*> devices = {
			{"WANDevice", WANDevice},
			{"WANConnectionDevice", WANConnectionDevice},
			{"WANIPConnection", WANIPConnection}
	};

	return devices;
}

vector<pair<string, string>> UPnPParser::get_device_info()
{
	tinyxml2::XMLElement *root = igd_xml.RootElement();
	tinyxml2::XMLElement *internet_gateway_device = root->FirstChildElement("device");

	vector<pair<string, string>> info;

	internet_gateway_device = internet_gateway_device->FirstChildElement();
	while(internet_gateway_device)
	{
		internet_gateway_device = internet_gateway_device->NextSiblingElement();
		if(internet_gateway_device && internet_gateway_device->GetText()) {
			pair<string, string> tag_text = make_pair<string, string>
							(internet_gateway_device->Value(),
							 internet_gateway_device->GetText());

			info.push_back(tag_text);
		}
	}

	return info;
}
