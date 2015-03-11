#include <arpa/inet.h>
#include <boost/format.hpp>
#include <ifaddrs.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "basic_upnp.hpp"
#include "parser.hpp"
#include "utilities.hpp"

using namespace std;
using namespace tinyxml2;

SimpleUPnP::SimpleUPnP(int time_out_) : time_out(time_out_)
{
	get_igd_location();
}

SimpleUPnP::~SimpleUPnP() { close(tcp_sock); }

void SimpleUPnP::get_igd_location()
{
	/*** Sends a broadcast to discover the Internet Gateway Device Location ***/
	send_broadcast();

    for(int i = 0; i < time_out; ++i) {
    	sleep(1);
        string upnp_response = read_response(udp_broadcast_socket, "Discovery");
        cout << upnp_response << endl;

        if(!upnp_response.empty()) {
            parse_igd_location(upnp_response);
            get_description();
            break;
        }
    }

    close(udp_broadcast_socket);
}

void SimpleUPnP::parse_igd_location(string &upnp_response)
{
    string::size_type begin = upnp_response.find("http://");
    string::size_type end = upnp_response.find("\r", begin);
    string location_url = upnp_response.assign(upnp_response, begin, end-begin);

    string::size_type host_begin, port_begin, path_begin;
    host_begin = location_url.find("://") + 3;
    port_begin = location_url.find(":", host_begin) + 1;
    path_begin = location_url.find("/", port_begin) + 1;

    igd_host = location_url.substr(host_begin, port_begin-host_begin-1);
    string str_port = location_url.substr(port_begin, path_begin-port_begin-1);
    igd_port = stoi(str_port);
    igd_path = location_url.substr(path_begin-1);
}

string SimpleUPnP::AddPortMapping(unsigned short external_port, const string &protocol, unsigned short internal_port,
						const string &internal_client, const string &description, int lease)
{
	string control_parameters = "<NewRemoteHost></NewRemoteHost>\r\n"
								"<NewExternalPort>%1%</NewExternalPort>\r\n"
								"<NewProtocol>%2%</NewProtocol>\r\n"
								"<NewInternalPort>%3%</NewInternalPort>\r\n"
								"<NewInternalClient>%4%</NewInternalClient>\r\n"
								"<NewEnabled>1</NewEnabled>\r\n"
								"<NewPortMappingDescription>%5%</NewPortMappingDescription>\r\n"
								"<NewLeaseDuration>%6%</NewLeaseDuration>";

	string command_arguments = (boost::format(control_parameters) % external_port % protocol % internal_port
																  % internal_client % description % lease).str();
	string request = make_request("AddPortMapping", command_arguments);

	return exec_command("AddPortMapping", request);

}

string SimpleUPnP::DeletePortMapping(unsigned short external_port, const string &protocol)
{
	string control_parameters = "<NewRemoteHost></NewRemoteHost>\r\n"
								"<NewExternalPort>%1%</NewExternalPort>\r\n"
								"<NewProtocol>%2%</NewProtocol>";

	string command_arguments = (boost::format(control_parameters) % external_port % protocol).str();

	string request = make_request("DeletePortMapping", command_arguments);
	return exec_command("DeletePortMapping", request);
}

string SimpleUPnP::GetConnectionTypeInfo()
{
	string command_arguments = "<NewConnectionType></NewConnectionType>\r\n"
								"<NewPossibleConnectionTypes></NewPossibleConnectionTypes>";

	string request = make_request("GetConnectionTypeInfo", command_arguments);
    return exec_command("GetConnectionTypeInfo", request);
}

string SimpleUPnP::SetConnectionType(string connection_type)
{
    string control_parameters = "<NewConnectionType>%1%</NewConnectionType>\r\n";

    string command_arguments = (boost::format(control_parameters) % connection_type).str();
	string request = make_request("SetConnectionType", command_arguments);
	return exec_command("SetConnectionType", request);
   
}

