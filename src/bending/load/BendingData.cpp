#include "BendingData.h"
#include "utils/FileUtil.h"
#include "utils/RegexUtil.h"
#include "utils/MatlabFileUtil.h"
#include <iostream>
std::vector<std::string> gBendingDataFaceInfoStr = {
    "front",
    "back",
    "unclassified"};
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
    mScale = 1;
    mPixelBezier = nullptr;
    mPhysicsBezier = nullptr;
}

void tBendingData::Init(std::string mat_path, float rho_g_SI)
{
    mRhoG = rho_g_SI;
    mMatPath = mat_path;
    // 1. load the mat, get the img path
    tMatFile *mat = cMatlabFileUtil::OpenMatFileReadOnly(mat_path);
    if (mat == nullptr)
    {
        std::cout << "init " << mMatPath << "failed\n";
        return;
    }
    mBezierARaw = cMatlabFileUtil::ParseAsVectorXd(mat, "A").segment(0, 2);
    mBezierBRaw = cMatlabFileUtil::ParseAsVectorXd(mat, "B").segment(0, 2);
    mBezierCRaw = cMatlabFileUtil::ParseAsVectorXd(mat, "C").segment(0, 2);
    mBezierDRaw = cMatlabFileUtil::ParseAsVectorXd(mat, "D").segment(0, 2);

    mUnitcm = cMatlabFileUtil::ParseAsDouble(mat, "unitCM");
    mImgPath = cMatlabFileUtil::ParseAsString(mat, "fileName");
    mFaceInfo = BuildFaceInfoFromSingleFilename(mMatPath);
    mWarpWeftAngle = GetWarpWeftAngleFromFilename(mat_path);
    SetScale(4);
    cMatlabFileUtil::CloseMatFile(mat);

    InitBezierData();
}

#include "utils/OpenCVUtil.h"

/**
 * \brief       trick method to make the image work with the bezier coords
*/
void tBendingData::ImageProject2dMatlab(cv::Mat &img) const
{
    cv::resize(img, img, cv::Size(), 2, 2);
}
#include "bezier/BezierCurvePhysics.h"
cv::Mat tBendingData::GetPicture(bool draw_bezier) const
{
    // img size *= 0.5
    cv::Mat img;
    {
        std::string img_path = cFileUtil::ConcatFilename(cFileUtil::GetDir(mMatPath), mImgPath);
        img = cOpencvUtil::LoadRGBImage(img_path);
        ImageProject2dMatlab(img);
        cv::resize(img, img, cv::Size(), 1.0 / mScale, 1.0 / mScale);
    }

    // begin to draw bezier (4 points)
    if (draw_bezier == true)
    {
        int height = img.rows;
        // Y axis need to be transformed
        cv::Point A(mBezierARaw[0], height - mBezierARaw[1]);
        cv::Point B(mBezierBRaw[0], height - mBezierBRaw[1]);
        cv::Point C(mBezierCRaw[0], height - mBezierCRaw[1]);
        cv::Point D(mBezierDRaw[0], height - mBezierDRaw[1]);
        tEigenArr<tVector2d> pt_lst = mPixelBezier->GetPointList();
        cv::circle(img, A, 7, cv::Scalar(255, 0, 0), CV_FILLED);
        cv::circle(img, B, 7, cv::Scalar(0, 255, 0), CV_FILLED);
        cv::circle(img, C, 7, cv::Scalar(0, 0, 255), CV_FILLED);
        cv::circle(img, D, 7, cv::Scalar(255, 255, 255), CV_FILLED);

        for (auto &pt : pt_lst)
        {
            cv::circle(img, cv::Point(pt[0], pt[1]), 2, cv::Scalar(150, 0, 24), CV_FILLED);
        }
    }
    return img;
}

int tBendingData::GetScale() const
{
    return this->mScale;
}

void tBendingData::SetScale(int scale)
{
    if (mScale == scale)
        return;
    else
    {
        // std::cout << "old A = " << mBezierA.transpose() << " old scale = " << mScale << std::endl;
        mBezierARaw = (mBezierARaw * mScale) / scale;
        // std::cout << "new A = " << mBezierA.transpose() << " new scale = " << scale << std::endl;
        mBezierBRaw = (mBezierBRaw * mScale) / scale;
        mBezierCRaw = (mBezierCRaw * mScale) / scale;
        mBezierDRaw = (mBezierDRaw * mScale) / scale;
        mUnitcm = (mUnitcm * mScale) / scale;
        mScale = scale;
    }
}

