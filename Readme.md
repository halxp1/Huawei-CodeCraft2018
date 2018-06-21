# 背景介绍
云平台为了满足不同租户的需求，提供了一种可随时自助获取、可弹性伸缩的云服务器，即弹性云服务器（Elastic Cloud Server，ECS）。为容纳更多的租户请求、并尽可能提高资源利用率、降低成本，自动化、智能化的资源调度管理系统非常关键。

由于租户对ECS实例（虚拟机，VM）请求的行为具有一定规律，可以通过对历史ECS实例请求的分析，预测到未来一段时间的ECS实例请求，然后对预测的请求分配资源（如下图所示），这样可以找到一个接近最优的分配策略，实现资源最大化利用，同时也能参考预测的结果制定云数据中心的建设计划。

  ![](https://github.com/chaoliuzihan/Huawei-CodeCraft2018/raw/master/image/1-1.png)

# 总体方案