string SimpleUPnP::GetSpecificPortMappingEntry(unsigned short external_port, const string &protocol)
{

	string control_parameters = "<NewRemoteHost></NewRemoteHost>\r\n"
								"<NewExternalPort>%1%</NewExternalPort>\r\n"
								"<NewProtocol>%2%</NewProtocol>\r\n"
								"<NewInternalPort></NewInternalPort>\r\n"
								"<NewInternalClient></NewInternalClient>\r\n"
								"<NewEnabled></NewEnabled>\r\n"
								"<NewPortMappingDescription></NewPortMappingDescription>\r\n"
								"<NewLeaseDuration></NewLeaseDuration>";

	string command_arguments = (boost::format(control_parameters) % external_port % protocol).str();
	string request = make_request("GetSpecificPortMappingEntry", command_arguments);
	return exec_command("GetSpecificPortMappingEntry", request);
}

string SimpleUPnP::GetStatusInfo()
{
	string command_arguments =  "<NewConnectionStatus></NewConnectionStatus>\r\n"
								"<NewLastConnectionError></NewLastConnectionError>\r\n"
								"<NewUptime></NewUptime>";

	string request = make_request("GetStatusInfo", command_arguments);
	return exec_command("GetStatusInfo", request);
}

string SimpleUPnP::GetExternalIPAddress()
{
	string command_arguments = "<NewExternalIPAddress></NewExternalIPAddress>";

	string request = make_request("GetExternalIPAddress", command_arguments);
	return exec_command("GetExternalIPAddress", request);
}

string SimpleUPnP::GetNatRSIPStatus()
{
	string command_arguments = "<NewRSIPAvailable></NewRSIPAvailable>\r\n"	\
								"<NewNATEnabled></NewNATEnabled>\r\n";

	string request = make_request("GetNATRSIPStatus", command_arguments);
	return exec_command("GetNATRSIPStatus", request);
}

string SimpleUPnP::ForceTermination()
{
	string request = make_request("ForceTermination", "");
	return exec_command("ForceTermination", request);
}

string SimpleUPnP::RequestTermination()
{
	string request = make_request("RequestTermination", "");
	return exec_command("RequestTermination", request);
}

string SimpleUPnP::RequestConnection()
{
	string request = make_request("RequestConnection", "");
	return exec_command("RequestConnection", request);
}

void SimpleUPnP::send_broadcast()
{
    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS.c_str());
    broadcast_addr.sin_port = htons(BROADCAST_PORT);

    udp_broadcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char opt_val = 1;
    setsockopt(udp_broadcast_socket, SOL_SOCKET, SO_BROADCAST, &opt_val, sizeof(char));
    fcntl(udp_broadcast_socket, F_SETFL, fcntl(udp_broadcast_socket, F_GETFL) | O_NONBLOCK);

    std::string SEARCH_REQUEST_STRING = "M-SEARCH * HTTP/1.1\r\n"
    									"ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
    									"MX: 2\r\n"
    									"MAN:\"ssdp:discover\"\r\n"
    									"HOST: 239.255.255.250:1900\r\n"
    									"\r\n";

    sendto(udp_broadcast_socket, SEARCH_REQUEST_STRING.c_str(), SEARCH_REQUEST_STRING.size(), 0, (struct sockaddr*)&broadcast_addr, sizeof(struct sockaddr_in));
}

void SimpleUPnP::prepare_tcp_socket()
{
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in upnp_address;
    upnp_address.sin_family = AF_INET;
    upnp_address.sin_addr.s_addr = inet_addr(igd_host.c_str());
    upnp_address.sin_port = htons(igd_port);
    int ret = connect(tcp_sock, (struct sockaddr*)&upnp_address, sizeof(struct sockaddr_in));
}

string SimpleUPnP::exec_command(const string &command, const string &request)
{
    prepare_tcp_socket();
    write(tcp_sock, request.c_str(), request.length());

    string response;
    int count = time_out;
    while(count--)
    {
        response = read_response(tcp_sock, command);
        if(!response.empty())
        	break;
    }

    close(tcp_sock);

    return response;
}

