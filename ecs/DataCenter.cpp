#include "DataCenter.h"
#include "Tools.h"
#include <iostream>
#include <algorithm>

DataCenter::DataCenter()
{
	m_TrainList.clear();
	m_FlavorList.clear();
	m_SplitedFlavorList.clear();
	m_data_num = 0;

	m_flavorCount = 0;
	//m_optimizationModel = -1;

	inputTrain = nullptr;
	inputFlavor = nullptr;

	m_error.clear();
	m_currentTimeQuantumIndex = -1;
	m_FlavorIndex = -1;

	m_startTrainTime.clear();
	m_startTrainTime_unix = 0;
	m_endTrainTime.clear();
	m_endTrainTime_unix = 0;

	m_startPredictTime.clear();
	m_startPredictTime_unix = 0;
	m_endPredictTime.clear();
	m_endPredictTime_unix = 0;
}

DataCenter::DataCenter(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM], int data_num, int model)
{
	m_TrainList.clear();
	m_FlavorList.clear();
	m_SplitedFlavorList.clear();
	m_data_num = data_num;

	m_flavorCount = 0;
	//m_optimizationModel = -1;

	inputTrain = data;
	inputFlavor = info;

	m_error.clear();
	m_splitModel = model;
	m_currentTimeQuantumIndex = -1;
	m_FlavorIndex = -1;

	m_startTrainTime.clear();
	m_startTrainTime_unix = 0;
	m_endTrainTime.clear();
	m_endTrainTime_unix = 0;

	m_startPredictTime.clear();
	m_startPredictTime_unix = 0;
	m_endPredictTime.clear();
	m_endPredictTime_unix = 0;
}

DataCenter::~DataCenter()
{
}

void DataCenter::setTrainData(char *data[MAX_DATA_NUM], int data_num)
{
	inputTrain = data;
	m_data_num = data_num;
}

void DataCenter::setInputData(char *info[MAX_INFO_NUM])
{
	inputFlavor = info;
}

void DataCenter::setSplitModel(int model)
{
	m_splitModel = model;
}

void DataCenter::readTrainData()
{
	for (int nIndex = 0; nIndex < m_data_num; ++nIndex)
	{
		std::string strRow = inputTrain[nIndex];
		std::vector<std::string> sp = Tools::split(strRow, "\t");

		Train tr;
		tr.id = sp[0];
		tr.flavorName = sp[1];
		tr.createaTime = sp[2];

		m_TrainList.push_back(tr);
	}
	m_startTrainTime = m_TrainList[0].createaTime;
	m_startTrainTime_unix = Tools::GetUnixTime(m_startTrainTime);
	m_endTrainTime = m_TrainList[(int)m_TrainList.size() - 1].createaTime;
	m_endTrainTime_unix = Tools::GetUnixTime(m_endTrainTime);
}

void DataCenter::readInputData()
{
	int nServerCount = std::stoi(inputFlavor[0]);
	for (int i = 0; i < nServerCount; i++)
	{

		// 物理机 类型
		std::string serverInfo = inputFlavor[i + 1];
		std::vector<std::string> sp = Tools::split(serverInfo, " ");
		Server server;

		server.serverCpuCount = std::stoi(sp[1]);
		server.serverMemCount = std::stoi(sp[2]);
		server.serverDiskCount = std::stoi(sp[3]);
		m_ServerList.push_back(server);
	}

	std::string flavorCount = inputFlavor[5];
	m_flavorCount = std::stoi(flavorCount);   //虚拟机数量

	int currentIndex_inputFlavor = 5;
	for (int nIndex = 0; nIndex < m_flavorCount; ++nIndex)
	{
		std::string strRow = inputFlavor[nIndex + 6];
		std::vector<std::string> sp = Tools::split(strRow, " ");

		Flavor fl;
		fl.flavorName = sp[0];
		fl.flavorCpu = std::stoi(sp[1]);
		fl.flavorMem = std::stoi(sp[2]) / 1024; //将内存的单位转换为G(原来为M)

		m_FlavorList.push_back(fl);
		currentIndex_inputFlavor = nIndex + 6;
	}

	Tools::sort(m_FlavorList);

	// currentIndex_inputFlavor += 2;
	// std::string optimizationModel = inputFlavor[currentIndex_inputFlavor];

	// transform(optimizationModel.begin(), optimizationModel.end(), optimizationModel.begin(), tolower);

	// if (0 == strncmp(optimizationModel.c_str(), "cpu", 3))
	// {
	// 	m_optimizationModel = OPTIMIZATION_CPU;
	// }
	// else if (0 == strncmp(optimizationModel.c_str(), "mem", 3))
	// {
	// 	m_optimizationModel = OPTIMIZATION_MEM;
	// }

	currentIndex_inputFlavor += 2;
	m_startPredictTime = inputFlavor[currentIndex_inputFlavor];
	m_startPredictTime_unix = Tools::GetUnixTime(m_startPredictTime);

	currentIndex_inputFlavor += 1;
	m_endPredictTime = inputFlavor[currentIndex_inputFlavor];
	m_endPredictTime_unix = Tools::GetUnixTime(m_endPredictTime);
}

