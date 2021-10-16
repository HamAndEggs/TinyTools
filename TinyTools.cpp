/*
   Copyright (C) 2017, Richard e Collins.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#include <getopt.h>
#include <assert.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <condition_variable>

#include "TinyTools.h"

/**
 * @brief Adds line and source file. There is a c++20 way now that is better. I need to look at that.
 */
#define TINYTOOLS_THROW(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}


namespace tinytools{	// Using a namespace to try to prevent name clashes as my class name is kind of obvious. :)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace math
{
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace string{

std::vector<std::string> SplitString(const std::string& pString, const char* pSeperator)
{
	std::vector<std::string> res;
	for (size_t p = 0, q = 0; p != pString.npos; p = q)
	{
		const std::string part(pString.substr(p + (p != 0), (q = pString.find(pSeperator, p + 1)) - p - (p != 0)));
		if( part.size() > 0 )
		{
			res.push_back(part);
		}
	}
	return res;
}

bool CompareNoCase(const char* pA,const char* pB,size_t pLength)
{
    assert( pA != nullptr || pB != nullptr );// Note only goes pop if both are null.
// If either or both NULL, then say no. A bit like a divide by zero as null strings are not strings.
    if( pA == nullptr || pB == nullptr )
        return false;

// If same memory then yes they match, doh!
    if( pA == pB )
        return true;

    if( pLength == 0 )
        pLength = strlen(pB);

    while( (*pA != 0 || *pB != 0) && pLength > 0 )
    {
        // Get here are one of the strings has hit a null then not the same.
        // The while loop condition would not allow us to get here if both are null.
        if( *pA == 0 || *pB == 0 )
        {// Check my assertion above that should not get here if both are null. Note only goes pop if both are null.
            assert( pA != NULL || pB != NULL );
            return false;
        }

        if( tolower(*pA) != tolower(*pB) )
            return false;

        pA++;
        pB++;
        pLength--;
    };

    // Get here, they are the same.
    return true;
}

};//namespace string{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace timers{
MillisecondTicker::MillisecondTicker(int pMilliseconds)
{
	SetTimeout(pMilliseconds);
}


void MillisecondTicker::SetTimeout(int pMilliseconds)
{
	assert(pMilliseconds > 0 );
	mTimeout = std::chrono::milliseconds(pMilliseconds);
	mTrigger = std::chrono::system_clock::now() + mTimeout;
}

bool MillisecondTicker::Tick(const std::chrono::system_clock::time_point pNow)
{
	if( mTrigger < pNow )
	{
		mTrigger += mTimeout;
		return true;
	}
	return false;
}

void MillisecondTicker::Tick(const std::chrono::system_clock::time_point pNow,std::function<void()> pCallback )
{
	assert( pCallback != nullptr );
	if( mTrigger < pNow )
	{
		mTrigger += mTimeout;
		pCallback();
	}
}

};//namespace timers{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace network{

std::string GetLocalIP()
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback;
 
	std::string ip = "Could not socket";
    if( sock > 0 )
	{
		std::memset(&loopback, 0, sizeof(loopback));
		loopback.sin_family = AF_INET;
		loopback.sin_addr.s_addr = 1;   // can be any IP address. Odd, but works. :/
		loopback.sin_port = htons(9);   // using debug port

		if( connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == 0 )
		{
			socklen_t addrlen = sizeof(loopback);
			if( getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == 0 )
			{
				char buf[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0)
				{
					ip = "Could not inet_ntop";
				}
				else
				{
					ip = buf;
				}
			}
			else
			{
				ip = "Could not getsockname";
			}
		}
		else
		{
			ip = "Could not connect";
		}

		// All paths that happen after opening sock will come past here. Less chance of accidentally leaving it open.
		close(sock);
	}

	return ip;
}

std::string GetHostName()
{
    char buf[256];
    ::gethostname(buf,256);
	return std::string(buf);
}

std::string GetNameFromIPv4(const std::string& pAddress)
{
    sockaddr_in deviceIP;
	memset(&deviceIP, 0, sizeof deviceIP);

	const int ret = inet_pton(AF_INET,pAddress.c_str(),&deviceIP.sin_addr);
	 if (ret <= 0)
	 {
		if (ret == 0)
			return "Not in presentation format";
		return "inet_pton failed";
	}

	deviceIP.sin_family = AF_INET;

	char hbuf[NI_MAXHOST];
	hbuf[0] = 0;


	getnameinfo((struct sockaddr*)&deviceIP,sizeof(deviceIP),hbuf, sizeof(hbuf), NULL,0, NI_NAMEREQD);
	const std::string name = hbuf;
	return name;
}

