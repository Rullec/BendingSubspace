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
    SetScale(4);
    cMatlabFileUtil::CloseMatFile(mat);
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
    std::string img_path = cFileUtil::ConcatFilename(cFileUtil::GetDir(mMatPath), mImgPath);
    cv::Mat img = cOpencvUtil::LoadRGBImage(img_path);
    ImageProject2dMatlab(img);
    // std::cout << "raw img size = " << img.rows << " " << img.cols << std::endl;
    cv::resize(img, img, cv::Size(), 1.0 / mScale, 1.0 / mScale);
    // std::cout << "new img size = " << img.rows << " " << img.cols << std::endl;

    // begin to draw bezier (4 points)
    if (draw_bezier == true)
    {
        int height = img.rows;
        // Y axis need to be transformed
        // cv::Point A(mBezierA[0], height - mBezierA[1]);
        // cv::Point B(mBezierB[0], height - mBezierB[1]);
        // cv::Point C(mBezierC[0], height - mBezierC[1]);
        // cv::Point D(mBezierD[0], height - mBezierD[1]);
        cv::Point A(mBezierA[0], height - mBezierA[1]);
        cv::Point B(mBezierB[0], height - mBezierB[1]);
        cv::Point C(mBezierC[0], height - mBezierC[1]);
        cv::Point D(mBezierD[0], height - mBezierD[1]);
        // std::cout << "scale = " << mScale << std::endl;
        // std::cout << "raw A = " << mBezierA.transpose() << ", draw A = " << A << std::endl;
        // std::cout << "raw B = " << mBezierB.transpose() << ", draw B = " << B << std::endl;
        // std::cout << "raw C = " << mBezierC.transpose() << ", draw C = " << C << std::endl;
        // std::cout << "raw D = " << mBezierD.transpose() << ", draw D = " << D << std::endl;
        // std::cout << "mat file = " << this->mMatPath << std::endl;
        int num_of_div = 100;
        auto bezier = std::make_shared<cBezierCurvePhysics>(
            num_of_div,
            tVector2d(mBezierA[0], height - mBezierA[1]),
            tVector2d(mBezierB[0], height - mBezierB[1]),
            tVector2d(mBezierC[0], height - mBezierC[1]),
            tVector2d(mBezierD[0], height - mBezierD[1]), 1);
        tEigenArr<tVector2d> pt_lst = bezier->GetPointList();
        cv::circle(img, A, 7, cv::Scalar(255, 0, 0), CV_FILLED);
        cv::circle(img, B, 7, cv::Scalar(0, 255, 0), CV_FILLED);
        cv::circle(img, C, 7, cv::Scalar(0, 0, 255), CV_FILLED);
        cv::circle(img, D, 7, cv::Scalar(255, 255, 255), CV_FILLED);

        for (auto &pt : pt_lst)
        {
            cv::circle(img, cv::Point(pt[0], pt[1]), 3, cv::Scalar(100, 100, 100), CV_FILLED);
        }

        return bezier->GetTorqueCurvatureCurve();
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
        mBezierA = (mBezierA * mScale) / scale;
        // std::cout << "new A = " << mBezierA.transpose() << " new scale = " << scale << std::endl;
        mBezierB = (mBezierB * mScale) / scale;
        mBezierC = (mBezierC * mScale) / scale;
        mBezierD = (mBezierD * mScale) / scale;
        mUnitcm = (mUnitcm * mScale) / scale;
        mScale = scale;
    }
}