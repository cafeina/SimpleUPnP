#include <map>
#include <string>
#include <vector>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

using namespace std;

map<string, vector<string>> GetInternalIP()
{
	struct ifaddrs * ifa=NULL;
	void * tmp_ip_addr;

	char ipv4_addr[INET_ADDRSTRLEN];
	char ipv6_addr[INET6_ADDRSTRLEN];
	map<string, vector<string>> addresses;

	getifaddrs(&ifa);

	while(ifa) {
		if (!ifa->ifa_addr)
			continue;

		if(strcmp(ifa->ifa_name, "lo") == 0) {
			ifa = ifa->ifa_next;
			continue;
		}

		if (ifa->ifa_addr->sa_family == AF_INET) { // check if it is IPv4
			tmp_ip_addr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmp_ip_addr, ipv4_addr, INET_ADDRSTRLEN);
			addresses["IPV4"].push_back(ipv4_addr);

		} else if (ifa->ifa_addr->sa_family == AF_INET6) { // check if it is IPv6
			tmp_ip_addr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			inet_ntop(AF_INET6, tmp_ip_addr, ipv6_addr, INET6_ADDRSTRLEN);
			addresses["IPV6"].push_back(ipv6_addr);
		}

		ifa = ifa->ifa_next;
	}

	if (ifa)
		freeifaddrs(ifa);

	return addresses;
}
