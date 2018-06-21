#include "DeployAlgorithm.h"
#include "PredictAlgorithm.h"
#include "DataCenter.h"
#include "Tools.h"
using namespace std;
#include <iostream>
#include <cstring>

DeployAlgorithm::DeployAlgorithm()
{
	// 模拟退火变量
	m_T = 100.0;   //模拟退火初始温度
	m_Tmin = 1.0;	//模拟退火终止温度
	m_maxRate = 0; // 最高评分
	m_optimizationResult.clear();
	m_r = TEMPERATURE; //温度下降系数
	m_result.clear();
	m_deployResult.clear();
}

DeployAlgorithm::DeployAlgorithm(PredictAlgorithm &pa, DataCenter &dc, int deployModel)
{
	// 模拟退火变量
	m_T = 100.0;   //模拟退火初始温度
	m_Tmin = 1.0;	//模拟退火终止温度
	m_maxRate = 0; // 最高评分
	m_optimizationResult.clear();
	m_r = TEMPERATURE; //温度下降系数
	m_deployResult.clear();

	m_pa = &pa;
	m_dc = &dc;
	m_deployModel = deployModel;
}

DeployAlgorithm::~DeployAlgorithm()
{
}

void DeployAlgorithm::setPredictAlgorithmObject(PredictAlgorithm &pa, DataCenter &dc)
{
	m_pa = &pa;
	m_dc = &dc;
}

std::string &DeployAlgorithm::getDeployResultStr()
{
	return m_result;
}

void DeployAlgorithm::setDeployModel(int deployModel)
{
	m_deployModel = deployModel;
}

std::string &DeployAlgorithm::getError()
{
	return m_error;
}

void DeployAlgorithm::startDeploy()
{
	//部署服务器
	DeployFlavor();
}

void DeployAlgorithm::DeployFlavor()
{
	//获取优化方式 CPU or MEM
	//m_optimizationModel = m_dc->getOptimizationModel();
	switch (m_deployModel)
	{
	case GREEDY_ALGORITHM:
		//{
		//	std::vector<Flavor> PredictResult;
		//	Tools::flavorListInit(PredictResult, m_pa->getPredictResultVector());
		//	std::vector<std::vector<Flavor>> deployServerList; //临时存储结果
		//	DeployFlavor_GREEDY_ALGORITHM(deployServerList, m_result, PredictResult);
		//	break;
		//}
		DeployFlavor_GREEDY_ALGORITHM(m_result);
		break;
	case GREEDY_ALGORITHM_KNAPSACK:
		DeployFlavor_GREEDY_ALGORITHM_KNAPSACK(m_result);
		break;
	case DEPLOY_AUTO:
		DeployFlavor_DEPLOY_AUTO();
		break;
	case SIMULATED_ANNEALING:
		DeployFlavor_SIMULATED_ANNEALING(m_result);
		break;
	default:
		break;
	}
}

double DeployAlgorithm::DeployFlavor_GREEDY_ALGORITHM(std::vector<std::vector<Flavor>> & deployServerList, std::string &result, std::vector<Flavor> PredictResult, int type)
{
	//执行贪婪算法
	int nServerCpuCount = m_dc->getServerCpuCount(type).serverCpuCount;
	int nServerMemCount = m_dc->getServerMemCount(type).serverMemCount;
	double serverResourceProportion = (double)nServerCpuCount / (double)nServerMemCount;

	while (checkFlavorFromFlavorList(PredictResult))
	{ //存在未分配的虚拟机
		std::vector<Flavor> currentServerFlavorList;
		Flavor *fl = getMaxResourceFlavor(PredictResult);
		if (nullptr != fl)
		{
			Flavor *tempFl = new Flavor;
			memcpy(tempFl, fl, sizeof(Flavor));
			currentServerFlavorList.push_back(*tempFl);
		}
		while (true)
		{ //不断抽取虚拟机放入服务器，直到抽取虚拟机失败
			fl = extractFlavor(PredictResult, currentServerFlavorList, serverResourceProportion, nServerCpuCount, nServerMemCount);
			if (nullptr == fl)
			{
				break;
			}

			if (!checkResource(currentServerFlavorList, fl, nServerCpuCount, nServerMemCount))
			{ //抽取资源合格
				Flavor *tempFl = new Flavor;
				memcpy(tempFl, fl, sizeof(Flavor));
				currentServerFlavorList.push_back(*tempFl);
			}
			else
			{
				//存在超分,将此虚拟机还回缓冲区
				for (size_t i = 0; i < PredictResult.size(); ++i)
				{
					if (PredictResult[i].flavorName == fl->flavorName)
					{
						PredictResult[i].nCount++;
						break;
					}
				}
			}
		}
		deployServerList.push_back(currentServerFlavorList);
	}
	//deployList_ServerTypes ds;
	//ds.serverType = DataCenter::GENERAL == 
	//m_deployResult.push_back(ds);
	return calcOccupancyRate(deployServerList, type);

	// double rate = calcOccupancyRate(m_optimizationModel, deployServerList);

	// /*****************************************************修正分配结果*******************************************************/
	// std::string predictResult = m_pa->getResultStr();
	// std::vector<std::string> flavorList = Tools::split(predictResult, "\n");
	// std::vector<std::vector<Flavor>> tempDeployServerList;
	// int nTotal = 0;
	// for (std::vector<std::vector<Flavor>>::iterator it = deployServerList.begin(); it != deployServerList.end(); ++it)
	// {
	// 	rate = calcOccupancyRate(m_dc->getOptimizationModel(), *it);
	// 	if (rate < 0.83)
	// 	{ //查找占用率较低的虚拟机规格，并统计每种规格出现的次数,用于修正预测结果
	// 		for (int i = 0; i < (*it).size(); ++i)
	// 		{
	// 			for (int j = 1; j < flavorList.size(); ++j)
	// 			{
	// 				std::vector<std::string> ss = Tools::split(flavorList[j], " ");
	// 				if (!ss.empty())
	// 				{
	// 					if ((*it)[i].flavorName == ss[0])
	// 					{
	// 						nTotal++;
	// 						flavorList[j] = ss[0] + " " + std::to_string(std::stoi(ss[1]) - 1);
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		tempDeployServerList.push_back(*it);
	// 	}
	// }
	// std::string deployResult;
	// flavorList[0] = std::to_string(std::stoi(flavorList[0]) - nTotal);
	// predictResult.clear();
	// for (int i = 0; i < flavorList.size(); ++i)
	// {
	// 	predictResult += flavorList[i] + "\n";
	// }
	// //统计分配结果，输出到字符串中 result
	// statisticsFlavor(tempDeployServerList, deployResult);

	// //*****************************************************修正分配结果*******************************************************/

	// result = predictResult + deployResult;
	// return rate;
}

