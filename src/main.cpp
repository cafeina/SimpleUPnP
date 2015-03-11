#include <string>
#include <iostream>
#include <unistd.h>

#include "basic_upnp.hpp"
#include "utilities.hpp"

using std::string;
using std::cout;
using std::endl;

int main()
{
    SimpleUPnP upnp;
    int port = 8080;

    upnp.DeviceInfo();

    cout << upnp.AddPortMapping(8080, "TCP", port, GetInternalIP()["IPV4"][0],
"Cafeina ROCKS", 60) << endl;

    cout << upnp.GetExternalIPAddress() << endl;

    return 0;
}
