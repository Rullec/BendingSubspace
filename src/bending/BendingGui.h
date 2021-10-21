#pragma once
#include "render/RenderableBase.h"
#include "bending/load/BendingDataLoader.h"
#include <string>

SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cBendingGui : public cRenderable
{
public:
    explicit cBendingGui();
    virtual void Init(std::string root_path);
    virtual void UpdateGui() override;
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() override;
    virtual void UpdatePlot() override;

protected:
    std::string mRawBendingDataRootDir;
    tBendingClothArray mBendingData;
    std::vector<std::string> mClothName, mAngleName;
    std::vector<std::string> mFaceName = {
        "Front", "Back"};
    struct
    {
        int mCurClothId, mCurFaceId, mCurAngleId;
        bool mEnableDrawBezier;
        bool mEnableNonlinearSolve;
        bool mSelectK1InNonlinearSolver; // select K1 in nonlinear shooting method solver
        void Init()
        {
            mCurClothId = 0, mCurFaceId = 0, mCurAngleId = 0;
            mEnableDrawBezier = false;
            mEnableNonlinearSolve = false;
            mSelectK1InNonlinearSolver = true;
            Sync();
        }
        bool IsChanged()
        {
            return (mCurClothId != mBeforeClothId) || (mCurFaceId != mBeforeFaceId) || (mCurAngleId != mBeforeAngleId) || (mEnableDrawBezier != mEnableDrawBezierBefore) || (mEnableNonlinearSolve != mEnableNonlinearSolveBefore) ||
                   (mSelectK1InNonlinearSolver != mSelectK1InNonlinearSolverBefore);
        }
        void Sync()
        {
            mBeforeClothId = mCurClothId;
            mBeforeFaceId = mCurFaceId;
            mBeforeAngleId = mCurAngleId;
            mEnableDrawBezierBefore = mEnableDrawBezier;
            mEnableNonlinearSolveBefore = mEnableNonlinearSolve;
            mSelectK1InNonlinearSolverBefore = mSelectK1InNonlinearSolver;
        }

    protected:
        int mBeforeClothId, mBeforeFaceId, mBeforeAngleId;
        bool mEnableDrawBezierBefore, mEnableNonlinearSolveBefore, mSelectK1InNonlinearSolverBefore;
    } mSelectState;

    bool mEnableDrawMK; // enable to draw torque-curvature relationship
    cRenderResourcePtr mEmptyResource, mRealPictureResource;
    tVectorXf mBendingTorqueRawList, mBendingCurvatureRawList;
    tVectorXf mBendingTorqueCutList, mBendingCurvatureCutList;
    float mLinearBendingStiffness;                                                // estimated linear bending stiffness
    float mNonLinearBendingStiffness_2ndterm, mNonLinearBendingStiffness_1stterm; // estimated nonlinear bending stiffness
    tVectorXd mSolvedX, mSolveY;                                                  // solved x_lst and y_lst
    double mBeta;                                                                 // variable in linear shooting method
    double mM, mN;                                                                // variable in nonlinear shooting method
    void UpdateClothResource();
    tBendingDataPtr GetCurData();
    void ExportClothStiffness();
    void ExportClothStiffnessTrain();
};

SIM_DECLARE_PTR(cBendingGui);
