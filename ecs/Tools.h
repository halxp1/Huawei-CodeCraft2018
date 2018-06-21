#ifndef TOOLS_H_
#define TOOLS_H_

#include "define.h"

class Tools
{
public:
	static std::vector<std::string> split(std::string str, std::string pattern);
	static time_t GetUnixTime(std::string TimeStamp);
	static void sort(std::vector<Flavor> & flavorData);
	static void flavorListInit(std::vector<Flavor> & predictedFlavorList, std::vector<Flavor> sourceFlavorList);
};

#endif
