#pragma once
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
#include "BendingData.h"

/**
 * \brief           given the bending data for one side of the fabric, collect the critical "warp, weft, bias" (nonlinear) bending stiffness, then convert them into Linctex GUI value
*/
class tBendingStiffnessCloth
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit tBendingStiffnessCloth();
    void Init(const tBendingDataList &data_lst);
    tVector3d GetLinearGUIValue() const;
    tVectorXd GetNonLinearGUIValue_1st() const;
    tVectorXd GetNonLinearGUIValue_2nd() const;
protected:
    // --------- bending stiffnes [N * m^2] ----------
    double mRhoG_SI;
    tVector3d mLinearBendingStiffness;                                                // linear bending stiffness [N \cdot m^2]
    tVector3d mNonLinearBendingStiffness_1stterm, mNonLinearBendingStiffness_2ndterm; // nonlinear bending stiffness [N \cdot m^2]
    void FetchStiffnessRaw(const tBendingDataList &data_lst);
    void ConvertToGUIValue();
    virtual double GetRhoGSI(const tBendingDataList &data_lst) const;
    // --------- bending stiffness GUI(0-100) -----------
    tVector3d mLinearBendingStiffnessGUI;
    tVector3d mNonLinearBendingStiffness_2ndtermGUI;
    tVector3d mNonLinearBendingStiffness_1sttermGUI;
};
SIM_DECLARE_PTR(tBendingStiffnessCloth);