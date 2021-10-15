#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
namespace cv
{
    class Mat;
};
enum eBendingDataFaceInfo
{
    FRONT_BENDING = 0,
    BACK_BENDING = 1,
    UNCLASSIFEID_BENDING = 2,
    NUM_OF_BENDING_DATA
};
eBendingDataFaceInfo BuildFaceInfoFromSingleFilename(std::string name);
extern std::vector<std::string> gBendingDataFaceInfoStr;
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
    virtual cv::Mat GetPicture(bool draw_bezier = false) const;

    virtual int GetScale() const;
    virtual void SetScale(int scale);

protected:
    eBendingDataFaceInfo eFaceInfo;
    std::string mMatPath;
    std::string mImgPath;
    std::string mFaceInfo;
    double mWarpWeftAngle;
    int mScale;
    tVector2d mBezierA, mBezierB, mBezierC, mBezierD;
    double mUnitcm;
    void ImageProject2dMatlab(cv::Mat & img) const;
};

SIM_DECLARE_PTR(tBendingData);
typedef std::vector<tBendingDataPtr> tBendingDataList;
double GetWarpWeftAngleFromFilename(std::string path);