void DeployAlgorithm::DeployFlavor_GREEDY_ALGORITHM(std::string &result)
{
	//1.抽取服务器
	//2.抽取虚拟机部署服务器
	std::vector<Flavor> PredictResult;
	Tools::flavorListInit(PredictResult, m_pa->getPredictResultVector());
	std::vector<std::vector<Flavor>> GeneralServerList;         //通用型服务器列表
	std::vector<std::vector<Flavor>> LargeMemoryServerList;     //大内存服务器列表
	std::vector<std::vector<Flavor>> HighPerformanceServerList; //大内存服务器列表

	int nServerIndex = calcNeedServerType(PredictResult);
	//1.计算虚拟机池里那种类型的虚拟机最多
	//2.根据上一步计算结果开启一台新的服务器
	//3.把刚开启的服务器装满
	//4.回到第一步
	int nServerCpuCount = 0;
	int nServerMemCount = 0;
	double serverResourceProportion = 0;
	while (checkFlavorFromFlavorList(PredictResult) && (-1 != nServerIndex))
	{ //存在未分配的虚拟机
		nServerCpuCount = m_dc->getServerCpuCount(nServerIndex).serverCpuCount;
		nServerMemCount = m_dc->getServerMemCount(nServerIndex).serverMemCount;
		serverResourceProportion = (double)nServerCpuCount / (double)nServerMemCount;

		std::vector<Flavor> currentServerFlavorList;
		Flavor *fl = getMatchMaxResourceFlavor(PredictResult, nServerIndex);
		if (nullptr != fl)
		{
			Flavor *tempFl = new Flavor;
			memcpy(tempFl, fl, sizeof(Flavor));
			currentServerFlavorList.push_back(*tempFl);
		}
		while (true)
		{ //不断抽取虚拟机放入服务器，直到抽取虚拟机失败
			fl = extractFlavor(PredictResult, currentServerFlavorList, serverResourceProportion, nServerCpuCount, nServerMemCount);
			if (nullptr == fl)
			{
				break;
			}

			if (!checkResource(currentServerFlavorList, fl, nServerCpuCount, nServerMemCount))
			{ //抽取资源合格
				Flavor *tempFl = new Flavor;
				memcpy(tempFl, fl, sizeof(Flavor));
				currentServerFlavorList.push_back(*tempFl);
			}
			else
			{
				//存在超分,将此虚拟机还回缓冲区
				for (size_t i = 0; i < PredictResult.size(); ++i)
				{
					if (PredictResult[i].flavorName == fl->flavorName)
					{
						PredictResult[i].nCount++;
						break;
					}
				}
			}
		}
		switch (nServerIndex)
		{//不同类型的服务器分开存储
		case DataCenter::GENERAL:
			GeneralServerList.push_back(currentServerFlavorList);
			break;
		case DataCenter::LARGE_MEMORY:
			LargeMemoryServerList.push_back(currentServerFlavorList);
			break;
		case DataCenter::HIGH_PERFORMANCE:
			HighPerformanceServerList.push_back(currentServerFlavorList);
			break;
		default:
			break;
		}
		nServerIndex = calcNeedServerType(PredictResult);
	}

	float occ = 0.83;

	std::vector<Flavor> cutFlavorList;
	Tools::flavorListInit(cutFlavorList, m_dc->m_FlavorList);

	for (int nServerIndex = 0; nServerIndex < GeneralServerList.size(); ++nServerIndex)
	{
		if (occ > calcOccupancyRate(GeneralServerList[nServerIndex], DataCenter::GENERAL))
		{
			for (int nFlavorIndex = 0; nFlavorIndex < GeneralServerList[nServerIndex].size(); ++nFlavorIndex)
			{
				for (int nIndex = 0; nIndex < cutFlavorList.size(); ++nIndex)
				{
					if (cutFlavorList[nIndex].flavorName == GeneralServerList[nServerIndex][nFlavorIndex].flavorName)
					{
						cutFlavorList[nIndex].nCount++;
					}
				}
			}
			GeneralServerList.erase(GeneralServerList.begin() + nServerIndex);
		}
	}

	for (int nServerIndex = 0; nServerIndex < LargeMemoryServerList.size(); ++nServerIndex)
	{
		if (occ > calcOccupancyRate(LargeMemoryServerList[nServerIndex], DataCenter::LARGE_MEMORY))
		{
			for (int nFlavorIndex = 0; nFlavorIndex < LargeMemoryServerList[nServerIndex].size(); ++nFlavorIndex)
			{
				for (int nIndex = 0; nIndex < cutFlavorList.size(); ++nIndex)
				{
					if (cutFlavorList[nIndex].flavorName == LargeMemoryServerList[nServerIndex][nFlavorIndex].flavorName)
					{
						cutFlavorList[nIndex].nCount++;
					}
				}
			}
			LargeMemoryServerList.erase(LargeMemoryServerList.begin() + nServerIndex);
		}
	}

	for (int nServerIndex = 0; nServerIndex < HighPerformanceServerList.size(); ++nServerIndex)
	{
		if (occ > calcOccupancyRate(HighPerformanceServerList[nServerIndex], DataCenter::HIGH_PERFORMANCE))
		{
			for (int nFlavorIndex = 0; nFlavorIndex < HighPerformanceServerList[nServerIndex].size(); ++nFlavorIndex)
			{
				for (int nIndex = 0; nIndex < cutFlavorList.size(); ++nIndex)
				{
					if (cutFlavorList[nIndex].flavorName == HighPerformanceServerList[nServerIndex][nFlavorIndex].flavorName)
					{
						cutFlavorList[nIndex].nCount++;
					}
				}
			}
			HighPerformanceServerList.erase(HighPerformanceServerList.begin() + nServerIndex);
		}
	}

	std::string predictResult = m_pa->getResultStr();

	earseFlavor(predictResult, cutFlavorList);
	//统计分配结果
	std::string deployResult;
	statisticsFlavor(GeneralServerList, LargeMemoryServerList, HighPerformanceServerList, deployResult);
	
	result = predictResult + "\n" + deployResult;
}
int DeployAlgorithm::calcNeedServerType(std::vector<Flavor> & PredictResult)
{//1.计算虚拟机池里那种类型的虚拟机最多
	//获取服务器类型列表
	std::vector<Server> serverList = m_dc->getServerList();
	int nGeneralFlavorCount = 0;
	int nLargeMemoryFlavorCount = 0;
	int nHighPerformanceFlavorCount = 0;

	double dbGeneralServerProportion = (double)serverList[0].serverCpuCount / (double)serverList[0].serverMemCount;
	double dbLargeMemoryServerProportion = (double)serverList[1].serverCpuCount / (double)serverList[1].serverMemCount;
	double dbHighPerformanceServerProportion = (double)serverList[2].serverCpuCount / (double)serverList[2].serverMemCount;

	int ServerIndex = -1;

	for (int flavorIndex = 0; flavorIndex < PredictResult.size(); ++flavorIndex)
	{
		double minDifference = MAXIMUM;
		double dbCurrentFlavorProportion = (double)PredictResult[flavorIndex].flavorCpu / (double)PredictResult[flavorIndex].flavorMem;
		//计算当前虚拟机跟那种服务器的类型比较接近，根据计算结果对虚拟机进行分类
		if (minDifference > fabs(dbGeneralServerProportion - dbCurrentFlavorProportion))
		{//通用型虚拟机
			minDifference = fabs(dbGeneralServerProportion - dbCurrentFlavorProportion);
			ServerIndex = DataCenter::GENERAL;
		}
		if (minDifference > fabs(dbLargeMemoryServerProportion - dbCurrentFlavorProportion))
		{//大内存型虚拟机
			minDifference = fabs(dbLargeMemoryServerProportion - dbCurrentFlavorProportion);
			ServerIndex = DataCenter::LARGE_MEMORY;
		}
		if (minDifference > fabs(dbHighPerformanceServerProportion - dbCurrentFlavorProportion))
		{//大计算类型虚拟机
			minDifference = fabs(dbHighPerformanceServerProportion - dbCurrentFlavorProportion);
			ServerIndex = DataCenter::HIGH_PERFORMANCE;
		}

		if (0 == ServerIndex)
		{//通用类型的虚拟机个数
			nGeneralFlavorCount += PredictResult[flavorIndex].nCount;
			PredictResult[flavorIndex].flavorType = DataCenter::GENERAL;
		}
		else if (1 == ServerIndex)
		{//大内存类型的虚拟机个数
			nLargeMemoryFlavorCount += PredictResult[flavorIndex].nCount;
			PredictResult[flavorIndex].flavorType = DataCenter::LARGE_MEMORY;
		}
		else if (2 == ServerIndex)
		{//大计算类型的虚拟机个数
			nHighPerformanceFlavorCount += PredictResult[flavorIndex].nCount;
			PredictResult[flavorIndex].flavorType = DataCenter::HIGH_PERFORMANCE;
		}
	}
	//统计三种类型的虚拟机类型，返回数量较多类型的索引，即找到了下一个应该放置什么类型的服务器
	if (nGeneralFlavorCount > nLargeMemoryFlavorCount)
	{//通用型较大
		if (nGeneralFlavorCount > nHighPerformanceFlavorCount)
		{//通用型最大
			return DataCenter::GENERAL;
		}
		else
		{//大算力最大
			return DataCenter::HIGH_PERFORMANCE;
		}
	}
	else
	{//大内存较大
		if (nLargeMemoryFlavorCount > nHighPerformanceFlavorCount)
		{//大内存最大
			return DataCenter::LARGE_MEMORY;
		}
		else
		{//大算力最大
			return DataCenter::HIGH_PERFORMANCE;
		}
	}
	return -1;
}