string  SimpleUPnP::make_soap_envelope(const string &command, const string &request)
{
	const std::string SOAP_ACTION = "<?xml version=\"1.0\"?>\r\n"
									"<s:Envelope xmlns:s="
									"\"http://schemas.xmlsoap.org/soap/envelope/\" "
									"s:encodingStyle="
									"\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
									"<s:Body>\r\n"
									"<u:%1% xmlns:u=\"%2%\">\r\n%3%\r\n"
									"</u:%4%>\r\n"
									"</s:Body>\r\n"
									"</s:Envelope>\r\n\r\n";

	std::cout << WANIPConnection << std::endl;
	string serviceType = WANIPConnection->FirstChildElement("serviceType")->GetText();
	std::cout << "soap" << std::endl;
	return (boost::format(SOAP_ACTION) % command % serviceType % request % command).str();
}

string SimpleUPnP::make_http_header(const string command, int soap_envelope_size)
{
	const std::string HTTP_HEADER_ACTION = "POST %1% HTTP/1.1\r\n"
										   "Host: %2%:%3%\r\n"
										   "User-Agent: Linux/3.14.4-1-ARCH, UPnP/1.0, MiniUPnPc/\r\n"
										   "Content-Length: %4%\r\n"
										   "Content-Type: text/xml\r\n"
										   "SOAPAction: \"%5%#%6%\"\r\n"
										   "Connection: Close\r\n"
										   "Cache-Control: no-cache\r\n"
										   "Pragma: no-cache\r\n\r\n";


	std::cout << "http" << std::endl;
	string serviceType = WANIPConnection->FirstChildElement("serviceType")->GetText();
	std::cout << "http" << std::endl;
	return (boost::format(HTTP_HEADER_ACTION) % controlURL % igd_host % igd_port % soap_envelope_size % serviceType % command).str();
}

string SimpleUPnP::make_request(const string &command, const string &command_arguments)
{
	string soap_envelope = make_soap_envelope(command, command_arguments);
	string http_header = make_http_header(command, soap_envelope.size());

	string request = http_header + soap_envelope;
    return request;
}

void SimpleUPnP::get_description()
{
    std::string request = (boost::format("GET %1% HTTP/1.1\r\nHost: %2%:%3%\r\n\r\n") % igd_path % igd_host % igd_port).str();

    prepare_tcp_socket();
    write(tcp_sock, request.c_str(), request.length());

    read_response(tcp_sock, "Description");

    close(tcp_sock);
}

void SimpleUPnP::parse_description(const string &xml_doc)
{
	parser = new UPnPParser(xml_doc);
	map<string, tinyxml2::XMLElement*> devices = parser->parse_description();

	WANDevice = devices["WANDevice"];
	WANConnectionDevice = devices["WANConnectionDevice"];
	WANIPConnection = devices["WANIPConnection"];

	controlURL = WANIPConnection->FirstChildElement("controlURL")->GetText();
}

vector<pair<string, string>> SimpleUPnP::DeviceInfo()
{
	vector<pair<string, string>> info = parser->get_device_info();
	return info;
}

string SimpleUPnP::read_response(int sock, const string &command)
{
    char buffer[BUFFER_SIZE];
    string response {};
    while(read(sock, buffer, BUFFER_SIZE) > 0) {
        response += buffer;
        memset(buffer, 0, BUFFER_SIZE);
    }

    if (command == "Description")
    	parse_description(response);


    string ret = parse_response(response, command);

    if(ret.empty())
    	return "";

    return ret;
}

string SimpleUPnP::parse_response(const string response, const string &command)
{
	size_t end_http_header = response.find("<?xml");
	if (end_http_header != string::npos) {
		string http_header = response.substr(0, end_http_header);

		string xml_doc = response.substr(end_http_header);
    	XMLDocument xml_response;
    	xml_response.Parse(xml_doc.c_str());
    	XMLElement *Envelope = xml_response.FirstChildElement("SOAP-ENV:Envelope");

    	if(Envelope) {
	    	XMLElement *Body = Envelope->FirstChildElement("SOAP-ENV:Body");
	    	if (Body) {
	    		XMLElement *request_name = Body->FirstChildElement();
	    		if(request_name) {
	    			cout << request_name->Value() << endl;
	    			XMLElement *return_val = request_name->FirstChildElement();
	    			while (return_val) {
	    				cout << "\t-> " << return_val->Name() << ": " << return_val->GetText() << endl;
	    				return_val = return_val->NextSiblingElement();
	    			}
	    		}
	    	}
	    }
	}

    return response;
}
