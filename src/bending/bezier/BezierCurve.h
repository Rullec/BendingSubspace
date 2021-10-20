#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
#include "memory"
/**
 * \brief       discretize a cubic beizer curve (determined by 4 points)
 */
class cBezierCurve : std::enable_shared_from_this<cBezierCurve>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit cBezierCurve(int num_of_div, const tVector2d &A,
                          const tVector2d &B, const tVector2d &C,
                          const tVector2d &D);
    virtual double GetTotalLength() const;
    virtual int GetNumOfDrawEdges() const;
    virtual tEigenArr<tVector2d> GetPointList();
    virtual double GetInitTheta() const;
    virtual tVectorXd GetCurvatureList() const;

protected:
    // tVectorXf mDrawBuffer;
    int mNumOfDiv;
    tVector2d A, B, C, D;
    // tMatrixXd mPointList;     // N
    tVectorXd mCurvatureList; // N
    tVectorXd mArclengthList; // N - 1
    tVectorXd mU, mOneMinusU; // N
    tVectorXd mPosX, mPosY;   // N
    virtual void InitPointlist(tMatrixXd &point_lst);
    virtual tVectorXd CalculateCurvature() const;
    virtual tVectorXd CalculateArcLengthList() const;
};

SIM_DECLARE_PTR(cBezierCurve);