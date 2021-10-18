#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
// please use the set-get method pair to visit curve data
typedef std::pair<tVectorXf, tVectorXf> tMKCurve; // first = M, second = K
const tVectorXf &GetMlist(const tMKCurve &curve);
const tVectorXf &GetKlist(const tMKCurve &curve);
template <typename T>
void SetMlist(tMKCurve &curve, const T &Mlist)
{
    curve.first = Mlist.cast<float>();
}
template <typename T>
void SetKlist(tMKCurve &curve, const T &Klist)
{
    curve.second = Klist.cast<float>();
}

/**
 * \brief       Given the M-K relationship (Torque-Curvature relationship), we can calculate its linear or nonlinear BendingStiffness 
*/
class tBendingStiffness
{
public:
    tBendingStiffness(
        const tMKCurve &mkcurve);
    float GetLinearBendingStiffness();
    void GetNonLinearBendingStiffness(float &a, float &b);
    const tMKCurve &GetLinear_FittedMKList() const;
    const tMKCurve &GetNonLinear_FittedMKList() const;

protected:
    tMKCurve mRawMKCurve, mFitMKCurve_linear, mFitMKCurve_nonlinear;
    float mKmin, mKmax; // curvature K range [min, max], used to calculate the fitted curve
    // the linera formula: M = a * K
    float mLinearBendingStiffness;
    // the nonlinera formula: M = a * K^2 + bK
    float mNonLinearBendingStiffness_a, mNonLinearBendingStiffness_b;

    float CreateLinearBendingStiffness() const;
    void CreateNonLinearBendingStiffness(float &k1, float &k2) const;
    void CreateFittedList_Linear(tMKCurve &mk_curve) const;
    void CreateFittedList_Nonlinear(tMKCurve &mk_curve) const;
};

SIM_DECLARE_PTR(tBendingStiffness);