double DeployAlgorithm::DeployFlavor_GREEDY_ALGORITHM_KNAPSACK(std::string &result)
{
	std::vector<Flavor> PredictResult;
	Tools::flavorListInit(PredictResult, m_pa->getPredictResultVector());
	std::vector<std::vector<Flavor>> deployServerList; //临时存储结果

	//多重背包加贪心算法,暂时没有实现

	return calcOccupancyRate(m_optimizationModel, deployServerList);
}

double DeployAlgorithm::DeployFlavor_SIMULATED_ANNEALING(std::string &result)
{
	// 设置随机种子
	srand((unsigned)time(NULL));

	//取预测结构
	std::vector<Flavor> PredictResult;
	Tools::flavorListInit(PredictResult, m_pa->getPredictResultVector());

	std::vector<std::vector<Flavor>> deployServerList; //临时存储结果

	std::vector<std::vector<Flavor>> flavorList; // 存储三个flavor 机型分组
	int flavorCount = m_dc->getFlavorCount();
	for (int count = 0; count < m_dc->m_ServerList.size(); count++)
	{
		std::vector<Flavor> tempFlavorList; // 临时机型分组

		// 将flavor 分为3组
		for (int i = 0; i < flavorCount; i++)
		{
			Flavor currentFlavor = PredictResult[i];
			currentFlavor.nCount = floor(currentFlavor.nCount / (3 - count));
			PredictResult[i].nCount -= currentFlavor.nCount;
			tempFlavorList.push_back(currentFlavor);
		}
		flavorList.push_back(tempFlavorList);
	}

	while (m_T > m_Tmin)
	{
		std::vector<std::vector<Flavor>> tempFlavorList = flavorList; // 存储临时的分配方案，在评分高于当前评分时替换当前分配方案
		// 随机值列表
		std::vector<float> randomList;
		for (int i = 0; i < m_dc->m_ServerList.size(); i++)
		{
			randomList.push_back((rand() / double(RAND_MAX)) * 0.5);
		}

		for (int i = 0; i < randomList.size(); i++)
		{
			auto afterBatchFlavorList = tempFlavorList[(i + 1) % randomList.size()]; // 后一个机型分组
			auto currentBatchFlavorList = tempFlavorList[i];		 // 当前机型分组

			// 虚拟机数量的移动， 将当前虚拟机数量的 random% 移动到后一个分组中
			for (int j = 0; j < flavorCount; j++)
			{
				int needToMove = floor(currentBatchFlavorList[j].nCount * randomList[i]); // 需要移动的虚拟机数量
				afterBatchFlavorList[j].nCount += needToMove;							  // 后一个机型分组 + 需要移动的虚拟机数量
				currentBatchFlavorList[j].nCount -= needToMove;							  // 当前机型分组 - 需要移动的虚拟机数量
			}
		}
		std::vector<deployList_ServerTypes> tempDeployResult;
		double rate = 0;
		for (int i = 0; i < randomList.size(); i++)
		{
			deployList_ServerTypes ds;
			ds.serverType = DataCenter::GENERAL == i ? "General" : DataCenter::LARGE_MEMORY == i ? "Large-Memory" : "High-Performance";
			std::string result;
			//std::vector<std::vector<Flavor>> deployServerList; //临时存储结果
			rate += DeployFlavor_GREEDY_ALGORITHM(ds.deployServerList, result, tempFlavorList[i], i);
			tempDeployResult.push_back(ds);
		}

		// 如果大于当前评分，替换分配方案，进入下一次分配
		if (rate > m_maxRate)
		{
			m_maxRate = rate;
			m_optimizationResult = tempFlavorList;
			flavorList = tempFlavorList;
			m_deployResult = tempDeployResult;
		}
		else //如果分数降低，则以一定概率保存结果，防止优化陷入局部最优解
		{
			if (exp((m_maxRate - rate) / m_T) > rand() / RAND_MAX)
			{
				m_maxRate = rate;
				m_optimizationResult = tempFlavorList;
				flavorList = tempFlavorList;
			}
		}

		m_T = m_r * m_T; // 一次循环结束，温度降低
	}
	//string deployResult;
	//statisticsFlavor(m_optimizationResult, deployResult);
	return m_maxRate;
}