void DataCenter::splitTrainData()
{
	if (m_splitModel == DataCenter::DAY)
	{
		splitTrainData_DAY();
	}
	else if (m_splitModel == DataCenter::SLACK_TIME_QUANTUM)
	{
		splitTrainData_SLACK_TIME_QUANTUM();
	}
	else if (m_splitModel == DataCenter::STRICT_TIME_QUANTUM)
	{
		splitTrainData_STRICT_TIME_QUANTUM(7);
	}
#ifdef _DEBUG
		//showTotalFlavor();
#endif // ! _DEBUG
}

void DataCenter::splitTrainData_DAY()
{
	int nTotal = 0;
	std::string strTrainEndTime = m_TrainList[(int)m_TrainList.size() - 1].createaTime;
	std::vector<std::string> sp_day = Tools::split(strTrainEndTime, "-");
	std::vector<std::string> sp_time = Tools::split(sp_day[2], " ");
	int day = 0;
	if (std::string(sp_time[1]) != std::string("00:00:00"))
	{
		day = std::stoi(sp_time[0]);
		day++;
	}
	else
	{
		day = std::stoi(sp_time[0]);
	}
	std::string splitEndTime = sp_day[0] + "-" + sp_day[1] + "-" + std::to_string(day) + " 00:00:00";
	time_t splitEndTime_unix = Tools::GetUnixTime(splitEndTime);

	std::string strTrainStartTime = m_TrainList[0].createaTime;
	sp_day = Tools::split(strTrainStartTime, "-");
	sp_time = Tools::split(sp_day[2], " ");

	std::string splitStartTime = sp_day[0] + "-" + sp_day[1] + "-" + sp_day[1] + " 00:00:00";
	time_t splitStartTime_unix = Tools::GetUnixTime(splitStartTime);

	time_t days = (splitEndTime_unix - splitStartTime_unix) / (3600 * 24);

	size_t stCurrentTrainListIndex = 0;
	for (int nDays = 0; nDays < days; ++nDays)
	{
		std::vector<Flavor> flavorDay(m_FlavorList);

		for (size_t nIndex = stCurrentTrainListIndex; nIndex < m_TrainList.size(); ++nIndex)
		{
			time_t seconds = Tools::GetUnixTime(m_TrainList[nIndex].createaTime) - (splitStartTime_unix + (nDays)*24 * 3600);
			if (seconds >= 24 * 3600)
			{
				stCurrentTrainListIndex = nIndex;
				break;
			}
			else
			{
				for (size_t i = 0; i < m_FlavorList.size(); ++i)
				{
					if (m_TrainList[nIndex].flavorName == flavorDay[i].flavorName)
					{
#ifdef _DEBUG
					//std::cout << nIndex << " ";
#endif
						flavorDay[i].nCount++;
						nTotal++;
						break;
					}
				}
			}
		}

		m_SplitedFlavorList.push_back(flavorDay);
	}
#ifdef _DEBUG
		//std::cout << "\nnTotal:" << nTotal << std::endl;
#endif
}

