#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "basic_upnp.h"

using std::string;

SimpleUPnP::SimpleUPnP(int time_out) : m_time_out(time_out)
{

}

void SimpleUPnP::get_igd_location()
{
    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS.c_str());
    broadcast_addr.sin_port = htons(BROADCAST_PORT);

    int udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char opt_val = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &opt_val, sizeof(char));

    sendto(udp_sock, SEARCH_REQUEST_STRING.c_str(), SEARCH_REQUEST_STRING.size(), 0, (struct sockaddr*)&broadcast_addr, sizeof(struct sockaddr_in));

    fcntl(udp_sock, F_SETFL, fcntl(udp_sock, F_GETFL) | O_NONBLOCK);

    for(int i = 0; i < m_time_out; ++i)
    {
        sleep(1);

        string upnp_response = read_response(udp_sock, "Discovery");

        if(!upnp_response.empty())
        {
            parse_igd_location(upnp_response);

            get_description();
        }
    }

    close(udp_sock);
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

void SimpleUPnP::exec_command(const std::string &command, const std::string protocol, unsigned short external_port, unsigned short internal_port, const std::string client, int lease_duration, const std::string description)
{
    std::string http_request = make_request(command, protocol, external_port, internal_port, client, lease_duration, description);

    prepare_tcp_socket();
    write(tcp_sock, http_request.c_str(), http_request.length());

    int time_out = 5;
    while(time_out--)
        read_response(tcp_sock, command);

    close(tcp_sock);
}

std::string SimpleUPnP::make_request(const std::string &command, const std::string protocol, unsigned short external_port, unsigned short internal_port, const std::string client, int lease_duration, const std::string description)
{
    string service = "urn:schemas-upnp-org:service:WANIPConnection:1";
    igd_path = "/ipc";

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    if(command == "AddPortMapping")
        sprintf(buffer, control_parameters.at(command).c_str(), external_port, protocol.c_str(),    internal_port, client.c_str(), description.c_str(), lease_duration);

    else if(command == "DeletePortMapping")
        sprintf(buffer, control_parameters.at(command).c_str(), external_port, protocol.c_str());

    else if(command == "GetExternalIPAddress")
        sprintf(buffer, control_parameters.at(command).c_str());

    std::string action_params = buffer;

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, SOAP_ACTION.c_str(), command.c_str(), service.c_str(), action_params.c_str(), command.c_str());

    std::string soap_message = buffer;

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, HTTP_HEADER_ACTION.c_str(), igd_path.c_str(), igd_host.c_str(), igd_port, soap_message.size(), service.c_str(), command.c_str());
    std::string action_message = buffer;

    string http_request = action_message + soap_message;
    return http_request;
}

void SimpleUPnP::get_description()
{
    std::string request = "GET " + igd_path + " HTTP/1.1\r\nHost: " + igd_host + ":" + std::to_string(igd_port) + "\r\n\r\n";

    prepare_tcp_socket();
    write(tcp_sock, request.c_str(), request.length());

    read_response(tcp_sock, "Description");

    close(tcp_sock);
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
    igd_port = std::stoi(str_port);
    igd_path = location_url.substr(path_begin-1);
}

std::string SimpleUPnP::read_response(int sock, const string &command)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    std::string response = "";
    while(read(sock, buffer, BUFFER_SIZE)>0)
    {
        response += buffer;
        memset(buffer, 0, BUFFER_SIZE);
    }

    parse_response(response, command);
    return response;
}

string SimpleUPnP::parse_response(const std::string response, const std::string &command)
{
    if(response == "") return "";

    std::regex rx;

    if(command == "GetExternalIPAddress")
        rx = std::regex("<NewExternalIPAddress>(.*)</NewExternalIPAddress>");
    else if(command == "Description")
        rx = std::regex("</deviceType>(.*)</deviceType>");
    else "";

    std::cmatch res;
    std::regex_search(response.c_str(), res, rx);

    return res[1];
}

