/* 
 * Struck: Structured Output Tracking with Kernels
 * 
 * Code to accompany the paper:
 *   Struck: Structured Output Tracking with Kernels
 *   Sam Hare, Amir Saffari, Philip H. S. Torr
 *   International Conference on Computer Vision (ICCV), 2011
 * 
 * Copyright (C) 2011 Sam Hare, Oxford Brookes University, Oxford, UK
 * 
 * This file is part of Struck.
 * 
 * Struck is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Struck is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Struck.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#pragma once

#include "Rect.h"
#include "Sample.h"

#include <vector>
#include <Eigen/Core>

#include <opencv/cv.h>

class Config;
class Features;
class Kernel;

class LaRank
{
public:
	LaRank(const Config& conf, const Features& features, const Kernel& kernel); //初始化参数 特征 核
	~LaRank();
	
	virtual void Eval(const MultiSample& x, std::vector<double>& results, std::vector<double>& p);
	virtual void Update(const MultiSample& x, int y);
	
	virtual void Debug();
	struct DisGussian
	{
		Eigen::VectorXd e;
		Eigen::VectorXd var;
	};
	DisGussian DisGuss;
	void computeDisGuss();

	

private:

	struct SupportPattern
	{
		std::vector<Eigen::VectorXd> x;
		std::vector<FloatRect> yv;       //目标变化关系
		std::vector<cv::Mat> images;     
		int y;           //索引值表明指定样本存放位置
		int refCount;        //统计sp的个数
	};

	struct SupportVector
	{
		SupportPattern* x;
		int y;
		double b;     //beta  
		double g;     //gradient  
		cv::Mat image;
	};
	
	const Config& m_config;
	const Features& m_features;
	const Kernel& m_kernel;
	
	std::vector<SupportPattern*> m_sps;
	std::vector<SupportVector*> m_svs;

	cv::Mat m_debugImage;
	
	double m_C;     //目标函数的系数C 和矩阵K
	Eigen::MatrixXd m_K;

	inline double Loss(const FloatRect& y1, const FloatRect& y2) const           //损失函数
	{
		// overlap loss
		return 1.0-y1.Overlap(y2);
		// squared distance loss
		//double dx = y1.XMin()-y2.XMin();
		//double dy = y1.YMin()-y2.YMin();
		//return dx*dx+dy*dy;
	}
	
	double ComputeDual() const;

	void SMOStep(int ipos, int ineg);
	std::pair<int, double> MinGradient(int ind);
	void ProcessNew(int ind);
	void Reprocess();
	void ProcessOld();
	void Optimize();

	int AddSupportVector(SupportPattern* x, int y, double g);
	void RemoveSupportVector(int ind);
	void RemoveSupportVectors(int ind1, int ind2);
	void SwapSupportVectors(int ind1, int ind2);
	
	void BudgetMaintenance();
	void BudgetMaintenanceRemove();

	double Evaluate(const Eigen::VectorXd& x, const FloatRect& y) const;
	void UpdateDebugImage();
};