void DataCenter::splitTrainData_SLACK_TIME_QUANTUM()
{
	int nTotal = 0;
	std::string strTrainStartTime = m_TrainList[0].createaTime;
	std::vector<std::string> sp_start_month = Tools::split(strTrainStartTime, "-");
	std::vector<std::string> sp_start_day = Tools::split(sp_start_month[2], " ");

	std::string strTrainEndTime = m_TrainList[(int)m_TrainList.size() - 1].createaTime;
	std::vector<std::string> sp_end_month = Tools::split(strTrainEndTime, "-");
	std::vector<std::string> sp_end_day = Tools::split(sp_end_month[2], " ");

	int total_train_days = (std::stoi(sp_end_month[1]) - std::stoi(sp_start_month[1])) * 30 + (std::stoi(sp_end_day[0]) - std::stoi(sp_start_day[0])) + 1;

	std::vector<std::string> sp_predict_start_month = Tools::split(m_startPredictTime, "-");
	std::vector<std::string> sp_predict_start_day = Tools::split(sp_predict_start_month[2], " ");

	std::vector<std::string> sp_predict_end_month = Tools::split(m_endPredictTime, "-");
	std::vector<std::string> sp_predict_end_day = Tools::split(sp_predict_end_month[2], " ");

	int predict_days = (std::stoi(sp_predict_end_month[1]) - std::stoi(sp_predict_start_month[1])) * 30 + (std::stoi(sp_predict_end_day[0]) - std::stoi(sp_predict_start_day[0]));

	int time_quantum = total_train_days / 7;

	size_t stCurrentTrainListIndex = m_TrainList.size() - 1; //?
	int r_time_quantum = 0;

	for (int nTimeQuantum = time_quantum; nTimeQuantum != 0; --nTimeQuantum)
	{
		std::vector<Flavor> flavorDay(m_FlavorList);
		for (size_t nIndex = stCurrentTrainListIndex; nIndex != -1; --nIndex)
		{
			int nDays = getDayFromCurrentToFirstDay(m_TrainList[nIndex].createaTime);
			if (0 > (nDays - 7 * (nTimeQuantum - 1)))
			{ //判断当前这条数据是否在当前周期内
				r_time_quantum++;
				stCurrentTrainListIndex = nIndex;
				break;
			}
			else
			{
				for (size_t i = 0; i < m_FlavorList.size(); ++i)
				{
					if (m_TrainList[nIndex].flavorName == flavorDay[i].flavorName)
					{
#ifdef _DEBUG
					//std::cout << nIndex << " ";
#endif
						flavorDay[i].nCount++;
						nTotal++;
						break;
					}
				}
			}
		}
		m_SplitedFlavorList.push_back(flavorDay);
	}
	reverse(m_SplitedFlavorList.begin(), m_SplitedFlavorList.end());

	//showTotalFlavor();

#ifdef _DEBUG
	//std::cout << "\nnTotal:" << nTotal << std::endl;
#endif
}