std::string GetNameFromIPv4(const uint32_t pAddress)
{
    sockaddr_in deviceIP;
	memset(&deviceIP, 0, sizeof deviceIP);

	deviceIP.sin_addr.s_addr = pAddress;
	deviceIP.sin_family = AF_INET;

	char hbuf[NI_MAXHOST];
	hbuf[0] = 0;

	getnameinfo((struct sockaddr*)&deviceIP,sizeof(deviceIP),hbuf, sizeof(hbuf), NULL,0, NI_NAMEREQD);
	const std::string name = hbuf;
	return name;
}

uint32_t GetIPv4FromName(const std::string& pHostName)
{
	struct addrinfo hints;
    struct addrinfo *result;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;    // only IPv4 please
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	uint32_t IPv4 = 0;

	if( getaddrinfo(pHostName.c_str(),NULL,&hints,&result) == 0 )
	{
		for( struct addrinfo *rp = result; rp != NULL && IPv4 == 0 ; rp = rp->ai_next)
		{
			if( rp->ai_addr->sa_family == AF_INET )
			{
				IPv4 = ((const sockaddr_in*)rp->ai_addr)->sin_addr.s_addr;
			}
		}

		// Clean up
		freeaddrinfo(result);
	}

	return IPv4;
}

void ScanNetworkIPv4(uint32_t pFromIPRange,uint32_t pToIPRange,std::function<bool(const uint32_t pIPv4,const char* pHostName)> pDeviceFound)
{
	// Because IP values are in big endian format I need disassemble to iterate over them.
	const int aFrom = (pFromIPRange&0x000000ff)>>0;
	const int bFrom = (pFromIPRange&0x0000ff00)>>8;
	const int cFrom = (pFromIPRange&0x00ff0000)>>16;
	const int dFrom = std::clamp(int((pFromIPRange&0xff000000)>>24),1,254);

	const int aTo = (pToIPRange&0x000000ff)>>0;
	const int bTo = (pToIPRange&0x0000ff00)>>8;
	const int cTo = (pToIPRange&0x00ff0000)>>16;
	const int dTo = std::clamp(int((pToIPRange&0xff000000)>>24),1,254);

	std::cout << "From " << aFrom << "." << bFrom << "." << cFrom << "." << dFrom << "\n";
	std::cout << "To " << aTo << "." << bTo << "." << cTo << "." << dTo << "\n";

	for( int a = aFrom ; a <= aTo ; a++ )
	{
		for( int b = bFrom ; b <= bTo ; b++ )
		{
			for( int c = cFrom ; c <= cTo ; c++ )
			{
				for( int d = dFrom ; d <= dTo ; d++ )
				{
					sockaddr_in deviceIP;
					memset(&deviceIP, 0, sizeof deviceIP);

					deviceIP.sin_addr.s_addr = MakeIP4V(a,b,c,d);
					deviceIP.sin_family = AF_INET;

					char hbuf[NI_MAXHOST];
					hbuf[0] = 0;

					if( getnameinfo((struct sockaddr*)&deviceIP,sizeof(deviceIP),hbuf, sizeof(hbuf), NULL,0, NI_NAMEREQD) == 0 )
					{
						if( pDeviceFound(deviceIP.sin_addr.s_addr,hbuf) == false )
						{
							return;// User asked to end.
						}
					}
				}
			}
		}
	}
}

bool IsPortOpen(uint32_t pIPv4,uint16_t pPort)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockfd < 0)
	{
		TINYTOOLS_THROW("ERROR opening socket");
		return false;
	}

    sockaddr_in deviceAddress;
	memset(&deviceAddress, 0, sizeof(deviceAddress));

	deviceAddress.sin_addr.s_addr = pIPv4;
	deviceAddress.sin_port = htons(pPort);
	deviceAddress.sin_family = AF_INET;

	const bool isOpen = connect(sockfd,(struct sockaddr *) &deviceAddress,sizeof(deviceAddress)) == 0;
	close(sockfd);
	return isOpen;
}

};// namespace network

///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace system{

/**
 * @brief Fetches the system uptime in a more human readable format
 */
