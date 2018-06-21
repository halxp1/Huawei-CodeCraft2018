#include "predict.h"
#include "iostream"
#include "define.h"
#include "DataCenter.h"
#include "PredictAlgorithm.h"
#include "DeployAlgorithm.h"

extern void write_result(const char *const buff, const char *const filename);

//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM], int data_num, char *filename)
{
	/*- ------------------------------------------------------------------------------ -/
	 *- 根据比赛要求，设计如下几个类(隐藏实现细节，方便根据需求随意切换算法）          -*
	 *-                                                                                -*
	 *- 类的功能如下：                                                                 -*
	 *-                                                                                -*
	 *- DataCenter类(数据中心类)完成以下功能:                                          -*
	 *-             1.读取数据                                                         -*
	 *-             2.分割数据                                                         -*
	 *-             3.为其它类提供接口                                                 -*
	 *-                                                                                -*
	 *- PredictAlgorithm类(预测算法类)完成以下功能:                                    -*
	 *-             1.根据数据中心的分割结果执行预测                                   -*
	 *-             2.误差校准                                                         -*
	 *-             备注:PredictAlgorithm是一个算法仓库类，将曾经用过的                -*
	 *-                  预测算法汇总到此类中，实现灵活且方便切换预测算法。            -*
	 *-                                                                                -*
	 *- DeployAlgorithm类(部署算法类)完成以下功能：                                    -*
	 *-             1.根据预测算法类的预测结果，将所有预测出来需要的虚拟机部署到服务器 -*
	 *-             备注：DeployAlgorithm是一个部署算法仓库类，将曾经用过的            -*
	 *-                   部署算法汇总到此类中，实现灵活且方便切换预测算法。           -*
	 *-                   根据比赛要求的优化，可自动选择部署算法，获得部署算法类       -*
	 *-                   中最优的结果返回。                                           -*
	 *- ------------------------------------------------------------------------------ -*/

	std::string result;
	//声明数据中心对象，同时设置必要的参数
	DataCenter dc(info, data, data_num, DataCenter::SLACK_TIME_QUANTUM); //松弛 7天
	//数据中心开始处理输入数据
	if (dc.start())
	{
		//声明预测算法对象
		PredictAlgorithm pa;
		//设置预测算法对象必要的参数
		pa.setDataCenterObject(dc);
		pa.setPredictAlgorithm(PredictAlgorithm::AVERAGE);  //二次指数平滑
		pa.setWeightModel(PredictAlgorithm::AUTO_WEIGHT); //指数平滑自动调参
		//pa.setWeight(0.57, 0.4, 0.5);
		//pa.setPredictMethod(PredictAlgorithm::DAY); FIXED_ERRORS
		//pa.setErrorsModel(PredictAlgorithm::DYNAMIC_ERRORS);
		pa.setErrorsModel(PredictAlgorithm::FIXED_ERRORS);  //设定误差方式
		pa.setErrorsFixed(60);
		//开始预测
		pa.startPredict();
		//获取预测结果

		//声明部署算法对象，同时设置必要的参数
		DeployAlgorithm da(pa, dc, DeployAlgorithm::GREEDY_ALGORITHM);
		//开始部署
		da.startDeploy(); 
		//获取部署结果
		result += da.getDeployResultStr();

		std::cout << result << std::endl;
	}
	const char *result_file = result.c_str();
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(result_file, filename);
}
