#include "Tools.h"

//字符串分割函数  
std::vector<std::string> Tools::split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;

	str += pattern;//扩展字符串以方便操作  
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


// 转换形如 2014-08-01 12:38:22 的时间戳到unixtime   
time_t Tools::GetUnixTime(std::string TimeStamp)
{
	const char * pszTimeStamp = TimeStamp.c_str();
	struct tm stTime;
	memset(&stTime, 0, sizeof(stTime));
	sscanf(pszTimeStamp, "%d-%d-%d %d:%d:%d",
		&stTime.tm_year, &stTime.tm_mon, &stTime.tm_mday,
		&stTime.tm_hour, &stTime.tm_min, &stTime.tm_sec);
	stTime.tm_year -= 1900;
	stTime.tm_mon--;
	return mktime(&stTime);
}

void Tools::sort(std::vector<Flavor> & flavorList)
{//冒泡排序
	Flavor temp;
	for (int i = 1; i < (int)flavorList.size(); i++) 
	{
		for (int j = (int)flavorList.size() - 1; j >= i; j--) 
		{
			if (atoi(flavorList[j].flavorName.c_str() + 6) < atoi(flavorList[j - 1].flavorName.c_str() + 6)) 
			{
				temp = flavorList[j - 1];
				flavorList[j - 1] = flavorList[j];
				flavorList[j] = temp;
			}

		}
	}
}

void Tools::flavorListInit(std::vector<Flavor> & predictedFlavorList, std::vector<Flavor> sourceFlavorList)
{
	for (size_t nIndex = 0; nIndex < sourceFlavorList.size(); ++nIndex)
	{
		Flavor fl = sourceFlavorList[nIndex];
		predictedFlavorList.push_back(fl);
	}
}
