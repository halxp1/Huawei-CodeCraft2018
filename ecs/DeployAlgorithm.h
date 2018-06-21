#ifndef DEPLOY_SERVER_H_
#define DEPLOY_SERVER_H_
#include "define.h"
#include "DataCenter.h"

class PredictAlgorithm;
class DataCenter;

class DeployAlgorithm
{
  public:
	enum
	{							   //Algorithm
		GREEDY_ALGORITHM,		   //贪婪算法
		GREEDY_ALGORITHM_KNAPSACK, //多重背包加贪心算法
		DEPLOY_AUTO,			   //自动选择部署算法
		SIMULATED_ANNEALING		   //模拟退火
	};

	DeployAlgorithm();
	DeployAlgorithm(PredictAlgorithm &pa, DataCenter &dc, int deploy_model = DEPLOY_AUTO);
	~DeployAlgorithm();
	void setPredictAlgorithmObject(PredictAlgorithm &pa, DataCenter &dc);
	std::string &getDeployResultStr();
	void setDeployModel(int deployModel = DEPLOY_AUTO);
	std::string &getError();
	void startDeploy();

  private:
	//部署虚拟机
	void DeployFlavor();

	//各个预测算法分别执行以下函数,返回值返回平均占用率，
	//优化CPU则返回CPU平均占用率，优化MEM则返回平均MEM占用率
	double DeployFlavor_GREEDY_ALGORITHM(std::vector<std::vector<Flavor>> & deployServerList, std::string &result, std::vector<Flavor> PredictResult, int type = DataCenter::GENERAL);
	void DeployFlavor_GREEDY_ALGORITHM(std::string &result);
	double DeployFlavor_GREEDY_ALGORITHM_KNAPSACK(std::string &result);
	double DeployFlavor_SIMULATED_ANNEALING(std::string &result);

	//DeployFlavor_DEPLOY_AUTO执行以上两种分配算法，选择平均占用率高的作为最终结果
	void DeployFlavor_DEPLOY_AUTO();

	//根据优化方式计算资源的平均占用率
	double calcOccupancyRate(int optimizationModel, const std::vector<std::vector<Flavor>> &deployResult, int type = DataCenter::GENERAL);

	float calcOccupancyRate(int optimizationModel, const std::vector<Flavor> &cur_deployResult, int type = DataCenter::GENERAL);

	float calcOccupancyRate(const std::vector<Flavor> &cur_deployResult, int nServerType);

	double calcOccupancyRate(const std::vector<std::vector<Flavor>> &deployResult, int type = DataCenter::GENERAL);
	/*************************************贪心算法子函数**************************************/

	//检查是否存在未分配的虚拟机
	bool checkFlavorFromFlavorList(std::vector<Flavor> &PredictResult);

	//抽取一个最大的虚拟机
	Flavor *getMaxResourceFlavor(std::vector<Flavor> &PredictResult);

	//抽取一个跟服务器类型匹配且消耗资源最大的虚拟机
	Flavor *getMatchMaxResourceFlavor(std::vector<Flavor> &PredictResult, int ServerType);

	//检查是否存在超分, 存在超分返回true，不存在超分返回false
	bool checkResource(const std::vector<Flavor> &PredictResult, Flavor *fl, int serverCpu, int serverMem);

	//根据当前服务器CPU与MEM的比值，抽取一个最适合的虚拟机，使得虚拟机部署到服务器后，对总体CPU比MEM影响最小
	Flavor *extractFlavor(std::vector<Flavor> &PredictResult, std::vector<Flavor> currentFlavorList, double serverProportion, int serverCpu, int serverMem);

	//获取指定资源的总数 CPU or MEM
	int getTotalResource(std::vector<Flavor> currentServerFlavorList, int CPU_or_MEM);

	void statisticsFlavor(std::vector<std::vector<Flavor>> &deployServerList, std::string &reslut);

	void statisticsFlavor(std::vector<std::vector<Flavor>> & GeneralServerList, std::vector<std::vector<Flavor>> & LargeMemoryServerList, 
		std::vector<std::vector<Flavor>> & HighPerformanceServerList, std::string &reslut);
	/*************************************贪心算法子函数**************************************/

	/********************************贪心+多重背包算法子函数**********************************/

	/********************************贪心+多重背包算法子函数**********************************/

	//计算需要的服务器类型
	int calcNeedServerType(std::vector<Flavor> & PredictResult);

	void insertFlavor(std::string & currentServerFlavor, std::string & flavorName);

	void earseFlavor(std::string & predictResult, std::vector<Flavor> & cutList);
  private:
	PredictAlgorithm *m_pa;
	DataCenter *m_dc;
	std::string m_result;
	int m_deployModel;
	std::string m_error;

	//优化方式 CPU or MEM
	int m_optimizationModel;

	// 模拟退火变量
	double m_T;   //模拟退火初始温度
	double m_Tmin;	//模拟退火终止温度
	double m_maxRate; // 最高评分
	std::vector<std::vector<Flavor>> m_optimizationResult;
	double m_r; //温度下降系数
			  // std::vector<int> dice; //骰子，每次随机投掷，取vector前两个变量作为每次退火需要交换顺序的虚拟机

	std::vector<deployList_ServerTypes> m_deployResult;
};

#endif