#pragma once
#include "BezierCurve.h"
#include "utils/OpenCVUtil.h"
#include "BendingStiffness.h"

/**
 * \brief           the bending curve which has physical meaning for curved fabric. It can estimate the bending stiffness (nonlinear or linear)
 * 
*/
class cBezierCurvePhysics : public cBezierCurve
{
public:
    explicit cBezierCurvePhysics(int num_of_div, const tVector2d &A,
                                 const tVector2d &B, const tVector2d &C,
                                 const tVector2d &D, double rho_g, bool cutted_from_highest_point = false,
                                 bool cutted_to_zero_curvature_point = false);
    virtual tVectorXd GetTorqueList() const;
    virtual tMKCurve GetTorqueCurvatureCurve() const;
    virtual tBendingStiffnessPtr GetEstimatedBendingStiffness();
    virtual double GetRhoG() const;

protected:
    tVectorXd mTorqueList; // torque list, calculate frm given linear density "rho * g"
    tMKCurve mTorqueCurvatureCurve;
    tBendingStiffnessPtr mBendingStiffness; // bending stiffness data
    double mRhoG;                           // rho * g
    void CutFromHighestPoint();
    void CutToNegativeCurvaturePoint();
    virtual tVectorXd CalculateTorque() const;
};

SIM_DECLARE_PTR(cBezierCurvePhysics);