void DeployAlgorithm::DeployFlavor_DEPLOY_AUTO()
{
	std::string result_GREEDY_ALGORITHM;
	std::vector<Flavor> PredictResult;
	Tools::flavorListInit(PredictResult, m_pa->getPredictResultVector());
	std::vector<std::vector<Flavor>> deployServerList; //临时存储结果
	double OccupancyRate_GREEDY_ALGORITHM =
		DeployFlavor_GREEDY_ALGORITHM(deployServerList, result_GREEDY_ALGORITHM, PredictResult);

	std::string result_GREEDY_ALGORITHM_KNAPSACK;
	double OccupancyRate_GREEDY_ALGORITHM_KNAPSACK =
		DeployFlavor_GREEDY_ALGORITHM_KNAPSACK(result_GREEDY_ALGORITHM_KNAPSACK);

	//目前多重背包算法还没实现，直接返回贪婪算法的结果
	//m_result = OccupancyRate_GREEDY_ALGORITHM > OccupancyRate_GREEDY_ALGORITHM_KNAPSACK ?
	//	result_GREEDY_ALGORITHM : result_GREEDY_ALGORITHM_KNAPSACK;
	m_result = result_GREEDY_ALGORITHM;
}

double DeployAlgorithm::calcOccupancyRate(int optimizationModel, const std::vector<std::vector<Flavor>> &deployResult, int type)
{
	//将所有服务器的资源占用率相加，最后求平均并返回
	double rate = 0.0;
	int nServerCpuCount = m_dc->getServerCpuCount(type).serverCpuCount;
	int nServerMemCount = m_dc->getServerMemCount(type).serverMemCount;

	for (size_t nServerIndex = 0; nServerIndex < deployResult.size(); ++nServerIndex)
	{
		int nCurrentServerCpuCount = 0, nCurrentServerMemCount = 0; //单个服务器资源总和
		for (size_t nFlavorIndex = 0; nFlavorIndex < deployResult[nServerIndex].size(); ++nFlavorIndex)
		{
			nCurrentServerCpuCount += deployResult[nServerIndex][nFlavorIndex].nCount * deployResult[nServerIndex][nFlavorIndex].flavorCpu;
			nCurrentServerMemCount += deployResult[nServerIndex][nFlavorIndex].nCount * deployResult[nServerIndex][nFlavorIndex].flavorMem;
		}

		if (OPTIMIZATION_CPU == m_optimizationModel)
		{
			rate += (nCurrentServerCpuCount / nServerCpuCount);
		}

		if (OPTIMIZATION_MEM == m_optimizationModel)
		{
			rate += (nCurrentServerMemCount / nServerMemCount);
		}
	}
	rate /= (int)deployResult.size();

	return rate;
}