void tBendingData::InitBezierData()
{
    std::string img_path = cFileUtil::ConcatFilename(cFileUtil::GetDir(mMatPath), mImgPath);

    int raw_img_height = 1080;

    int transformed_img_height = 2.0 / mScale * raw_img_height;
    // cv::Mat img = cOpencvUtil::LoadRGBImage(img_path);
    // ImageProject2dMatlab(img);
    // cv::resize(img, img, cv::Size(), 1.0 / mScale, 1.0 / mScale);

    // transform the data points to opengl coordinates
    int height = transformed_img_height;
    // Y axis need to be transformed

    mBezierATransformed = tVector2d(mBezierARaw[0], height - mBezierARaw[1]);
    mBezierBTransformed = tVector2d(mBezierBRaw[0], height - mBezierBRaw[1]);
    mBezierCTransformed = tVector2d(mBezierCRaw[0], height - mBezierCRaw[1]);
    mBezierDTransformed = tVector2d(mBezierDRaw[0], height - mBezierDRaw[1]);

    // 1. create the bezier pixel
    // int num_of_div = 100;
    mPixelBezier = std::make_shared<cBezierCurve>(
        200, mBezierATransformed,
        mBezierBTransformed,
        mBezierCTransformed,
        mBezierDTransformed);

    // 2. create the SI physics image
    mBezierASI = (mBezierATransformed / mUnitcm) * 1e-2;
    mBezierBSI = (mBezierBTransformed / mUnitcm) * 1e-2;
    mBezierCSI = (mBezierCTransformed / mUnitcm) * 1e-2;
    mBezierDSI = (mBezierDTransformed / mUnitcm) * 1e-2;
    // move to the origin
    mBezierBSI -= mBezierASI;
    mBezierCSI -= mBezierASI;
    mBezierDSI -= mBezierASI;
    mBezierASI -= mBezierASI;
    // mirror
    mBezierASI *= -1;
    mBezierBSI *= -1;
    mBezierCSI *= -1;
    mBezierDSI *= -1;
    // construct the physics bezier
}

cBezierCurvePhysicsPtr tBendingData::GetBezierPhysic()
{
    if (mPhysicsBezier == nullptr)
    {
        // std::cout << "bezier A(SI) = " << mBezierASI.transpose() << std::endl;
        // std::cout << "bezier B(SI) = " << mBezierBSI.transpose() << std::endl;
        // std::cout << "bezier C(SI) = " << mBezierCSI.transpose() << std::endl;
        // std::cout << "bezier D(SI) = " << mBezierDSI.transpose() << std::endl;
        mPhysicsBezier = std::make_shared<cBezierCurvePhysics>(
            200,
            mBezierASI,
            mBezierBSI,
            mBezierCSI,
            mBezierDSI,
            mRhoG);
    }
    return mPhysicsBezier;
}

cBezierCurvePhysicsPtr tBendingData::GetBezierPhysicCuttedFromHighest()
{

    if (mPhysicsBezier_CuttedFromHighestPoint_ == nullptr)
    {
        mPhysicsBezier_CuttedFromHighestPoint_ = std::make_shared<cBezierCurvePhysics>(
            200,
            mBezierASI,
            mBezierBSI,
            mBezierCSI,
            mBezierDSI,
            mRhoG, true, false);
    }
    return mPhysicsBezier_CuttedFromHighestPoint_;
}

cBezierCurvePhysicsPtr tBendingData::GetBezierPhysic_CuttedFromHighest_ToZeroCurvature()
{

    if (mPhysicsBezier_CuttedFromHighestPoint_ToZeroCurvaturePoint == nullptr)
    {
        mPhysicsBezier_CuttedFromHighestPoint_ToZeroCurvaturePoint = std::make_shared<cBezierCurvePhysics>(
            200,
            mBezierASI,
            mBezierBSI,
            mBezierCSI,
            mBezierDSI,
            mRhoG, true, true);
    }
    return mPhysicsBezier_CuttedFromHighestPoint_ToZeroCurvaturePoint;
}