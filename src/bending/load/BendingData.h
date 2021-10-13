#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"

class tBendingData
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit tBendingData();
    // void Init(std::string img_path, std::string mat_path);
    // std::string mImagePath;
    std::string mMatPath;

protected:
};

SIM_DECLARE_PTR(tBendingData);
typedef std::vector<tBendingDataPtr> tBendingDataList;

class tBendingDataCloth
{
public:
    tBendingDataCloth();

    int mId;
    std::string mDataDir;
    tBendingDataList mFrontData, mBackData;
};

SIM_DECLARE_PTR(tBendingDataCloth);
int LoadClothData(std::string data_dir,
                  std::vector<std::string> &img_list,
                  std::vector<std::string> &mat_list);