#include "wanipconnection.hpp"

using namespace std;

std::string SimpleUPnP::AddPortMapping(unsigned short external_port, const string &protocol, unsigned short internal_port,
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

	//string request = make_request("AddPortMapping", command_arguments);
	//std::cout << request << std::endl;
	//return exec_command("AddPortMapping", request);
	return command_arguments;
}
