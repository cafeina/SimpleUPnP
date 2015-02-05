#include <string>
#include <iostream>
#include <unistd.h>

#include "basic_upnp.h"

using std::string;

void get_in_ip()
{

}

int main()
{
    SimpleUPnP u;
    int port = 8080;
    //u.RouterInfo();
    //u.GetStatusInfo();
    //u.GetNatRSIPStatus();
    std::cout << "1" << std::endl;
    std::cout << u.AddPortMapping(8080, "TCP", port, u.GetInternalIP()["IPV4"][0],
"Gleison ROCKS", 60) << std::endl;
    std::cout << "2" << std::endl;
    std::cout << u.GetExternalIPAddress() << std::endl;
    std::cout << u.GetConnectionTypeInfo() << std::endl;

//    std::cout << u.GetSpecificPortMappingEntry(8080, "TCP") << std::endl;

    return 0;
}
