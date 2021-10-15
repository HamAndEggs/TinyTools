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

#ifndef TINY_TOOLS_H
#define TINY_TOOLS_H

#include <getopt.h>

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

/**
 * @brief Adds line and source file. There is a c++20 way now that is better. I need to look at that.
 */
#define TINYTOOLS_THROW(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}


namespace tinytools{	// Using a namespace to try to prevent name clashes as my class name is kind of obvious. :)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace math
{
	inline float GetFractional(float pValue)
	{
		return std::fmod(pValue,1.0f);
	}

	inline float GetInteger(float pValue)
	{
		return pValue - GetFractional(pValue);
	}

	/**
	 * @brief Rounds a floating point value into multiplies of 0.5
	 * -1.2 -> -1.0
	 * -1.0 -> -1.0
	 * -0.8 -> -1.0
	 * 2.3 -> 2.5
	 * 2.8 -> 3.0
	 */
	inline float RoundToPointFive(float pValue)
	{
		const float integer = GetInteger(pValue);
		const float frac = std::round(GetFractional(pValue)*2.0f) / 2.0f;
		return integer + frac;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace string{

std::vector<std::string> SplitString(const std::string& pString, const char* pSeperator);

// If pNumChars == 0 then full length is used.
// assert(cppmake::CompareNoCase("onetwo","one",3) == true);
// assert(cppmake::CompareNoCase("onetwo","ONE",3) == true);
// assert(cppmake::CompareNoCase("OneTwo","one",3) == true);
// assert(cppmake::CompareNoCase("onetwo","oneX",3) == true);
// assert(cppmake::CompareNoCase("OnE","oNe") == true);
// assert(cppmake::CompareNoCase("onetwo","one") == true);	// Does it start with 'one'
// assert(cppmake::CompareNoCase("onetwo","onetwothree",6) == true);
// assert(cppmake::CompareNoCase("onetwo","onetwothreeX",6) == true);
// assert(cppmake::CompareNoCase("onetwo","onetwothree") == false); // sorry, but we're searching for more than there is... false...
// assert(cppmake::CompareNoCase("onetwo","onetwo") == true);
/**
 * @brief Does an ascii case insensitive test within the full string or a limited start of the string.
 * 
 * @param pA 
 * @param pB 
 * @param pLength If == 0 then length of second string is used. If first is shorter, will always return false.
 * @return true 
 * @return false 
 */
bool CompareNoCase(const char* pA,const char* pB,size_t pLength = 0);

inline bool CompareNoCase(const std::string& pA,const std::string& pB,size_t pLength = 0)
{
	return CompareNoCase(pA.c_str(),pB.c_str(),pLength);
}


};//namespace string{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace timers{
class MillisecondTicker
{
public:
	MillisecondTicker() = default;
    MillisecondTicker(int pMilliseconds);


	/**
	 * @brief Sets the new timeout interval, resets internal counter.
	 * 
	 * @param pMilliseconds 
	 */
	void SetTimeout(int pMilliseconds);

	/**
	 * @brief Returns true if trigger ticks is less than now
	 */
    bool Tick(){return Tick(std::chrono::system_clock::now());}
    bool Tick(const std::chrono::system_clock::time_point pNow);

	/**
	 * @brief Calls the function if trigger ticks is less than now. 
	 */
    void Tick(std::function<void()> pCallback){Tick(std::chrono::system_clock::now(),pCallback);}
    void Tick(const std::chrono::system_clock::time_point pNow,std::function<void()> pCallback );


private:
    std::chrono::milliseconds mTimeout;
    std::chrono::system_clock::time_point mTrigger;
};

};//namespace timers{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace network{

/**
 * @brief 
 * Used the excellent answer by SpectreVert from the topic below.
 * https://stackoverflow.com/questions/49335001/get-local-ip-address-in-c
 * A little restructure to minimise typo bugs. (e.g fogetting close on socket)
 * @return std::string 
 */
std::string GetLocalIP();

/**
 * @brief Get Host Name, handy little wrapper.
 */
std::string GetHostName();

/**
 * @brief Get the host name from the IP address
 * e.g. const std::string name = GetNameFromIPv4("192.168.1.100");
 */
std::string GetNameFromIPv4(const std::string& pAddress);

/**
 * @brief Get the host name from the IP address
 * e.g. const std::string name = GetNameFromIPv4(MakeIP4V(192,168,1,100));
 */
std::string GetNameFromIPv4(const uint32_t pAddress);

/**
 * @brief Scans from PI range to PI range. Broadcast IP's ignored.
 * 
 * @param pFromIPRange 
 * @param pToIPRange 
 */
void ScanNetworkIPv4(uint32_t pFromIPRange,uint32_t pToIPRange,std::function<bool(const uint32_t pIPv4,const char* pHostName)> pDeviceFound);

/**
 * @brief Creates an 32 bit value from the passed IPv4 address.
 * https://www.sciencedirect.com/topics/computer-science/network-byte-order
 */
inline uint32_t MakeIP4V(uint8_t pA,uint8_t pB,uint8_t pC,uint8_t pD)
{
	// Networking byte order is big endian, so most significan't byte is byte 0.
	return (uint32_t)((pA << 0) | (pB << 8) | (pC << 16) | pD << 24);
}

/**
 * @brief Makes a string, IE 192.168.1.1 from the passed in IPv4 value.
 * 
 * @param pIPv4 In big endian format https://www.sciencedirect.com/topics/computer-science/network-byte-order
 * @return std::string 
 */
inline std::string IPv4ToString(uint32_t pIPv4)
{
	int a = (pIPv4&0x000000ff)>>0;
	int b = (pIPv4&0x0000ff00)>>8;
	int c = (pIPv4&0x00ff0000)>>16;
	int d = (pIPv4&0xff000000)>>24;

	return std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c) + "." + std::to_string(d);
}

};// namespace network

