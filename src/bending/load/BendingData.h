#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"

enum eBendingDataFaceInfo
{
    FRONT_BENDING = 0,
    BACK_BENDING = 1,
    UNCLASSIFEID_BENDING = 2
};
eBendingDataFaceInfo BuildFaceInfoFromSingleFilename(std::string name);
class tBendingData
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit tBendingData();
    virtual void Init(std::string mat_path);
    virtual std::string GetMatPath() const { return mMatPath; }
    virtual std::string GetImgPath() const { return mImgPath; }
    virtual std::string GetFaceInfo() const { return mFaceInfo; }
    virtual double GetWarpWeftAngle() const { return mWarpWeftAngle; }

protected:
    eBendingDataFaceInfo eFaceInfo;
    std::string mMatPath;
    std::string mImgPath;
    std::string mFaceInfo;
    double mWarpWeftAngle;
    tVector2d mBezierA, mBezierB, mBezierC, mBezierD;
    double mUnitcm;
};

SIM_DECLARE_PTR(tBendingData);
typedef std::vector<tBendingDataPtr> tBendingDataList;
double GetWarpWeftAngleFromFilename(std::string path);