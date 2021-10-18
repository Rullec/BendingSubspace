#include "BezierCurvePhysics.h"

cBezierCurvePhysics::cBezierCurvePhysics(int num_of_div, const tVector2d &A,
                                         const tVector2d &B, const tVector2d &C,
                                         const tVector2d &D, double rho_g, bool cutted_from_highest_point /*= false*/,
                                         bool cutted_to_zero_curvature_point /* = false*/
                                         )
    :

      cBezierCurve(num_of_div, A, B, C, D), mRhoG(rho_g)
{
    mTorqueList = CalculateTorque();
    if (cutted_from_highest_point == true)
    {
        CutFromHighestPoint();
    }
    if (cutted_to_zero_curvature_point)
    {
        CutToNegativeCurvaturePoint();
    }
    SetMlist(mTorqueCurvatureCurve, mTorqueList);
    SetKlist(mTorqueCurvatureCurve, mCurvatureList);
}
tVectorXd cBezierCurvePhysics::GetTorqueList() const
{
    return mTorqueList;
}
#include "utils/FileUtil.h"
#include "utils/TimeUtil.hpp"
tMKCurve cBezierCurvePhysics::GetTorqueCurvatureCurve() const
{
    return mTorqueCurvatureCurve;
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

void cBezierCurvePhysics::CutFromHighestPoint()
{
    // 1. get the highest point
    double max_value = -1e10;
    int max_value_idx = -1;
    for (int i = 0; i < mPosY.size(); i++)
    {
        if (mPosY[i] > max_value)
        {
            max_value = mPosY[i];
            max_value_idx = i;
        }
    }
    int cutted_from_idx = int(mNumOfDiv - 0.9 * (mNumOfDiv - max_value_idx));
    printf("highest idx %d, cutted idx %d\n", max_value_idx, cutted_from_idx);
    // std::cout << "Y = " << mPosY.transpose() << std::endl;

    // cutted
    int raw_size = mNumOfDiv;
    mNumOfDiv = raw_size - cutted_from_idx;
    mPosY = mPosY.segment(cutted_from_idx, mNumOfDiv).eval(); // N
    mPosX = mPosX.segment(cutted_from_idx, mNumOfDiv).eval(); // N
    for (int i = mNumOfDiv - 1; i >= 0; i--)
    {
        mPosX[i] -= mPosX[0];
        mPosY[i] -= mPosY[0];
    }
    mArclengthList = mArclengthList.segment(cutted_from_idx, mNumOfDiv - 1).eval(); // N - 1
    for (int i = mArclengthList.size() - 1; i >= 0; i--)
    {
        mArclengthList[i] -= mArclengthList[0];
    }
    mCurvatureList = mCurvatureList.segment(cutted_from_idx, mNumOfDiv).eval(); // N
    mU = mU.segment(cutted_from_idx, mNumOfDiv).eval();                         // N
    mOneMinusU = mOneMinusU.segment(cutted_from_idx, mNumOfDiv).eval();         // N
    mTorqueList = mTorqueList.segment(cutted_from_idx, mNumOfDiv).eval();       // N
    // cut the curvature list and
}

void cBezierCurvePhysics::CutToNegativeCurvaturePoint()
{
    int cutted_to_idx = mNumOfDiv - 1;
    for (int idx = 0; idx < mNumOfDiv; idx++)
    {
        if (mTorqueList[idx] < 0)
        {
            cutted_to_idx = idx;
            break;
        }
    }
    int raw_size = mNumOfDiv;
    mNumOfDiv = cutted_to_idx + 1;
    mPosY = mPosY.segment(0, mNumOfDiv).eval();                       // N
    mPosX = mPosX.segment(0, mNumOfDiv).eval();                       // N
    mArclengthList = mArclengthList.segment(0, mNumOfDiv - 1).eval(); // N - 1
    mCurvatureList = mCurvatureList.segment(0, mNumOfDiv).eval();     // N
    mU = mU.segment(0, mNumOfDiv).eval();                             // N
    mOneMinusU = mOneMinusU.segment(0, mNumOfDiv).eval();             // N
    mTorqueList = mTorqueList.segment(0, mNumOfDiv).eval();
}

/**
 * \brief               Get Estimated bending stiffness
*/
tBendingStiffnessPtr cBezierCurvePhysics::GetEstimatedBendingStiffness()
{
    if (mBendingStiffness == nullptr)
    {
        tMKCurve curve;
        SetMlist(curve, this->mTorqueList);
        SetKlist(curve, this->mCurvatureList);
        mBendingStiffness = std::make_shared<tBendingStiffness>(curve);
    }
    return mBendingStiffness;
}

/**
 * \brief               Get rho * G
*/
double cBezierCurvePhysics::GetRhoG() const
{
    return this->mRhoG;
}