bool GetUptime(uint64_t& rUpDays,uint64_t& rUpHours,uint64_t& rUpMinutes)
{
    rUpDays = 0;
    rUpHours = 0;
    rUpMinutes = 0;

    std::ifstream upTimeFile("/proc/uptime");
    if( upTimeFile.is_open() )
    {
        rUpDays = 999;
        char buf[256];
        buf[255] = 0;
        if( upTimeFile.getline(buf,255,' ') )
        {
            const uint64_t secondsInADay = 60 * 60 * 24;
            const uint64_t secondsInAnHour = 60 * 60;
            const uint64_t secondsInAMinute = 60;

            const uint64_t totalSeconds = std::stoull(buf);
            rUpDays = totalSeconds / secondsInADay;
            rUpHours = (totalSeconds - (rUpDays*secondsInADay)) / secondsInAnHour;
            rUpMinutes = (totalSeconds - (rUpDays*secondsInADay) - (rUpHours * secondsInAnHour) ) / secondsInAMinute;
            return true;
        }
    }
    return false;
}

std::string GetUptime()
{
	uint64_t upDays,upHours,upMinutes;
	GetUptime(upDays,upHours,upMinutes);
	std::string upTime;
	std::string space;
	if( upDays > 0 )
	{
		if( upDays == 1 )
		{
			upTime = "1 Day";
		}
		else
		{
			upTime = std::to_string(upDays) + " Days";
		}
		space = " ";
	}

	if( upHours > 0 )
	{
		if( upHours == 1 )
		{
			upTime += space + "1 Hour";
		}
		else
		{
			upTime += space + std::to_string(upHours) + " Hours";
		}
		space = " ";
	}

	if( upMinutes == 1 )
	{
		upTime += "1 Minute";
	}
	else
	{
		upTime += space + std::to_string(upMinutes) + " Minutes";
	}

	return upTime;
}

