#include "BezierCurvePhysics.h"

cBezierCurvePhysics::cBezierCurvePhysics(int num_of_div, const tVector2d &A,
                                         const tVector2d &B, const tVector2d &C,
                                         const tVector2d &D, double rho_g)
    :

      cBezierCurve(num_of_div, A, B, C, D), mRhoG(rho_g)
{
    mTorqueList = CalculateTorque();
}
tVectorXd cBezierCurvePhysics::GetTorqueList() const
{
    return mTorqueList;
}
// #include "matplot/matplot.h"
#include "matplot\\matplot.h"
#include "utils/FileUtil.h"
#include "utils/TimeUtil.hpp"
cv::Mat cBezierCurvePhysics::GetTorqueCurvatureCurve() const
{
    // rendered by result
    // mTorqueList: num_of_seg
    // mK: num_of_div
    int num_of_seg = mNumOfDiv - 1;

    std::vector<double> x(num_of_seg), y(num_of_seg);
    for (int i = 0; i < num_of_seg; i++)
    {
        x[i] = mCurvatureList[i];
        y[i] = mTorqueList[i];
    }
    // memcpy(x.data(), mCurvatureList.data(), sizeof(double) * num_of_seg);
    // memcpy(y.data(), mTorqueList.data(), sizeof(double) * num_of_seg);
    std::string save_img = "tmp.png";
    matplot::plot(x, y);
    matplot::save(save_img);
    Sleep(30);
    // matplot::wait();

    // exit(0);
    cv::Mat img = cOpencvUtil::LoadRGBImage(save_img);
    // cFileUtil::RemoveFile(save_img);
    // matplot::show();
    // exit(1);
    return img;
}

tVectorXd cBezierCurvePhysics::CalculateTorque() const
{
    double sum_mass_rhog_from_x_to_end = 0,
           sum_mass_rhog_prod_x_from_x_to_end = 0;
    std::vector<double> torque_lst = {0};
    int num_of_seg = mNumOfDiv - 1;
    for (int i = num_of_seg - 1; i >= 0; i--)
    {
        sum_mass_rhog_from_x_to_end += mArclengthList[i] * mRhoG;
        sum_mass_rhog_prod_x_from_x_to_end += mArclengthList[i] * mRhoG * (mPosX[i] + mPosX[i + 1]) / 2;
        double COM = sum_mass_rhog_prod_x_from_x_to_end / sum_mass_rhog_from_x_to_end;
        double force_arm = COM - mPosX[i];
        double force = sum_mass_rhog_from_x_to_end;
        double torque = force_arm * force;
        torque_lst.push_back(torque);
    }
    std::reverse(torque_lst.begin(), torque_lst.end());
    tVectorXd ret(torque_lst.size());
    memcpy(ret.data(), torque_lst.data(), sizeof(double) * ret.size());
    // std::cout << "rhog = " << mRhoG << std::endl;
    // std::cout << "torque list = " << ret.transpose() << std::endl;
    // exit(1);
    return ret;
}