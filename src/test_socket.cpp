#include "config/config.h"
#include "log/logger.h"
#include "socket/address.h"

using namespace lon::socket;

void test_address()
{
    IPv4Address ipv4(INADDR_LOOPBACK, 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv4 addr:" << ipv4.toString();

    IPv6Address ipv6("::1", 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv6 addr:" << ipv6.toString();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    test_address();
    return 0;
}