//检查是否存在未分配的虚拟机
bool DeployAlgorithm::checkFlavorFromFlavorList(std::vector<Flavor> &PredictResult)
{
	for (size_t i = 0; i < PredictResult.size(); ++i)
	{
		if (PredictResult[i].nCount > 0)
		{
			return true; //有剩余
		}
	}
	return false; //没有剩余
}

Flavor *DeployAlgorithm::getMaxResourceFlavor(std::vector<Flavor> &PredictResult)
{
	for (size_t maxIndex = PredictResult.size() - 1; 0 != maxIndex; --maxIndex)
	{
		if (0 != PredictResult[maxIndex].nCount)
		{
			PredictResult[maxIndex].nCount--;
			return &PredictResult[maxIndex];
		}
	}
	return nullptr;
}

//抽取一个跟服务器类型匹配且消耗资源最大的虚拟机
Flavor *DeployAlgorithm::getMatchMaxResourceFlavor(std::vector<Flavor> &PredictResult, int ServerType)
{
	for (size_t maxIndex = PredictResult.size() - 1; 0 != maxIndex; --maxIndex)
	{
		if ((0 != PredictResult[maxIndex].nCount) && (ServerType == PredictResult[maxIndex].flavorType))
		{
			PredictResult[maxIndex].nCount--;
			return &PredictResult[maxIndex];
		}
	}
	return nullptr;
}

