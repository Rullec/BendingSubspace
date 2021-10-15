#pragma once
#include "BezierCurve.h"
#include "utils/OpenCVUtil.h"


class cBezierCurvePhysics : public cBezierCurve
{
public:
    explicit cBezierCurvePhysics(int num_of_div, const tVector2d &A,
                                 const tVector2d &B, const tVector2d &C,
                                 const tVector2d &D, double rho_g);
    virtual tVectorXd GetTorqueList() const;
    virtual cv::Mat GetTorqueCurvatureCurve() const;
    
protected:
    tVectorXd mTorqueList;
    double mRhoG; // rho * g
    virtual tVectorXd CalculateTorque() const;
};

SIM_DECLARE_PTR(cBezierCurvePhysics);