void DataCenter::splitTrainData_STRICT_TIME_QUANTUM(int time_lenth)
{
	int nTotal = 0;
	std::string strTrainEndTime = m_TrainList[(int)m_TrainList.size() - 1].createaTime;
	std::vector<std::string> sp_day = Tools::split(strTrainEndTime, "-");
	std::vector<std::string> sp_time = Tools::split(sp_day[2], " ");
	int day = 0;
	if (std::string(sp_time[1]) != std::string("00:00:00"))
	{
		day = std::stoi(sp_time[0]);
		day++;
	}
	else
	{
		day = std::stoi(sp_time[0]);
	}
	std::string splitEndTime = sp_day[0] + "-" + sp_day[1] + "-" + std::to_string(day) + " 00:00:00";
	time_t splitEndTime_unix = Tools::GetUnixTime(splitEndTime);

	std::string strTrainStartTime = m_TrainList[0].createaTime;
	sp_day = Tools::split(strTrainStartTime, "-");
	sp_time = Tools::split(sp_day[2], " ");

	std::string splitStartTime = sp_day[0] + "-" + sp_day[1] + "-" + sp_day[1] + " 00:00:00";
	time_t splitStartTime_unix = Tools::GetUnixTime(splitStartTime);

	double time_len = (double)(splitEndTime_unix - splitStartTime_unix) / (double)(24 * 3600 * time_lenth);

	int time_quantum = (int)time_len + (fmod(time_len, 1.0) > 0.0 ? 1 : 0);

	size_t stCurrentTrainListIndex = m_TrainList.size() - 1;
	int r_time_quantum = 0;
	for (int nTimeQuantum = time_quantum; nTimeQuantum != 0; --nTimeQuantum)
	{
		std::vector<Flavor> flavorDay(m_FlavorList);
		for (size_t nIndex = stCurrentTrainListIndex; nIndex != -1; --nIndex)
		{
			time_t space = splitEndTime_unix - Tools::GetUnixTime(m_TrainList[nIndex].createaTime) - r_time_quantum * time_lenth * 24 * 3600;
			if (space >= 24 * 3600 * time_lenth)
			{
				r_time_quantum++;
				stCurrentTrainListIndex = nIndex;
				break;
			}
			else
			{
				for (size_t i = 0; i < m_FlavorList.size(); ++i)
				{
					if (m_TrainList[nIndex].flavorName == flavorDay[i].flavorName)
					{
#ifdef _DEBUG
						std::cout << nIndex << " ";
#endif
						flavorDay[i].nCount++;
						nTotal++;
						break;
					}
				}
			}
		}
		m_SplitedFlavorList.push_back(flavorDay);
	}
	reverse(m_SplitedFlavorList.begin(), m_SplitedFlavorList.end());
#ifdef _DEBUG
	std::cout << "\nnTotal:" << nTotal << std::endl;
#endif
}

bool DataCenter::start()
{
	//分割数据以前，先清空缓冲区
	m_TrainList.clear();
	m_FlavorList.clear();
	m_SplitedFlavorList.clear();

	if ((nullptr != inputTrain) && (nullptr != inputFlavor))
	{
		readTrainData();
		readInputData();
		splitTrainData();
		return true;
	}
	m_error += "\nERROR!!!!!\nclass [DataCenter]: \n  Can not found any input data or info.\n  Do you forget \
to call DataCenter::setTrainData() or DataCenter::setInputData() befor use this class ?\n";
	return false;
}

const std::string &DataCenter::getError()
{
	return m_error;
}

size_t DataCenter::getTimeQuantumCount()
{
	return m_SplitedFlavorList.size();
}

bool DataCenter::getFirstTimeQuantumFlavorList(std::vector<Flavor> &fl)
{
	m_currentTimeQuantumIndex = 0;
	if (m_currentTimeQuantumIndex < getTimeQuantumCount())
	{
		fl = m_SplitedFlavorList[m_currentTimeQuantumIndex];
		return true;
	}
	m_error += "[DataCenter] ERROR!!!\n  TimeQuantumIndex error, current TimeQuantumIndex is:" + std::to_string(m_currentTimeQuantumIndex);
	return false;
}
bool DataCenter::getNextTimeQuantumFlavorList(std::vector<Flavor> &fl)
{
	m_currentTimeQuantumIndex++;
	if (m_currentTimeQuantumIndex < getTimeQuantumCount())
	{
		fl = m_SplitedFlavorList[m_currentTimeQuantumIndex];
		return true;
	}
	return false;
}

size_t DataCenter::getFlavorCount()
{
	return m_flavorCount;
}

bool DataCenter::getFirstFlavorListFromTimeQuantum(std::vector<Flavor> &fl)
{
	m_FlavorIndex = 0;
	if (m_FlavorIndex < getFlavorCount())
	{
		fl.clear();
		for (size_t nTimeQuantumIndex = 0; nTimeQuantumIndex < m_SplitedFlavorList.size(); ++nTimeQuantumIndex)
		{
			fl.push_back(m_SplitedFlavorList[nTimeQuantumIndex][m_FlavorIndex]);
		}
		return true;
	}
	m_error += "[DataCenter] ERROR!!!\n  FlavorIndex error, current FlavorIndex is:" + std::to_string(m_FlavorIndex);
	return false;
}