///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace system{

/**
 * @brief Returns seconds since 1970, the epoch. I put this is as I can never rememeber the correct construction using c++ :D
 */
inline int64_t SecondsSinceEpoch()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

/**
 * @brief Fetches the system uptime in a more human readable format
 */
bool GetUptime(uint64_t& rUpDays,uint64_t& rUpHours,uint64_t& rUpMinutes);

/**
 * @brief Return the uptime as a nice human readable string.
 */
std::string GetUptime();

/**
 * @brief USed to track the deltas from last time function is called. Needed to be done like this to beable to see instantaneous CPU load. Method copied from htop source.
 * https://www.linuxhowtos.org/manpages/5/proc.htm
 * https://github.com/htop-dev/htop/
 */
struct CPULoadTracking
{
	uint64_t mUserTime;		// <! Time spent in user mode and hosting virtual machines.
	uint64_t mTotalTime;	// <! Total time, used to create the percentage.
};

/**
 * @brief Will return N + 1 entries for each hardware thread, +1 for total avarage.
 * First call expects pTrackingData to be zero in length, and will initialise the vector. Don't mess with the vector as you're break it all. ;)
 * Index of map is core ID, The ID of the CPU core (including HW threading, so a 8 core 16 thread system will have 17 entries, 16 for threads and one for total)
 * The load is the load of user space and virtual guests on the system. Does not include system time load.
 */
bool GetCPULoad(std::map<int,CPULoadTracking>& pTrackingData,int& rTotalSystemLoad,std::map<int,int>& rCoreLoads);

/**
 * @brief Get the Memory Usage, all values passed back in 1K units because that is what the OS sends back.
 * Used https://gitlab.com/procps-ng/procps as reference as it's not as simple as reading the file. :-? Thanks Linus.....
 */
bool GetMemoryUsage(size_t& rMemoryUsedKB,size_t& rMemAvailableKB,size_t& rMemTotalKB,size_t& rSwapUsedKB);

};//namespace system{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace threading{

/**
 * @brief This class encapsulates a thread that can sleep for any amount of time but wake up when the ownering, main thread, needs to exit the application.
 */
class SleepableThread
{
public:

	/**
	 * @brief This will start the thread and return, pTheWork will be called 'ticked' by the thread after each pause, and exit when asked to.
	 * pPauseInterval is seconds.
	 */
	void Tick(int pPauseInterval,std::function<void()> pTheWork);

	/**
	 * @brief Called from another thread, will ask it to exit and then wait for it to do so.
	 * 
	 */
	void TellThreadToExitAndWait();

private:
    bool mKeepGoing;                  	//!< A boolean that will be used to signal the worker thread that it should exit.
	std::thread mWorkerThread;			//!< The thread that will do your work and then sleep for a bit. 
    std::condition_variable mSleeper;   //!< Used to sleep for how long asked for but also wake up if we need to exit.
    std::mutex mSleeperMutex;			//!< This is used to correctly use the condition variable.
};

};//namespace threading{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
class CommandLineOptions
{
public:
	CommandLineOptions(const std::string& pUsageHelp);

	void AddArgument(char pArg,const std::string& pLongArg,const std::string& pHelp,int pArgumentOption = no_argument,std::function<void(const std::string& pOptionalArgument)> pCallback = nullptr);
	bool Process(int argc, char *argv[]);
	bool IsSet(char pShortOption)const;
	bool IsSet(const std::string& pLongOption)const;
	void PrintHelp()const;

private:
	struct Argument
	{
		Argument(const std::string& pLongArgument,const std::string& pHelp,const int pArgumentOption,std::function<void(const std::string& pOptionalArgument)> pCallback):
			mLongArgument(pLongArgument),
			mHelp(pHelp),
			mArgumentOption(pArgumentOption),
			mCallback(pCallback),
			mIsSet(false)
		{

		}

		const std::string mLongArgument;
		const std::string mHelp;
		const int mArgumentOption;
		std::function<void(const std::string& pOptionalArgument)> mCallback;
		bool mIsSet;	//!< This will be true if the option was part of the commandline. Handy for when you just want to know true or false.
	};
	
	const std::string mUsageHelp;
	std::map<char,Argument> mArguments;
	
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace tinytools
	
#endif //TINY_TOOLS_H
