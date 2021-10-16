
#include <iostream>
#include <array>

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

    const std::array<uint16_t,28> ports =
    {
        21,// ftp
        22,// ssh
        23,// telnet
        25,// smtp
        53,// domain name system
        80,// http
        110,// pop3
        111,// rpcbind
        135,// msrpc
        139,// netbios-ssn
        143,// imap
        443,// https
        445,// microsoft-ds
        993,// imaps
        995,// pop3s
        1723,// pptp
        3306,// mysql
        3389,// ms-wbt-server
        5900,// vnc
        8001,// tcp  open  vcom-tunnel
        8002,// tcp  open  teradataordbms
        8080,// tcp  open  http-proxy
        9080,// tcp  open  glrpc
        9999,// tcp  open  abyss
        32768,// /tcp open  filenet-tms
        32769,// /tcp open  filenet-rpc
        32770,// /tcp open  sometimes-rpc3
        32771,// /tcp open  sometimes-rpc5
    };

    ScanNetworkIPv4(MakeIP4V(192,168,1,1),MakeIP4V(192,168,1,255),[ports](const uint32_t pIPv4,const char* pHostName)
    {
        std::cout << IPv4ToString(pIPv4) << " == " << pHostName << " ";

        bool firstHit = true;
        for( auto p : ports )
        {
            if( IsPortOpen(pIPv4,p) )
            {
                if( firstHit )
                {
                    firstHit = false;
                    std::cout << "Port scan: ";
                }
                std::cout << p << " ";
            }
        }

        if( firstHit )
        {
            std::cout << "No intresting ports open ";
        }

        std::cout << "\n";

        return true;
    });

// And quit\n";
    return EXIT_SUCCESS;
}