bool GetCPULoad(std::map<int,CPULoadTracking>& pTrackingData,int& rTotalSystemLoad,std::map<int,int>& rCoreLoads)
{
	// If pTrackingData is empty then we're initalising the state so lets build our starting point.
	const bool initalising = pTrackingData.size() ==  0;

	std::ifstream statFile("/proc/stat");
	if( statFile.is_open() )
	{
		// Written so that it'll read the lines in any order. Easy to do and safe.
		while( statFile.eof() == false )
		{
			std::string what;
			statFile >> what;
			if( string::CompareNoCase(what,"cpu") )
			{
				// The the rest of the bits we want.
				std::string usertimeSTR, nicetimeSTR, systemtimeSTR, idletimeSTR, ioWaitSTR, irqSTR, softIrqSTR, stealSTR, guestSTR, guestniceSTR;
				statFile >> usertimeSTR >>
								 nicetimeSTR >>
								 systemtimeSTR >>
								 idletimeSTR >>
								 ioWaitSTR >>
								 irqSTR >>
								 softIrqSTR >>
								 stealSTR >>
								 guestSTR >>
								 guestniceSTR;
				// Read rest of line.
				std::string eol;
				std::getline(statFile,eol,'\n');

				const uint64_t userTime = std::stoull(usertimeSTR);	
				const uint64_t niceTime = std::stoull(nicetimeSTR);
				const uint64_t idleTime = std::stoull(idletimeSTR);
				const uint64_t stealTime = std::stoull(stealSTR);
				
				const uint64_t systemAllTime = std::stoull(systemtimeSTR) + std::stoull(irqSTR) + std::stoull(softIrqSTR);
//				const uint64_t virtualTime = std::stoull(guestSTR) + std::stoull(guestniceSTR); // According to a comment in hot, guest and guest nice are already incorerated in user time.
				const uint64_t totalTime = userTime + niceTime + systemAllTime + idleTime + stealTime; // No need to add virtualTime, I want that to be part of user time. If not will not see load on CPU from virtual guest.

				int cpuID = -1; // This is the total system load.
				if( what.size() > 3 )
				{
					sscanf(what.data(),"cpu%d",&cpuID);
					if( cpuID < 0 || cpuID > 128 )
					{
						TINYTOOLS_THROW("Error in GetCPULoad, cpu Id read from /proc/stat has a massive index, not going ot happen mate... cpuID == " + std::to_string(cpuID));
					}
				}

				if( initalising )
				{
					if( pTrackingData.find(cpuID) != pTrackingData.end() )
					{
						TINYTOOLS_THROW("Error in GetCPULoad, trying to initalising and found a duplicate CPU id, something went wrong. cpuID == " + std::to_string(cpuID));
					}

					CPULoadTracking info;
					info.mUserTime		= userTime;
					info.mTotalTime		= totalTime;
					pTrackingData.emplace(cpuID,info);
				}
				else
				{
					// Workout the deltas to get cpu load in percentage.
					auto core = pTrackingData.find(cpuID);
					if( core == pTrackingData.end() )
					{
						TINYTOOLS_THROW("Error in GetCPULoad, trying to workout load but found a new CPU id, something went wrong. cpuID == " + std::to_string(cpuID));
					}

					// Copied from htop!
					// Since we do a subtraction (usertime - guest) and cputime64_to_clock_t()
					// used in /proc/stat rounds down numbers, it can lead to a case where the
					// integer overflow.
					#define WRAP_SUBTRACT(a,b) (a > b) ? a - b : 0					
					const uint64_t deltaUser = WRAP_SUBTRACT(userTime,core->second.mUserTime);
					const uint64_t deltaTotal = WRAP_SUBTRACT(totalTime,core->second.mTotalTime);
					#undef WRAP_SUBTRACT
					core->second.mUserTime = userTime;
					core->second.mTotalTime = totalTime;

					if( deltaTotal > 0 )
					{
						const uint64_t percentage = deltaUser * 100 / deltaTotal;
						if( cpuID == -1 )
						{
							rTotalSystemLoad = (int)percentage;
						}
						else
						{
							rCoreLoads[cpuID] = (int)percentage;
						}
					}
					else
					{
						if( cpuID == -1 )
						{
							rTotalSystemLoad = 0;
						}
						else
						{
							rCoreLoads[cpuID] = 0;
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool GetMemoryUsage(size_t& rMemoryUsedKB,size_t& rMemAvailableKB,size_t& rMemTotalKB,size_t& rSwapUsedKB)
{
	std::ifstream memFile("/proc/meminfo");
	if( memFile.is_open() )
	{
		// We'll build a map of the data lines that have key: value combination.
		std::map<std::string,size_t> keyValues;
		while( memFile.eof() == false )
		{
			std::string memLine;
			std::getline(memFile,memLine);
			if( memLine.size() > 0 )
			{
				std::vector<std::string> memParts = string::SplitString(memLine," ");// Have to do like this as maybe two or three parts. key: value [kb]
				if( memParts.size() != 2 && memParts.size() != 3 )
				{
					TINYTOOLS_THROW("Failed to correctly read a line from /proc/meminfo did the format change? Found line \"" + memLine + "\" and split it into " + std::to_string(memParts.size()) + " parts");
				}

				if( memParts[0].back() == ':' )
				{
					memParts[0].pop_back();
					keyValues[memParts[0]] = std::stoull(memParts[1]);
				}
			}
		}

		try
		{
			const size_t Buffers = keyValues.at("Buffers");
			const size_t Cached = keyValues["Cached"];
			const size_t SReclaimable = keyValues["SReclaimable"];
			const size_t MemAvailable = keyValues["MemAvailable"];
			const size_t MemTotal = keyValues["MemTotal"];
			const size_t MemFree = keyValues["MemFree"];
			const size_t SwapFree = keyValues["SwapFree"];
			const size_t SwapTotal = keyValues["SwapTotal"];

			const size_t mainCached = Cached + SReclaimable;
			// if kb_main_available is greater than kb_main_total or our calculation of
			// mem_used overflows, that's symptomatic of running within a lxc container
			// where such values will be dramatically distorted over those of the host.
			if (MemAvailable > MemTotal)
			{
				rMemAvailableKB = MemFree;
			}
			else
			{
				rMemAvailableKB = MemAvailable;
			}

			rMemoryUsedKB = MemTotal - MemFree - mainCached - Buffers;
			if (rMemoryUsedKB < 0)
			{
				rMemoryUsedKB = MemTotal - MemFree;
			}

			rMemTotalKB = MemTotal;
			rSwapUsedKB = SwapTotal - SwapFree;

			return true;
		}
		catch( std::out_of_range& e ){std::cerr << " failed to read all the data needed from /proc/meminfo " << e.what();}// We'll return false so they know something went wrong.
	}
	return false;
}

};//namespace system{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace threading{

void SleepableThread::Tick(int pPauseInterval,std::function<void()> pTheWork)
{
	if( pTheWork == nullptr )
	{
		TINYTOOLS_THROW("SleepableThread passed nullpoint for the work to do...");
	}

	mWorkerThread = std::thread([this,pPauseInterval,pTheWork]()
	{
		while(mKeepGoing)
		{
			pTheWork();
			std::unique_lock<std::mutex> lk(mSleeperMutex);
			mSleeper.wait_for(lk,std::chrono::seconds(pPauseInterval));
		};
	});
}

void SleepableThread::TellThreadToExitAndWait()
{
	mKeepGoing = false;
	mSleeper.notify_one();
	if( mWorkerThread.joinable() )
	{
		mWorkerThread.join();
	}
}

};//namespace threading{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
CommandLineOptions::CommandLineOptions(const std::string& pUsageHelp):mUsageHelp(pUsageHelp)
{// Always add this, everyone does. BE rude not to. ;)
	AddArgument('h',"help","Display this help and exit");
}

void CommandLineOptions::AddArgument(char pArg,const std::string& pLongArg,const std::string& pHelp,int pArgumentOption,std::function<void(const std::string& pOptionalArgument)> pCallback)
{
	if( mArguments.find(pArg) != mArguments.end() )
	{
		TINYTOOLS_THROW(std::string("class CommandLineOptions: Argument ") + pArg + " has already been registered, can not contine");
	}
	
	mArguments.emplace(pArg,Argument(pLongArg,pHelp,pArgumentOption,pCallback));
}

bool CommandLineOptions::Process(int argc, char *argv[])
{
	std::vector<struct option> longOptions;
	std::string shortOptions;

	// Build the data for the getopt_long function that will do all the work for us.
	for( auto& opt : mArguments)
	{
		shortOptions += opt.first;
		if( opt.second.mArgumentOption == required_argument )
		{
			shortOptions += ":";
		}
		// Bit of messing about because mixing c code with c++
		struct option newOpt = {opt.second.mLongArgument.c_str(),opt.first,nullptr,opt.first};
		longOptions.emplace_back(newOpt);
	}
	struct option emptyOpt = {NULL, 0, NULL, 0};
	longOptions.emplace_back(emptyOpt);

	int c,oi;
	while( (c = getopt_long(argc,argv,shortOptions.c_str(),longOptions.data(),&oi)) != -1 )
	{
		auto arg = mArguments.find(c);
		if( arg == mArguments.end() )
		{// Unknow option, print help and bail.
			std::cout << "Unknown option \'" << c << "\' found.\n";
			PrintHelp();
			return false;
		}
		else
		{
			arg->second.mIsSet = true;
			std::string optionalArgument;
			if( optarg )
			{// optarg is defined in getopt_code.h 
				optionalArgument = optarg;
			}

			if( arg->second.mCallback != nullptr )
			{
				arg->second.mCallback(optionalArgument);
			}
		}
	};

	// See if help was asked for.
	if( IsSet('h') )
	{
		PrintHelp();
		return false;
	}

	return true;
}

bool CommandLineOptions::IsSet(char pShortOption)const
{
	return mArguments.at(pShortOption).mIsSet;
}

bool CommandLineOptions::IsSet(const std::string& pLongOption)const
{
	for( auto& opt : mArguments)
	{
		if( string::CompareNoCase(opt.second.mLongArgument,pLongOption) )
		{
			return opt.second.mIsSet;
		}
	}

	return false;
}

void CommandLineOptions::PrintHelp()const
{
	std::cout << mUsageHelp << "\n";

	std::vector<char> shortArgs;
	std::vector<std::string> longArgs;
	std::vector<std::string> descriptions;

	for( auto& opt : mArguments)
	{
		shortArgs.push_back(opt.first);
		descriptions.push_back(opt.second.mHelp);
		if( opt.second.mArgumentOption == required_argument )
		{
			longArgs.push_back("--" + opt.second.mLongArgument + "=arg");
		}
		else if( opt.second.mArgumentOption == optional_argument )
		{
			longArgs.push_back("--" + opt.second.mLongArgument + "[=arg]");
		}
		else
		{
			longArgs.push_back("--" + opt.second.mLongArgument);
		}
	}

	// Now do a load of formatting of the output.
	size_t DescMaxSpace = 0;
	for(auto lg : longArgs)
	{
		size_t l = 5 + lg.size(); // 5 == 2 spaces + -X + 1 for space for short arg.
		if( DescMaxSpace < l )
			DescMaxSpace = l;
	}

	DescMaxSpace += 4; // Add 4 spaces for formatting.
	for(size_t n=0;n<shortArgs.size();n++)
	{
		std::string line = "  -";
		line += shortArgs[n];
		line += " ";
		line += longArgs[n];
		line += " ";
		std::cout << line;

		size_t space = DescMaxSpace - line.size();
		const std::vector<std::string> lines = string::SplitString(descriptions[n],"\n");
		for(auto line : lines)
		{
			std::cout << std::string(space,' ') << line << '\n';
			space = DescMaxSpace + 2;// For subsequent lines.
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace tinytools
	