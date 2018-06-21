#ifndef DEFINE_H_
#define DEFINE_H_

#include <string>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <vector>

/************************Train**********************/
struct Train //定义的训练数据
{
	Train()
	{
		id.clear();
		flavorName.clear();
		createaTime.clear();
	}
	std::string id;			 //虚拟机id
	std::string flavorName;  //虚拟机规格
	std::string createaTime; //创建时间
};
/************************Train**********************/

/************************Flavor*********************/
struct Flavor
{
	Flavor()
	{
		flavorName.clear();
		flavorCpu = 0;
		flavorMem = 0;
		nCount = 0;
		s1 = 0.000001;
		s2 = 0.000001;
		s3 = 0.000001;
		createTimeUnix = 0;
		flavorType = -1;
	}
	std::string flavorName; //需要预测的虚拟机
	int flavorCpu;			//虚拟机cpu数
	int flavorMem;			//虚拟机内存大小
	int nCount;				//出现次数
	double s1;				//一次指数平滑值
	double s2;				//二次指数平滑值
	double s3;				//三次指数平滑值
	time_t createTimeUnix;  //创建时间的UNIX时间戳
	int flavorType;//虚拟机对应服务器的类型
};

/************************Flavor*********************/

/************************Server*********************/
struct Server
{
	Server()
	{
		serverCpuCount = 0;
		serverMemCount = 0;
		serverDiskCount = 0;
	}

	int serverCpuCount;  // cpu数量
	int serverMemCount;  // 内存数量
	int serverDiskCount; // 硬盘
};
/************************Server*********************/

struct deployList_ServerTypes
{
	deployList_ServerTypes()
	{
		serverType.clear();
		deployServerList.clear();
	}
	std::string serverType;
	std::vector<std::vector<Flavor>> deployServerList;
};

#define OPTIMIZATION_CPU 0
#define OPTIMIZATION_MEM 1

#define MAXIMUM 999999

#define CPU 0
#define MEM 1

#define TEMPERATURE 0.99

#endif