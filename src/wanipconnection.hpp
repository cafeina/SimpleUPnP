#ifndef WANIPCONNECTION_HPP
#define WANIPCONNECTION_HPP

#include <string>

class WANIPConnection
{
public:
	WANIPConnection();


private:

	std::string GetConnectionTypeInfo();
	std::string AddPortMapping(unsigned short, const std::string&, unsigned short, const std::string&, const std::string&, int);
	std::string DeletePortMapping(unsigned short, const std::string&);
	std::string GetStatusInfo();
	std::string GetNatRSIPStatus();
	std::string GetExternalIPAddress();
	std::string GetSpecificPortMappingEntry(unsigned short, const std::string&);
	std::string SetConnectionType(std::string);
	std::string controlURL;
	std::string serviceType;
};

#endif
