#ifndef DATA_CENTER_H_
#define DATA_CENTER_H_

#include "lib_io.h"
#include "define.h"

class DataCenter
{
  public:
	friend class PredictAlgorithm;
	friend class DeployAlgorithm;
  public:
	enum
	{//Model
		DAY,				 //按天分割
		STRICT_TIME_QUANTUM, //严格的时间段分割, 7天一个单位(根据UNIX时间戳)
		SLACK_TIME_QUANTUM,  //松弛的时间段分割，7天一个单位(根据日期分割)
	};
	enum
	{// 物理机类型
		GENERAL,		  // 通用
		LARGE_MEMORY,	  // 大内存
		HIGH_PERFORMANCE, // 高性能
	};

	DataCenter();
	DataCenter(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM], int data_num, int model = DataCenter::SLACK_TIME_QUANTUM);
	~DataCenter();
	void setTrainData(char *data[MAX_DATA_NUM], int data_num);
	void setInputData(char *info[MAX_INFO_NUM]);
	void setSplitModel(int model = DataCenter::SLACK_TIME_QUANTUM);

	bool start();
	const std::string &getError();

  private:
	void readTrainData();
	void readInputData();

	//分割数据
	void splitTrainData();
	void splitTrainData_DAY();
	void splitTrainData_SLACK_TIME_QUANTUM();
	void splitTrainData_STRICT_TIME_QUANTUM(int time_lenth);

	//获取时间周期数
	size_t getTimeQuantumCount();
	//获取第一个时间周期规格列表
	bool getFirstTimeQuantumFlavorList(std::vector<Flavor> &fl);
	//获取下一个时间周期规格列表
	bool getNextTimeQuantumFlavorList(std::vector<Flavor> &fl);

	//获取规格总数
	size_t getFlavorCount();
	//获取每个时间周期中指定规格的列表
	bool getFirstFlavorListFromTimeQuantum(std::vector<Flavor> &fl);
	//获取下一个指定规格的列表
	bool getNextFlavorListFromTimeQuantum(std::vector<Flavor> &fl);

	//获取指定周期指定规格出现的次数
	int getFlavorCount(int nTimeQuantum, int nFlavor);

	//获取分割模式
	int getSplitModel();
	time_t getPredictStartTime_unix();
	time_t getPredictEndTime_unix();

	std::string getFlavorName(int nTimeQuantum, int nFlavor);

	//设置、获取平滑值
	void setFlavor_s1(int nTimeQuantum, int nFlavor, double value);
	double getFlavor_s1(int nTimeQuantum, int nFlavor);
	void setFlavor_s2(int nTimeQuantum, int nFlavor, double value);
	double getFlavor_s2(int nTimeQuantum, int nFlavor);
	void setFlavor_s3(int nTimeQuantum, int nFlavor, double value);
	double getFlavor_s3(int nTimeQuantum, int nFlavor);

	//获取Input文件中的优化方式 MEM or CPU
	//int getOptimizationModel();

	Server getServerCpuCount(int type = DataCenter::GENERAL);
	Server getServerMemCount(int type = DataCenter::GENERAL);

	//当前天数与第一天之间的天数
	int getDayFromCurrentToFirstDay(std::string strTime);

	//获取时间段长度
	int getTimeQuantumLenth()
	{
		return ((m_endPredictTime_unix - m_startPredictTime_unix) / (24 * 3600));
	}

	std::vector<Server> & getServerList()
	{
		return m_ServerList;
	}
#ifdef _DEBUG
	void showTotalFlavor();
#endif // ! _DEBUG
	time_t getTrainStartTime_unix()
	{
		return m_startTrainTime_unix;
	}
	time_t getTrainEndTime_unix()
	{
		return m_endTrainTime_unix;
	}
  private:
	//input
	char **inputTrain;
	char **inputFlavor;
	int m_data_num;

	//传入的训练数据与规格列表
	std::vector<Train> m_TrainList;   
	std::vector<Flavor> m_FlavorList;

	//分割完成的列表，外层为时间段，内层为规格
	std::vector<std::vector<Flavor>> m_SplitedFlavorList;

	//时间段索引和规格索引，方便循环使用
	size_t m_currentTimeQuantumIndex;
	size_t m_FlavorIndex;

	//传入文件读取到的信息 [input.txt]
	std::vector<Server> m_ServerList;

	int m_flavorCount;		 // 虚拟机数量
	//int m_optimizationModel; //传入文件中读取到的优化模式 CPU or MEM

	//训练数据的开始与结束时间
	std::string m_startTrainTime;
	time_t m_startTrainTime_unix;
	std::string m_endTrainTime;
	time_t m_endTrainTime_unix;

	std::string m_startPredictTime;
	time_t m_startPredictTime_unix;
	std::string m_endPredictTime;
	time_t m_endPredictTime_unix;

	//分割模式
	int m_splitModel;

	std::string m_error;
};

#endif
