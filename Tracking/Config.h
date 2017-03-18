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

#include <vector>
#include <string>
#include <ostream>

#define VERBOSE (0)

class Config
{
public:
	//Config() { SetDefaults(); }
	Config();
	//选择提取特征的方式  Haar特征，Raw特征，Histogram特征  
	enum FeatureType
	{
		kFeatureTypeHaar,
		kFeatureTypeRaw,
		kFeatureTypeHistogram
	};
	//SVM四种核函数  
	enum KernelType
	{
		kKernelTypeLinear,
		kKernelTypeGaussian,
		kKernelTypeIntersection,
		kKernelTypeChi2
	};

	struct FeatureKernelPair
	{
		FeatureType feature;
		KernelType kernel;
		std::vector<double> params;
	};
	
	bool							quietMode;
	bool							debugMode;
	
	std::string						sequenceBasePath;  //图像序列路径
	std::string						sequenceName;	   //图像文件夹名称
	std::string						resultsPath;
	
	int								frameWidth;
	int								frameHeight;
	
	int								seed;
	int								searchRadius;	   //搜索半径
	double							svmC;			   //正则化系数
	int								svmBudgetSize;	   //支持向量数量
	std::vector<FeatureKernelPair>	features;
	
	friend std::ostream& operator<< (std::ostream& out, const Config& conf);
	
private:
	void SetDefaults();
	static std::string FeatureName(FeatureType f);
	static std::string KernelName(KernelType k);
};

