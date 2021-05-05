// // +build ignore

// //批量梯度BGD（batch gradient descent）

// #include <Eigen/Core>

// // 稠密矩阵的代数运算（逆，特征值等）
// #include <Eigen/Dense>
// #include <string>
// #include <fstream>
// #include <vector>
// #include <regex>
// #include <algorithm>

// #include "util.hpp"

// namespace supervised
// {

// 	// linearregression_bgd batch gradient descent for linear regression
// 	// input:
// 	// X:  one row represent one sample
//     // numEpochs： epochs amount
// 	// output :
// 	// otheta: result theta
// 	// if fail, return -1, otherwise 0. the result theta return by otheta
// 	int linearregression_bgd(const Eigen::MatrixXd &X,
// 			const Eigen::VectorXd &y, const Eigen::VectorXd &init_theta,
// 			double alpha, std::size_t numEpochs, std::vector<double> &otheta)
// 	{

// 		auto m = X.rows();
// 		if (X.cols() != init_theta.rows() || m != y.rows())
// 		{
// 			return -1;
// 		}

// 		auto pars_len = init_theta.rows();
// 		auto theta = init_theta;
// 		//
// 		for (size_t i = 0; i < numEpochs; i++)
// 		{
// 			Eigen::VectorXd _error = X * theta - y;

// 			auto temp = theta;
// 			for (int j = 0; j < pars_len; j++)
// 			{
// 				auto t = (alpha * _error.dot(X.col(j))) / m;
// 				temp(j) = theta(j) - t;
// 			}

// 			theta = temp;
// 			// cost[i] = squareloss(X, y, theta)
// 		}

// 		otheta.clear();
// 		std::copy(theta.data(), theta.data() + theta.size(), std::back_inserter(otheta));

// 		return 0;
// 	}

// }; // namespace supervised

