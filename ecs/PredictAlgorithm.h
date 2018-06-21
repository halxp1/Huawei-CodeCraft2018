#ifndef PREDICT_ALGORITHM_H_
#define PREDICT_ALGORITHM_H_
#include "define.h"

class DataCenter;

class PredictAlgorithm
{
public:
	friend class DeployAlgorithm;
public:
	enum {//Algorithm
		SINGLE_EXPONENTIAL_SMOOTHING, //一次指数平滑
		SECOND_EXPONENTIAL_SMOOTHING, //二次指数平滑
		THREE_EXPONENTIAL_SMOOTHING,  //三次指数平滑
		LINEAR_REGRESSION,            //线性回归
		AVERAGE                       //平均
	};
	enum {//误差调节方式
		FIXED_ERRORS,                 //固定误差
		DYNAMIC_ERRORS,               //动态误差
	};
	enum {//预测方式
		DAY,                          //按天预测, 若要按天预测，数据分割必须也按天分割
		TIME_QUANTUM                  //按时间周期预测
	};
	enum {//是否开启自动调参
		AUTO_WEIGHT,
		MANUAL_WEIGHT
	};

	PredictAlgorithm();
	~PredictAlgorithm();
	void setDataCenterObject(DataCenter & dc);
	//设置预测算法 SINGLE_EXPONENTIAL_SMOOTHING/SECOND_EXPONENTIAL_SMOOTHING/THREE_EXPONENTIAL_SMOOTHING
	void setPredictAlgorithm(int predict_model = SECOND_EXPONENTIAL_SMOOTHING);
	void startPredict();
	const std::string & getError();//返回错误信息
	
	/*********************************指数平滑算法***********************************/
	//设置平滑权重
	void setWeight(double weight_s1 = 0.57, double weight_s2 = 0.4, double weight_s3 = 0.0);
	
	void setWeightModel(int weightModel = AUTO_WEIGHT);
	//设置固定误差
	void setErrorsFixed(int errorsFixed = 20);

	//设置误差调节权重
	void setErrorsWeight(double errorsWeight = 0.8);
	/*********************************指数平滑算法***********************************/

	//误差调节模式 FIXED_ERRORS or DYNAMIC_ERRORS
	void setErrorsModel(int errorsModel = FIXED_ERRORS);

	//设置预测方法 DAY or TIME_QUANTUM
	void setPredictMethod(int predictMethod);

	int getPredictAlgorithm();

	//获取最终预测结果字符串
	std::string getResultStr();

private:

	//首先指数不需要预测部分的指数平滑值
	void calcCommonExponentialSmoothing();

	//首先指数不需要预测部分的指数平滑值(自动调参)
	void calcCommonExponentialSmoothing_AUTO_WEIGHT(std::vector<Flavor> & flBuf, int predictMethod, float weight);


	void calcCommonExponentialSmoothing_AUTO_WEIGHT();

	//计算公共部分S1
	void calcCommonS1();

	void calcWeight(int predictAlgorithm);


#ifdef _DEBUG
	void showPredictResult_s1();
#endif

private:
	//预测
	void predict_SINGLE_EXPONENTIAL_SMOOTHING();
	void predict_SECOND_EXPONENTIAL_SMOOTHING();
	void predict_THREE_EXPONENTIAL_SMOOTHING();

	//自动调参
	void predict_SINGLE_EXPONENTIAL_SMOOTHING_AUTO_WEIGHT();
	void predict_SECOND_EXPONENTIAL_SMOOTHING_AUTO_WEIGHT();
	void predict_THREE_EXPONENTIAL_SMOOTHING_AUTO_WEIGHT();
	void predict_AVERAGE();

	void predict_LINEAR_REGRESSION();
	//线性回归子函数

	//计算梯度
	float calcGradient(const std::vector<std::vector<float>> & trainData, const std::vector<float> & feature, int nAlpha, int nDimensions);

	//计算预测函数
	float calcPredictFunction(const std::vector<float> & cur_trainData, const std::vector<float> & feature, int nDimensions);

	//正则化 梯度
	void normalize(std::vector<float> & feature, int nDimensions);

	//更新 alpha
	float updateAlpha(const std::vector<std::vector<float>> & trainData, const std::vector<float> & feature, std::vector<float>  & featureGradient, float nAlpha, int nDimensions);

	bool isSame(std::vector<float> &feature, std::vector<float> &nextFeature, int nDimensions);
	
	float dotProduct(std::vector<float> & featureGradient, std::vector<float> & featureGradient_1, int nDimensions);
	
	float lossFunction(const std::vector<float> &feature,const  std::vector<std::vector<float>> &trainData, int nDimensions);
	//线性回归子函数

	//误差校准
	void errorCalibration();

	//统计预测结果
	void statisticalPredictResults();

	//返回统计结果字符串，部署的时候会用到
	const std::vector<Flavor> & getPredictResultVector();

	void cutPredictHeader();
	void cutPredictTail();
private:
	DataCenter * m_dc;
	int m_predictAlgorithm;//预测算法
	std::string m_error;//错误信息
	double m_weight_s1;//一次平滑权重
	double m_weight_s2;//二次平滑权重
	double m_weight_s3;//三次平滑权重
	int m_errorsFixed; //固定误差值
	double m_errorsWeight;//动态误差权重
	int m_errorsModel;//误差调节方式
	int m_predictMethod;//预测方法

	int m_weightModel;//是否开启自动调参

	int m_TimeSpan;//预测开始时间与训练数据结束时间之间的跨度

	std::vector<std::vector<Flavor>> m_PredictedFlavor;//预测所得的规格列表以及每个规格的数量
	
	//预测结果汇总
	std::vector<Flavor> m_PredictResult;
	int nTotalPredictCount;//预测出来的虚拟机总数

	//自动调参相关
	std::vector<float> m_alphaList_s1;//一次指数平滑参数列表
	std::vector<float> m_alphaList_s2;//二次指数平滑参数列表
	std::vector<float> m_alphaList_s3;//二次指数平滑参数列表

	std::vector<float> m_errorsList_s1;//误差列表
	std::vector<float> m_errorsList_s2;//误差列表
	std::vector<float> m_errorsList_s3;//误差列表

	std::vector<double> flavorProportionList;

};

#endif
