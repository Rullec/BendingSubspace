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
    CreateNonLinearBendingStiffness(mNonLinearBendingStiffness_a, mNonLinearBendingStiffness_b, mNonLinearBendingStiffness_c);

    // 2. calculate the fitted curve
    CreateFittedList_Linear(mFitMKCurve_linear);
    CreateFittedList_Nonlinear(mFitMKCurve_nonlinear);
    CreateFittedList_Nonlinear_withconstant(mFitMKCurve_nonlinear_withconstant);
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

void tBendingStiffness::GetNonLinearBendingStiffnessWithConstant(float &a, float &b, float &c)
{
    GetNonLinearBendingStiffness(a, b);
    c = this->mNonLinearBendingStiffness_c;
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

/**
 * \brief       M = k1 * K^2  + k2 * K + constant
*/
void tBendingStiffness::CreateNonLinearBendingStiffness(float &k1, float &k2, float &constant) const
{
    // homogenous, no constant term
    {
        // tVectorXd K_bar = GetKlist(mRawMKCurve).cast<double>();
        // tVectorXd K_bar2 = K_bar.array().pow(2).cast<double>();
        // tVectorXd M_bar = GetMlist(mRawMKCurve).cast<double>();

        // tMatrix3d A = tMatrix3d::Zero();
        // A(0, 0) = K_bar2.transpose() * K_bar2;
        // A(0, 1) = K_bar2.transpose() * K_bar;
        // A(1, 0) = K_bar2.transpose() * K_bar;
        // A(1, 1) = K_bar.transpose() * K_bar;
        // tVector2d b = tVector2d(K_bar2.transpose() * M_bar, K_bar.transpose() * M_bar);
        // tVector2d x = A.inverse() * b;
        // k1 = x[0];
        // k2 = x[1];
    }
    // add the constant term
    {
        tVectorXd K_bar = GetKlist(mRawMKCurve).cast<double>();
        tVectorXd K_bar2 = K_bar.array().pow(2).cast<double>();
        tVectorXd M_bar = GetMlist(mRawMKCurve).cast<double>();

        tMatrix3d A = tMatrix3d::Zero();
        A(0, 0) = K_bar2.transpose() * K_bar2;
        A(0, 1) = K_bar2.transpose() * K_bar;
        A(0, 2) = K_bar2.sum();
        A(1, 0) = K_bar2.transpose() * K_bar;
        A(1, 1) = K_bar.transpose() * K_bar;
        A(1, 2) = K_bar.sum();
        A(2, 0) = K_bar2.sum();
        A(2, 1) = K_bar.sum();
        A(2, 2) = 1 * K_bar.size();

        tVector3d b = tVector3d(K_bar2.transpose() * M_bar, K_bar.transpose() * M_bar, M_bar.sum());
        tVector3d x = A.inverse() * b;
        k1 = x[0];
        k2 = x[1];
        constant = x[2];
        std::cout << "[solved] x = " << (A.inverse() * b).transpose() << std::endl;
    }
    // std::cout << "A = \n"
    //           << A << std::endl;
    // std::cout << "b = "
    //           << b.transpose() << std::endl;
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

const tMKCurve &tBendingStiffness::GetNonLinear_FittedMKList_withconstant() const
{
    return mFitMKCurve_nonlinear_withconstant;
}

void tBendingStiffness::CreateFittedList_Nonlinear_withconstant(tMKCurve &mk_curve) const
{
    int num_of_div = 200;
    tVectorXf Klst = tVectorXf::LinSpaced(num_of_div, mKmin, mKmax);

    tVectorXf K2lst = Klst.array().pow(2);
    tVectorXf Mlst = mNonLinearBendingStiffness_a * K2lst + mNonLinearBendingStiffness_b * Klst + mNonLinearBendingStiffness_c * tVectorXf::Ones(num_of_div);
    SetMlist(mk_curve, Mlst);
    SetKlist(mk_curve, Klst);
}