bool DataCenter::getNextFlavorListFromTimeQuantum(std::vector<Flavor> &fl)
{
	m_FlavorIndex++;
	if (m_FlavorIndex < getFlavorCount())
	{
		fl.clear();
		for (size_t nTimeQuantumIndex = 0; nTimeQuantumIndex < m_SplitedFlavorList.size(); ++nTimeQuantumIndex)
		{
			fl.push_back(m_SplitedFlavorList[nTimeQuantumIndex][m_FlavorIndex]);
		}
		return true;
	}
	return false;
}

int DataCenter::getFlavorCount(int nTimeQuantum, int nFlavor)
{
	return m_SplitedFlavorList[nTimeQuantum][nFlavor].nCount;
}

std::string DataCenter::getFlavorName(int nTimeQuantum, int nFlavor)
{
	return m_SplitedFlavorList[nTimeQuantum][nFlavor].flavorName;
}

void DataCenter::setFlavor_s1(int nTimeQuantum, int nFlavor, double value)
{
	m_SplitedFlavorList[nTimeQuantum][nFlavor].s1 = value;
}

void DataCenter::setFlavor_s2(int nTimeQuantum, int nFlavor, double value)
{
	m_SplitedFlavorList[nTimeQuantum][nFlavor].s2 = value;
}

void DataCenter::setFlavor_s3(int nTimeQuantum, int nFlavor, double value)
{
	m_SplitedFlavorList[nTimeQuantum][nFlavor].s3 = value;
}

int DataCenter::getSplitModel()
{
	return m_splitModel;
}

time_t DataCenter::getPredictStartTime_unix()
{
	return m_startPredictTime_unix;
}
time_t DataCenter::getPredictEndTime_unix()
{
	return m_endPredictTime_unix;
}

double DataCenter::getFlavor_s1(int nTimeQuantum, int nFlavor)
{
	return m_SplitedFlavorList[nTimeQuantum][nFlavor].s1;
}

double DataCenter::getFlavor_s2(int nTimeQuantum, int nFlavor)
{
	return m_SplitedFlavorList[nTimeQuantum][nFlavor].s2;
}

double DataCenter::getFlavor_s3(int nTimeQuantum, int nFlavor)
{
	return m_SplitedFlavorList[nTimeQuantum][nFlavor].s3;
}

//int DataCenter::getOptimizationModel()
//{
//	return m_optimizationModel;
//}

Server DataCenter::getServerCpuCount(int type)
{
	return m_ServerList[type];
}

Server DataCenter::getServerMemCount(int type)
{
	return m_ServerList[type];
}

#ifdef _DEBUG
void DataCenter::showTotalFlavor()
{
	int nTotalFlavor = 0;
	for (int i = 0; i < m_SplitedFlavorList.size(); ++i)
	{
		for (int j = 0; j < m_SplitedFlavorList[i].size(); ++j)
		{
			nTotalFlavor += m_SplitedFlavorList[i][j].nCount;
		}
	}
	std::cout << "Total flavor count is:" << nTotalFlavor << std::endl;
}

#endif // ! _DEBUG

int DataCenter::getDayFromCurrentToFirstDay(std::string strTime)
{
	std::vector<std::string> sp_inputDay = Tools::split(strTime, "-");
	std::vector<std::string> sp_inputTime = Tools::split(sp_inputDay[2], " ");

	std::vector<std::string> sp_startTrainDay = Tools::split(m_startTrainTime, "-");
	std::vector<std::string> sp_startTrainTime = Tools::split(sp_startTrainDay[2], " ");

	return (std::stoi(sp_inputDay[1]) - std::stoi(sp_startTrainDay[1])) * 30 +
		   (std::stoi(sp_inputTime[0]) - std::stoi(sp_startTrainTime[0]));
}
