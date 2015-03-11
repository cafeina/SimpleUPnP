#include <map>
#include <string>
#include <utility>
#include <vector>

#include "parser.hpp"
#include "tinyxml2.h"

const int BUFFER_SIZE = 4096;
const std::string BROADCAST_ADDRESS = "239.255.255.250";
const unsigned short BROADCAST_PORT = 1900;

class AddressList
{
public:
	void add(const std::string &family, const std::string &interface, const std::string &addr)
	{
		std::pair<std::string, std::string> interface_and_address = std::make_pair(interface, addr);
		addresses[family].push_back(interface_and_address);
	}

	void list()
	{
		for(auto element : addresses) {
			std::cout << element.first << std::endl;
			for(auto zimba : element.second) {
				std::cout << "\t" << zimba.first << std::endl;
				std::cout << "\t" << zimba.second << std::endl;
			}
		}
	}

private:
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> addresses;
};

class SimpleUPnP
{
public:
    SimpleUPnP(int=3);
    ~SimpleUPnP();

    std::string GetConnectionTypeInfo();
    std::string AddPortMapping(unsigned short, const std::string&, unsigned short, const std::string&, const std::string&, int);
    std::string DeletePortMapping(unsigned short, const std::string&);
    std::string ForceTermination();
    std::string GetStatusInfo();
    std::string GetNatRSIPStatus();
    std::string GetExternalIPAddress();
    std::string GetSpecificPortMappingEntry(unsigned short, const std::string&);
    std::string RequestTermination();
    std::string RequestConnection();
    std::string SetConnectionType(std::string);

    std::vector<std::pair<std::string, std::string>> DeviceInfo();

private:
    void get_igd_location();
    void parse_igd_location(std::string&);
    std::string make_soap_envelope(const std::string&, const std::string&);
    std::string make_http_header(const std::string, int);
    void prepare_tcp_socket();
	void send_broadcast();
	std::string exec_command(const std::string&, const std::string&);
	std::string make_request(const std::string&, const std::string&);
	void get_description();
	void parse_description(const std::string&);
	std::string read_response(int, const std::string&);
	std::string parse_response(const std::string, const std::string&);

private:
    tinyxml2::XMLDocument igd_xml;
    tinyxml2::XMLElement *InternetGatewayDevice;
	tinyxml2::XMLElement *WANDevice;
	tinyxml2::XMLElement *WANConnectionDevice;
	tinyxml2::XMLElement *WANIPConnection;

	UPnPParser *parser;

    unsigned short igd_port;
    std::string igd_host;
    std::string igd_path;
    std::string controlURL;
    
    int tcp_sock;
    int udp_broadcast_socket;
    int time_out;
};




