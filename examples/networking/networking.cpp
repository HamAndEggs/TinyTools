
#include <iostream>

#include "TinyTools.h"

using namespace tinytools::network;

int main(int argc, char *argv[])
{
// Say hello to the world!
    std::cout << "Hello world, a skeleton app generated by appbuild.\n";

// Display the constants defined by app build. \n";
    std::cout << "Application Version " << APP_VERSION << '\n';
    std::cout << "Build date and time " << APP_BUILD_DATE_TIME << '\n';
    std::cout << "Build date " << APP_BUILD_DATE << '\n';
    std::cout << "Build time " << APP_BUILD_TIME << '\n';

    std::cout << "IP: " << GetLocalIP() << " Hostname: " << GetHostName() << "\n";

    ScanNetworkIPv4(MakeIP4V(192,168,1,1),MakeIP4V(192,168,1,255),[](const uint32_t pIPv4,const char* pHostName)
    {
        std::cout << IPv4ToString(pIPv4) << " == " << pHostName << "\n";
        return true;
    });

// And quit\n";
    return EXIT_SUCCESS;
}