bool DeployAlgorithm::checkResource(const std::vector<Flavor> &currentFlavorList, Flavor *fl, int serverCpu, int serverMem)
{
	int totalCPU = 0, totalMem = 0;
	for (int i = 0; i < (int)currentFlavorList.size(); ++i)
	{
		totalCPU += currentFlavorList[i].flavorCpu;
		totalMem += currentFlavorList[i].flavorMem;
	}
	if (nullptr != fl)
	{
		totalCPU += fl->flavorCpu;
		totalMem += fl->flavorMem;
	}

	if ((totalCPU > serverCpu) || (totalMem > serverMem))
	{
		return true;
	}
	return false;
}

Flavor *DeployAlgorithm::extractFlavor(std::vector<Flavor> &PredictResult, std::vector<Flavor> currentFlavorList, double serverProportion, int serverCpu, int serverMem)
{
	double tempCpu = 0.0; //CPU占用比
	double tempMem = 0.0; //MEM占用比

	int index = 0;
	double min_cpu_mem = MAXIMUM; //内存使用率与CPU使用率相差
	double max_cpu = -1;		  //最大CPU占用率
	double max_mem = -1;		  //最大MEM占用率

	if (checkFlavorFromFlavorList(PredictResult))
	{ //有剩余虚拟机才抽取
		for (size_t i = 0; i < PredictResult.size(); ++i)
		{
			if (0 < PredictResult[i].nCount)
			{
				//当前规格加上已分配规格的CPU使用率
				tempCpu = (double)(getTotalResource(currentFlavorList, CPU) + PredictResult[i].flavorCpu) / (double)serverCpu;
				//当前规格加上已分配规格的MEM使用率
				tempMem = (double)(getTotalResource(currentFlavorList, MEM) + PredictResult[i].flavorMem) / (double)serverMem;

				if (!checkResource(currentFlavorList, &PredictResult[i], serverCpu, serverMem))
				{
					//均衡优化
					double sub_cpu_mem = fabs(tempCpu - tempMem); //CPU使用率与MEM使用率之差
					if (min_cpu_mem > sub_cpu_mem)
					{ //选取CPU利用率与MEM利用率相差较低的虚拟机
						min_cpu_mem = sub_cpu_mem;
						index = i;
					}
				}
			}
		}

		if (MAXIMUM == min_cpu_mem)
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
	PredictResult[index].nCount--;
	return &PredictResult[index];
}

int DeployAlgorithm::getTotalResource(std::vector<Flavor> currentServerFlavorList, int CPU_or_MEM)
{
	int nTotal = 0;
	for (size_t i = 0; i < currentServerFlavorList.size(); ++i)
	{
		(CPU == CPU_or_MEM) ? (nTotal += currentServerFlavorList[i].flavorCpu) : (nTotal += currentServerFlavorList[i].flavorMem);
	}
	return nTotal;
}

//统计每个服务器中虚拟机的种类与个数
void DeployAlgorithm::statisticsFlavor(std::vector<std::vector<Flavor>> &deployServerList, std::string &reslut)
{
	reslut += std::to_string((int)deployServerList.size());
	//reslut += "\n";
	std::string flavorNameList;
	for (size_t i = 0; i < deployServerList.size(); ++i)
	{
		reslut += ("\n" + std::to_string(i + 1) + " ");
		flavorNameList.clear();

		double cpu = 0.0, mem = 0.0;
		for (size_t j = 0; j < deployServerList[i].size(); ++j)
		{
			cpu += deployServerList[i][j].flavorCpu; //当前服务器总共使用的CPU数
			mem += deployServerList[i][j].flavorMem; //当前服务器总共使用的内存数

			char *p = strstr((char *)flavorNameList.c_str(), std::string(deployServerList[i][j].flavorName + std::string(" ")).c_str());
			if (nullptr != p)
			{
				//找到了
				char *pPre = p, *pEnd;
				while ((' ' != *pPre) && pPre++)
					;
				pEnd = pPre + 1;
				while ((' ' != *pEnd) && pEnd++)
					;
				int nCount = stoi(std::string(pPre, pEnd));
				nCount++;
				char *pszBuf = new char[(int)flavorNameList.size() + 10];
				memset(pszBuf, 0, (int)flavorNameList.size() + 10);
				strncpy(pszBuf, flavorNameList.c_str(), flavorNameList.length() - strlen(pPre));
				strcat(pszBuf, " ");
				strcat(pszBuf, std::to_string(nCount).c_str());
				strcat(pszBuf, pEnd);
				flavorNameList = pszBuf;
				delete[] pszBuf;
			}
			else
			{ //没找到
				flavorNameList += deployServerList[i][j].flavorName;
				flavorNameList += " 1 ";
			}
		}
		//删除末尾的空格
		flavorNameList.erase((int)flavorNameList.size() - 1);
		reslut += flavorNameList;
		//reslut += flavorNameList + std::string(" cpu rate:" + std::to_string(cpu / m_dc->getServerCpuCount() * 100) + "% ") + std::string(" mem rate:" + std::to_string(mem / m_dc->getServerMemCount() * 100) + "%") + "\n";
	}
	//经测试发现，最后一台服务器可能没有装任何虚拟机，故在此处检查，发现一台空的服务器则移除
	char *pLastServer = strrchr((char *)reslut.c_str(), '\n');
	if (nullptr == strstr(pLastServer, "flavor"))
	{
		memset(pLastServer, 0, strlen(pLastServer));
		int nServerCount = std::stoi(reslut);
		nServerCount--;
		char *pTail = strstr((char *)reslut.c_str(), "\n");
		std::string tempResult = std::to_string(nServerCount);
		tempResult += pTail;
		reslut.clear();
		reslut = tempResult;
	}
}

void DeployAlgorithm::statisticsFlavor(std::vector<std::vector<Flavor>> & GeneralServerList, std::vector<std::vector<Flavor>> & LargeMemoryServerList, 
	std::vector<std::vector<Flavor>> & HighPerformanceServerList, std::string &reslut)
{
	reslut += std::string("General ") + std::to_string(GeneralServerList.size()) + "\n";
	for (int nIndex = 0; nIndex < GeneralServerList.size(); ++nIndex)
	{
		reslut += "General-" + std::to_string(nIndex + 1) + " ";
		std::string currentServerFlavor;
		int nTotalCpu = 0;
		int nTotalMem = 0;
		for (int nFlavorIndex = 0; nFlavorIndex < GeneralServerList[nIndex].size(); ++nFlavorIndex)
		{
			nTotalCpu += GeneralServerList[nIndex][nFlavorIndex].flavorCpu;
			nTotalMem += GeneralServerList[nIndex][nFlavorIndex].flavorMem;
			insertFlavor(currentServerFlavor, GeneralServerList[nIndex][nFlavorIndex].flavorName);
		}
		//reslut += currentServerFlavor + "            cpu:" + std::to_string((nTotalCpu / 56.0) * 100.0) + "%   mem:" + std::to_string((nTotalMem / 128.0) * 100.0) + "%";
		reslut += currentServerFlavor;
		reslut += "\n";
	}
	reslut += std::string("\nLarge-Memory ") + std::to_string(LargeMemoryServerList.size()) + "\n";
	for (int nIndex = 0; nIndex < LargeMemoryServerList.size(); ++nIndex)
	{
		reslut += "Large-Memory-" + std::to_string(nIndex + 1) + " ";
		std::string currentServerFlavor;
		int nTotalCpu = 0;
		int nTotalMem = 0;
		for (int nFlavorIndex = 0; nFlavorIndex < LargeMemoryServerList[nIndex].size(); ++nFlavorIndex)
		{
			nTotalCpu += LargeMemoryServerList[nIndex][nFlavorIndex].flavorCpu;
			nTotalMem += LargeMemoryServerList[nIndex][nFlavorIndex].flavorMem;
			insertFlavor(currentServerFlavor, LargeMemoryServerList[nIndex][nFlavorIndex].flavorName);
		}
		//reslut += currentServerFlavor + "            cpu:" + std::to_string((nTotalCpu / 84.0) * 100.0) + "%   mem:" + std::to_string((nTotalMem / 256.0) * 100.0) + "%";
		reslut += currentServerFlavor;
		reslut += "\n";
	}
	reslut += std::string("\nHigh-Performance ") + std::to_string(HighPerformanceServerList.size()) + "\n";
	for (int nIndex = 0; nIndex < HighPerformanceServerList.size(); ++nIndex)
	{
		reslut += "High-Performance-" + std::to_string(nIndex + 1) + " ";
		std::string currentServerFlavor;
		int nTotalCpu = 0;
		int nTotalMem = 0;
		for (int nFlavorIndex = 0; nFlavorIndex < HighPerformanceServerList[nIndex].size(); ++nFlavorIndex)
		{
			nTotalCpu += HighPerformanceServerList[nIndex][nFlavorIndex].flavorCpu;
			nTotalMem += HighPerformanceServerList[nIndex][nFlavorIndex].flavorMem;
			insertFlavor(currentServerFlavor, HighPerformanceServerList[nIndex][nFlavorIndex].flavorName);
		}
		//reslut += currentServerFlavor + "    cpu:" + std::to_string((nTotalCpu / 112.0) * 100.0) + "%   mem:" + std::to_string((nTotalMem / 192.0) * 100.0) + "%";
		reslut += currentServerFlavor;
		reslut += "\n";
	}
}
void DeployAlgorithm::insertFlavor(std::string & currentServerFlavor, std::string & flavorName)
{
	std::vector<std::string> ss = Tools::split(currentServerFlavor, flavorName + " ");
	if (1 != ss.size())
	{//字符串中已经存在该虚拟机名称
		int nCount = std::stoi(ss[1]);
		nCount++;
		const char * pSpace = ss[1].c_str();
		while ((' ' != *pSpace) && (pSpace++));
		pSpace++;
		currentServerFlavor = ss[0] + flavorName + " " + std::to_string(nCount) + std::string(" ") + pSpace;
	}
	else
	{//字符串中不存在该虚拟机名称
		currentServerFlavor += flavorName + " 1 ";
	}
}
float DeployAlgorithm::calcOccupancyRate(int optimizationModel, const std::vector<Flavor> &cur_deployResult, int type)
{
	int nTotalCpu = 0;
	int nTotalMem = 0;
	for (int i = 0; i < cur_deployResult.size(); ++i)
	{
		nTotalCpu += cur_deployResult[i].flavorCpu;
		nTotalMem += cur_deployResult[i].flavorMem;
	}
	if (OPTIMIZATION_CPU == m_optimizationModel)
	{
		return ((float)nTotalCpu / (float)m_dc->getServerCpuCount(type).serverCpuCount);
	}

	if (OPTIMIZATION_MEM == m_optimizationModel)
	{
		return ((float)nTotalMem / (float)m_dc->getServerMemCount(type).serverMemCount);
	}
	return 0;
}

double DeployAlgorithm::calcOccupancyRate(const std::vector<std::vector<Flavor>> &deployResult, int type)
{
	//将所有服务器的资源占用率相加，最后求平均并返回
	double rate = 0.0;
	int nServerCpuCount = m_dc->getServerCpuCount(type).serverCpuCount;
	int nServerMemCount = m_dc->getServerMemCount(type).serverMemCount;

	for (size_t nServerIndex = 0; nServerIndex < deployResult.size(); ++nServerIndex)
	{//多个服务器资源综合
		int nCurrentServerCpuCount = 0, nCurrentServerMemCount = 0; //单个服务器资源总和
		for (size_t nFlavorIndex = 0; nFlavorIndex < deployResult[nServerIndex].size(); ++nFlavorIndex)
		{//单个服务器的资源总和
			//nCurrentServerCpuCount += deployResult[nServerIndex][nFlavorIndex].nCount * deployResult[nServerIndex][nFlavorIndex].flavorCpu;
			//nCurrentServerMemCount += deployResult[nServerIndex][nFlavorIndex].nCount * deployResult[nServerIndex][nFlavorIndex].flavorMem;
			nCurrentServerCpuCount += deployResult[nServerIndex][nFlavorIndex].flavorCpu;
			nCurrentServerMemCount += deployResult[nServerIndex][nFlavorIndex].flavorMem;
		}

		rate += (nCurrentServerCpuCount / nServerCpuCount) * 0.5;

		rate += (nCurrentServerMemCount / nServerMemCount) * 0.5;
	}
	rate /= (int)deployResult.size();

	return rate;
}

float DeployAlgorithm::calcOccupancyRate(const std::vector<Flavor> &cur_deployResult, int nServerType)
{
	int nTotalCpu = 0;
	int nTotalMem = 0;

	for (int nIndex = 0; nIndex < cur_deployResult.size(); ++nIndex)
	{
		nTotalCpu += cur_deployResult[nIndex].flavorCpu;
		nTotalMem += cur_deployResult[nIndex].flavorMem;
	}
	float cpu_occ = (float)nTotalCpu / (float)m_dc->getServerCpuCount(nServerType).serverCpuCount;
	float mem_occ = (float)nTotalMem / (float)m_dc->getServerCpuCount(nServerType).serverMemCount;

	return cpu_occ < mem_occ ? cpu_occ : mem_occ;
}

void DeployAlgorithm::earseFlavor(std::string & predictResult, std::vector<Flavor> & cutList)
{
	for (int nIndex = 0; nIndex < cutList.size(); ++nIndex)
	{
		size_t szIndex = predictResult.find(cutList[nIndex].flavorName);
		szIndex += cutList[nIndex].flavorName.length() + 1;
		std::string sCount(predictResult.begin() + szIndex, predictResult.end());
		int nCount = std::stoi(sCount);
		nCount -= cutList[nIndex].nCount;
		cutList[nIndex].nCount = nCount;
	}

	int nTotalPredictCount = 0;
	//预测结果临时存储在Flavor结构体的nCount中

	std::string result;
	std::string tempStr;

	for (size_t nFlavorIndex = 0; nFlavorIndex < cutList.size(); ++nFlavorIndex)
	{
		nTotalPredictCount += cutList[nFlavorIndex].nCount;
		tempStr += cutList[nFlavorIndex].flavorName;
		tempStr += " ";
		tempStr += std::to_string(cutList[nFlavorIndex].nCount);
		tempStr += "\n";
	}
	result = std::to_string(nTotalPredictCount);
	result += "\n";
	result += tempStr;
	// return result;

	predictResult = result;
}