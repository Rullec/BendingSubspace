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
SIM_DECLARE_CLASS_AND_PTR(cBezierCurvePhysics);
SIM_DECLARE_CLASS_AND_PTR(cBezierCurve);
class tBendingData
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit tBendingData();
    virtual void Init(std::string mat_path, float rho_g_SI);
    virtual std::string GetMatPath() const { return mMatPath; }
    virtual std::string GetImgPath() const { return mImgPath; }
    virtual std::string GetFaceInfo() const { return mFaceInfo; }
    virtual double GetWarpWeftAngle() const { return mWarpWeftAngle; }
    virtual cv::Mat GetPicture(bool draw_bezier = false) const;

    virtual int GetScale() const;
    virtual void SetScale(int scale);
    virtual cBezierCurvePhysicsPtr GetBezierPhysic();
    virtual cBezierCurvePhysicsPtr GetBezierPhysicCutted();
    virtual double GetRhoG() const { return mRhoG; }
    virtual double GetUnitCM() const { return mUnitcm; }

protected:
    eBendingDataFaceInfo eFaceInfo;
    std::string mMatPath;
    std::string mImgPath;
    std::string mFaceInfo;
    double mWarpWeftAngle;
    int mScale;
    tVector2d mBezierARaw, mBezierBRaw, mBezierCRaw, mBezierDRaw;                                 // bezier points load from .mat, in 0.5 picture coords
    tVector2d mBezierATransformed, mBezierBTransformed, mBezierCTransformed, mBezierDTransformed; // bezier points fitted for opengl rendering coords
    tVector2d mBezierASI, mBezierBSI, mBezierCSI, mBezierDSI;
    double mUnitcm;
    cBezierCurvePtr mPixelBezier;                                // bezier curve in pixel coords
    cBezierCurvePhysicsPtr mPhysicsBezier;                       // bezier curve in SI format
    cBezierCurvePhysicsPtr mPhysicsBezierCuttedFromHighestPoint; // bezier curve in SI format, and cutted from highest
    double mRhoG;
    void ImageProject2dMatlab(cv::Mat &img) const;
    void InitBezierData();
};

SIM_DECLARE_PTR(tBendingData);
typedef std::vector<tBendingDataPtr> tBendingDataList;
double GetWarpWeftAngleFromFilename(std::string path);