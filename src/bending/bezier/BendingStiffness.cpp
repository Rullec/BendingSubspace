#include "BendingStiffness.h"
#include <iostream>
const tVectorXf &GetMlist(const tMKCurve &curve)
{
    return curve.first;
}
const tVectorXf &GetKlist(const tMKCurve &curve)
{
    return curve.second;
}

tBendingStiffness::tBendingStiffness(
    const tMKCurve &raw_curve_) : mRawMKCurve(raw_curve_)
{

    mNonLinearBendingStiffness_a = 0, mNonLinearBendingStiffness_b = 0;
    const tVectorXf &K_lst = GetKlist(mRawMKCurve);
    mKmin = K_lst.minCoeff();
    mKmax = K_lst.maxCoeff();

    // 1. calculate bending stiffness
    mLinearBendingStiffness = CreateLinearBendingStiffness();
    CreateNonLinearBendingStiffness(mNonLinearBendingStiffness_a, mNonLinearBendingStiffness_b);

    // 2. calculate the fitted curve
    CreateFittedList_Linear(mFitMKCurve_linear);
    CreateFittedList_Nonlinear(mFitMKCurve_nonlinear);
}

/**
 * \brief           return the linear bending stiffness, estimated from given data
*/
float tBendingStiffness::GetLinearBendingStiffness()
{
    return this->mLinearBendingStiffness;
}

/**
 * \brief           return the nonlinear bending stiffness, estimated from given data
*/
void tBendingStiffness::GetNonLinearBendingStiffness(float &a, float &b)
{
    a = mNonLinearBendingStiffness_a;
    b = mNonLinearBendingStiffness_b;
}

/**
 * \brief           return the fitted MKCurve by linear estimated bending stiffness
*/
const tMKCurve &tBendingStiffness::GetLinear_FittedMKList() const
{
    return this->mFitMKCurve_linear;
}

/**
 * \brief           return the fitted MKCurve by nonlinear estimated bending stiffness
*/
const tMKCurve &
tBendingStiffness::GetNonLinear_FittedMKList() const
{
    return this->mFitMKCurve_nonlinear;
}

/**
 * \brief           calculate linear bending stiffness by least square
 * 
 *          coef = (MTK)/(KTK)
*/
float tBendingStiffness::CreateLinearBendingStiffness() const

{
    tVectorXf M_lst = GetMlist(this->mRawMKCurve);
    tVectorXf K_lst = GetKlist(this->mRawMKCurve);
    float k = M_lst.dot(K_lst) / (K_lst.dot(
                                     K_lst));
    // double thre = 5e-8;
    // if (k < thre)
    // {
    //     std::cout << "[warn] linear k = " << k << "was revised to " << thre << std::endl;
    //     k = thre;
    // }
    return k;
}

void tBendingStiffness::CreateNonLinearBendingStiffness(float &k1, float &k2) const
{
    tVectorXd K_bar = GetKlist(mRawMKCurve).cast<double>();
    tVectorXd K_bar2 = K_bar.array().pow(2).cast<double>();
    tVectorXd M_bar = GetMlist(mRawMKCurve).cast<double>();

    tMatrix2d A = tMatrix2d::Zero();
    A(0, 0) = K_bar2.transpose() * K_bar2;
    A(0, 1) = K_bar2.transpose() * K_bar;
    A(1, 0) = K_bar2.transpose() * K_bar;
    A(1, 1) = K_bar.transpose() * K_bar;
    tVector2d b = tVector2d(K_bar2.transpose() * M_bar, K_bar.transpose() * M_bar);
    tVector2d x = A.inverse() * b;
    k1 = x[0];
    k2 = x[1];
    // std::cout << "A = \n"
    //           << A << std::endl;
    // std::cout << "b = "
    //           << b.transpose() << std::endl;
    std::cout << "[solved] x = " << (A.inverse() * b).transpose() << std::endl;
}
void tBendingStiffness::CreateFittedList_Linear(tMKCurve &curve) const
{
    int num_of_div = 200;
    tVectorXf Klst = tVectorXf::LinSpaced(num_of_div, mKmin, mKmax);
    tVectorXf Mlst = Klst * mLinearBendingStiffness;
    SetMlist(curve, Mlst);
    SetKlist(curve, Klst);
}
void tBendingStiffness::CreateFittedList_Nonlinear(tMKCurve &curve) const
{
    int num_of_div = 200;
    tVectorXf Klst = tVectorXf::LinSpaced(num_of_div, mKmin, mKmax);

    tVectorXf K2lst = Klst.array().pow(2);
    tVectorXf Mlst = mNonLinearBendingStiffness_a * K2lst + mNonLinearBendingStiffness_b * Klst;
    SetMlist(curve, Mlst);
    SetKlist(curve, Klst);
}