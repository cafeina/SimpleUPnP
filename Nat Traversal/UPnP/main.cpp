#include <string>
#include <unistd.h>

#include "basic_upnp.h"

using std::string;

int main()
{
    SimpleUPnP u;
    u.get_igd_location();
    string external_ip = u.exec_command("GetExternalIPAddress");
    int port = 50423;
    
    sleep(2);
    //u.add_port_mapping(5456, 5456, "UDP", "192.168.1.101");
    u.exec_command("AddPortMapping", "TCP", port, port, "192.168.1.101", 0, "Gleison ROCKS");
    sleep(10);
    u.exec_command("DeletePortMapping", "TCP", port);

    

}
