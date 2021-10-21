#include "bending/load/BendingDataFace.h"
#include "utils/LogUtil.h"
#include "bending/bezier/BendingStiffnessToLinctexGUIConverter.h"
tBendingStiffnessCloth::tBendingStiffnessCloth()
{
    mLinearBendingStiffness.setZero();
    mNonLinearBendingStiffness_1stterm.setZero();
    mNonLinearBendingStiffness_2ndterm.setZero();

    mLinearBendingStiffnessGUI.setZero();
    mNonLinearBendingStiffness_1sttermGUI.setZero();
    mNonLinearBendingStiffness_2ndtermGUI.setZero();
}

#include "bending/bezier/BezierCurvePhysics.h"
void tBendingStiffnessCloth::Init(const tBendingDataList &data_lst)
{
    mRhoG_SI = GetRhoGSI(data_lst);
    // std::cout << "[convert] rhog = " << mRhoG_SI << std::endl;
    FetchStiffnessRaw(data_lst);
    // std::cout << "linear bending stiffness warp_weft_bias = " << mLinearBendingStiffness.transpose() << std::endl;
    // std::cout << "nonlinear bending stiffness 1st warp_weft_bias = " << mNonLinearBendingStiffness_1stterm.transpose() << std::endl;
    // std::cout << "nonlinear bending stiffness 2nd warp_weft_bias = " << mNonLinearBendingStiffness_2ndterm.transpose() << std::endl;
    ConvertToGUIValue();
    // std::cout << "convert to GUI value done\n";
}

void tBendingStiffnessCloth::FetchStiffnessRaw(const tBendingDataList &data_lst)
{
    auto GetIndex = [](double angle)
    {
        if (std::fabs(angle - 0) < 1e-6)
        {
            return 0;
        }
        else if (std::fabs(angle - 90) < 1e-6)
        {
            return 1;
        }
        else if (std::fabs(angle - 45) < 1e-6)
        {
            return 2;
        }
        return -1;
    };
    bool visited[3] = {false, false, false};
    for (auto &data : data_lst)
    {
        double angle = data->GetWarpWeftAngle();
        int idx = GetIndex(angle);
        if (idx == -1)
        {
            continue;
        }
        else
        {
            SIM_ASSERT(visited[idx] == false);
            visited[idx] = true;
            auto bezier = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();
            auto bending_stiff = bezier->GetEstimatedBendingStiffness();
            double linear_bs = bending_stiff->GetLinearBendingStiffness();
            float nonlinear_bs_1stterm, nonlinear_bs_2ndterm;
            bending_stiff->GetNonLinearBendingStiffness(nonlinear_bs_2ndterm, nonlinear_bs_1stterm);

            mLinearBendingStiffness[idx] = linear_bs;
            mNonLinearBendingStiffness_1stterm[idx] = nonlinear_bs_1stterm;
            mNonLinearBendingStiffness_2ndterm[idx] = nonlinear_bs_2ndterm;
        }
    }
    SIM_ASSERT(visited[0] == true);
    SIM_ASSERT(visited[1] == true);
    SIM_ASSERT(visited[2] == true);
}

/**
 * \brief           calculate the GUI value from the calculated stiffness
*/
void tBendingStiffnessCloth::ConvertToGUIValue()
{
    mLinearBendingStiffnessGUI = cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_Linear(
        this->mLinearBendingStiffness, mRhoG_SI);
    // std::cout << "convert GUI = " << mLinearBendingStiffnessGUI.transpose() << std::endl;
}

double tBendingStiffnessCloth::GetRhoGSI(const tBendingDataList &data_lst) const
{
    tVectorXd rhog_si(data_lst.size());
    for (int i = 0; i < data_lst.size(); i++)
    {
        auto &data = data_lst[i];
        rhog_si[i] = data->GetRhoG();
    }
    tVectorXd diff = rhog_si.eval() - rhog_si.eval().mean() * tVectorXd::Ones(data_lst.size());
    SIM_ASSERT(diff.norm() < 1e-6);
    return rhog_si[0];
}
tVector3d tBendingStiffnessCloth::GetLinearGUIValue() const
{
    return this->mLinearBendingStiffnessGUI;
}