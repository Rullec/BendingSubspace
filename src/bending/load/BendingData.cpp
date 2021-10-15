#include "BendingData.h"
#include "utils/FileUtil.h"
#include "utils/RegexUtil.h"
#include "utils/MatlabFileUtil.h"
#include <iostream>

double GetWarpWeftAngleFromFilename(std::string path)
{
    std::string name = cFileUtil::GetFilename(path);

    int st = name.find("-") + 1;
    int ed = name.find(".mat");
    auto sub = name.substr(st, ed - st + 1);
    if (sub.size() == 0)
    {
        SIM_WARN("failed to get mat id from {}", path);
        return -1;
    }
    double idx = std::stof(sub.c_str());
    // printf("name %s float %f\n", name.c_str(),
    //        idx);
    return idx;
}

eBendingDataFaceInfo BuildFaceInfoFromSingleFilename(std::string name_)
{
    std::string name = cFileUtil::ConvertToLowerCase(cFileUtil::GetFilename(name_));
    if (-1 != name.find("front"))
    {
        return eBendingDataFaceInfo::FRONT_BENDING;
    }
    else if (-1 != name.find("back"))
    {
        return eBendingDataFaceInfo::BACK_BENDING;
    }
}
tBendingData::tBendingData()
{
}

void tBendingData::Init(std::string mat_path)
{
    mMatPath = mat_path;
    // 1. load the mat, get the img path
    tMatFile *mat = cMatlabFileUtil::OpenMatFileReadOnly(mat_path);
    if (mat == nullptr)
    {
        std::cout << "init " << mMatPath << "failed\n";
        return;
    }
    mBezierA = cMatlabFileUtil::ParseAsVectorXd(mat, "A").segment(0, 2);
    mBezierB = cMatlabFileUtil::ParseAsVectorXd(mat, "B").segment(0, 2);
    mBezierC = cMatlabFileUtil::ParseAsVectorXd(mat, "C").segment(0, 2);
    mBezierD = cMatlabFileUtil::ParseAsVectorXd(mat, "D").segment(0, 2);

    mUnitcm = cMatlabFileUtil::ParseAsDouble(mat, "unitCM");
    mImgPath = cMatlabFileUtil::ParseAsString(mat, "fileName");
    mFaceInfo = BuildFaceInfoFromSingleFilename(mMatPath);
    mWarpWeftAngle = GetWarpWeftAngleFromFilename(mat_path);
    cMatlabFileUtil::CloseMatFile(mat);
}
