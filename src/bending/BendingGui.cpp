#include "BendingGui.h"
#include "bending/load/BendingDataLoader.h"
#include "render/RenderResource.h"
#include "utils/LogUtil.h"
#include "imgui.h"
#include "bending/bezier/BezierCurvePhysics.h"
#include <iostream>
#include "bezier/BezierShootSolver.h"
namespace ImGui
{
    bool Combo(const char *label, int *current_item, const std::vector<std::string> &name_lst, int items_count, int height_in_items = -1)
    {
        std::vector<const char *> input_name_lst(0);
        for (auto &name : name_lst)
            input_name_lst.push_back(name.c_str());
        return ImGui::Combo(label, current_item, input_name_lst.data(), items_count, height_in_items);
    };
};
cBendingGui::cBendingGui()
{
    mEnableDrawMK = true;
}
#include "implot.h"
void cBendingGui::UpdateGui()
{
    ImVec2 init_window_size = ImVec2(300, 200);
    ImGui::SetNextWindowSize(init_window_size, ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = 0;
    bool open = false;
    bool *p_open = &open;

    ImGui::Begin("config", p_open, window_flags);
    // ImGui::Combo("cloth", &mCurClothId, mClothName, mClothName.size());
    ImGui::Combo("cloth", &mSelectState.mCurClothId, mClothName, mClothName.size());
    ImGui::Spacing();
    ImGui::Combo("face", &mSelectState.mCurFaceId, {"front", "back"}, 2);
    ImGui::Spacing();
    ImGui::Combo("angle", &mSelectState.mCurAngleId, mAngleName, mAngleName.size());
    ImGui::Spacing();
    ImGui::Checkbox("draw_bezier", &mSelectState.mEnableDrawBezier);
    ImGui::SameLine();
    ImGui::Checkbox("Torque-Curvature plot", &mEnableDrawMK);
    ImGui::SameLine();
    ImGui::Checkbox("Enable nonlinear", &mSelectState.mEnableNonlinearSolve);
    if (mSelectState.mEnableNonlinearSolve)
    {
        ImGui::SameLine();
        ImGui::Checkbox("selct k1 nonlinear", &mSelectState.mSelectK1InNonlinearSolver);
    }
    ImGui::Spacing();
    {
        auto data = GetCurData();
        ImGui::Text("rho*G = %.3f, unitCM = %.3f, scale = %d", data->GetRhoG(), data->GetUnitCM(), data->GetScale());
        ImGui::Spacing();

        auto bending_stiffness = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature()->GetEstimatedBendingStiffness();

        float linear_bs = bending_stiffness->GetLinearBendingStiffness();
        float nonlinear_bs_a, nonlinear_bs_b;
        bending_stiffness->GetNonLinearBendingStiffness(nonlinear_bs_a, nonlinear_bs_b);
        ImGui::Text("linear bs %.3e", linear_bs);
        ImGui::Text("nonlinear bs %.3e, %.3e", nonlinear_bs_a, nonlinear_bs_b);
        ImGui::Text("(bs stands for bending stiffness), beta=%.1f, length = %.3f", mBeta, data->GetBezierPhysicCuttedFromHighest()->GetTotalLength());
    }
    if (mSelectState.IsChanged() == true)
    {
        std::cout << "change to cloth " << mSelectState.mCurClothId << std::endl;
        mSelectState.Sync();
        UpdateClothResource();
    }

    ImGui::End();
    // if (mCurClothId != mPrevClothId)
    // {
    //     UpdateCloth();
    //     mPrevClothId = mCurClothId;
    // }
}
std::vector<cRenderResourcePtr> cBendingGui::GetRenderingResource()
{
    if (mRealPictureResource->IsEmpty())
    {

        return {mEmptyResource};
    }
    else
    {
        return {mRealPictureResource};
    }
}

#include "utils/OpenCVUtil.h"
#include "utils/TimeUtil.hpp"
void cBendingGui::Init(std::string root_path)
{
    mRawBendingDataRootDir = root_path;
    cTimeUtil::Begin("init bezier");
    mBendingData = BuildBendingClothArray(mRawBendingDataRootDir);
    cTimeUtil::End("init bezier");
    SIM_INFO("load {} bending data from {}", mBendingData.size(), mRawBendingDataRootDir);

    mClothName = BuildClothName(mBendingData);
    mAngleName = BuildAngleName(mBendingData);
    mEmptyResource = std::make_shared<cRenderResource>();
    mRealPictureResource = std::make_shared<cRenderResource>();

    mEmptyResource->ConvertFromOpencv(cOpencvUtil::ScaleDownImageToRange(cOpencvUtil::LoadRGBImage("assets/test_screen.jpeg"), 800));

    mSelectState.Init();
    mSelectState.mEnableDrawBezier = true;
    mSelectState.mEnableNonlinearSolve = true;
    mSelectState.mCurClothId = 0;
    mSelectState.Sync();
    UpdateClothResource();
}

void cBendingGui::UpdateClothResource()
{
    auto cur_cloth = mBendingData[mSelectState.mCurClothId];
    int face_id = mSelectState.mCurFaceId;
    int angle_id = mSelectState.mCurAngleId;
    float angle = std::stof(mAngleName[angle_id]);
    std::cout << cur_cloth->GetDir() << " face " << face_id << " angle " << angle << " enable draw bezier " << mSelectState.mEnableDrawBezier << std::endl;

    auto data = GetBendingData(cur_cloth, face_id, angle);
    // begin to do loading
    {
        // load img
        cv::Mat img = data->GetPicture(mSelectState.mEnableDrawBezier);
        mRealPictureResource->ConvertFromOpencv(img);
    }

    {
        auto cutted_bezier_short = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();
        auto bending_stiffness = cutted_bezier_short->GetEstimatedBendingStiffness();
        // 1. get the rho * g, and bending stiffness G
        double linear_bs = bending_stiffness->GetLinearBendingStiffness();
        std::cout << "linear bs = " << linear_bs << std::endl;
        double rho_g = cutted_bezier_short->GetRhoG();
        std::cout << "rho*G = " << rho_g << std::endl;

        double total_arclength = data->GetBezierPhysicCuttedFromHighest()->GetTotalLength();
        std::cout << "total_arclength = " << total_arclength << std::endl;
        // exit(1);
        // double beta = -rho_g * std::pow(total_arclength, 3) / (linear_bs);
        double beta = rho_g * std::pow(total_arclength, 3) / (linear_bs);
        // beta *= 10;
        // 2. get init theta

        double theta0 = cutted_bezier_short->GetInitTheta();

        // 3. calculate t
        double t0 = 0;
        {
            double k_min = 1e-10;
            double k_max = 1e-5;
            // std::cout << "linear bs = " << linear_bs << std::endl;
            SIM_ASSERT(linear_bs > k_min);
            SIM_ASSERT(linear_bs < k_max);

            t0 = (std::log(linear_bs) - std::log(k_min)) / (std::log(k_max) - std::log(k_min)) * 0.1;
            t0 = std::min(t0, 0.3) / total_arclength;
        }
        mBeta = beta;
        // std::cout << "beta = " << beta << std::endl;
        // std::cout << "theta0 = " << theta0 << std::endl;
        // std::cout << "t0 = " << t0 << std::endl;
        // exit(1);
        // double stepsize = 1e-1;
        // double beta, double theta0, int num_of_samples, tVectorXd &x_solved,
        // tVectorXd &y_solved

        // linear
        if (mSelectState.mEnableNonlinearSolve == false)
        {
            double min_err = cBezierShootSolver::ShootLinearSolveRobust(beta, theta0, 2000, mSolvedX, mSolveY);
            mSolvedX *= total_arclength;
            mSolveY *= total_arclength;
        }

        else
        {
            std::cout << "[nonlinear] init curvature = " << cutted_bezier_short->GetCurvatureList()[0] << std::endl;
            float a = 0, b = 0;
            bending_stiffness->GetNonLinearBendingStiffness(a, b);
            cBezierShootSolver::ShootNonLinearSolveRobustNormalized(rho_g, a, b, total_arclength, theta0, 2000, mSolvedX, mSolveY, mSelectState.mSelectK1InNonlinearSolver);
            // nonlinear
            // double m = 2 * a / (rho_g * std::pow(total_arclength, 4)), n = b / (rho_g * std::pow(total_arclength, 3));

            // cBezierShootSolver::ShootNonLinearSolveRobust(m, n, theta0, 2000, mSolvedX, mSolveY);
            // mSolvedX *= total_arclength;
            // mSolveY *= total_arclength;
        }
    }
    // {
    //     auto cutted_bezier_short = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();
    //     auto bending_stiffness = cutted_bezier_short->GetEstimatedBendingStiffness();
    //     // 1. get the rho * g, and bending stiffness G
    //     float a = 0, b = 0;
    //     bending_stiffness->GetNonLinearBendingStiffness(a, b);
    //     double rho_g = cutted_bezier_short->GetRhoG();

    //     double total_arclength = data->GetBezierPhysicCuttedFromHighest()->GetTotalLength();
    //     // exit(1);
    //     // double beta = -rho_g * std::pow(total_arclength, 3) / (linear_bs);
    //     std::cout << "a = " << a << " b = " << b << std::endl;
    //     mM = a / (rho_g * std::pow(total_arclength, 4));
    //     mN = -b / (rho_g * std::pow(total_arclength, 3));
    //     std::cout << "M = " << mM << " n = " << mN << std::endl;
    //     // 2. get init theta

    //     double theta0 = cutted_bezier_short->GetInitTheta();

    //     // 3. calculate t
    //     double t0 = 0;
    //     {
    //         double k_min = 1e-8;
    //         double k_max = 1e-5;
    //         SIM_ASSERT(b > k_min);
    //         SIM_ASSERT(b < k_max);

    //         t0 = (std::log(b) - std::log(k_min)) / (std::log(k_max) - std::log(k_min)) * 0.1;
    //         t0 = std::min(t0, 0.3) / total_arclength;

    //         if (mM < 0)
    //         {
    //             t0 = std::min(t0, -mN * mN / (4 * mM) / 2);
    //         }
    //     }
    //     // exit(1);
    //     double stepsize = 1e-1;
    //     double min_err = cBezierShootSolver::ShootNonLinearSolve(
    //         mM, mN, theta0, t0, stepsize, mSolvedX, mSolveY);
    //     mSolvedX *= total_arclength;
    //     mSolveY *= total_arclength;
    // }
}
void PlotCurve(std::string title, const tMKCurve &curve)
{
    ImPlot::PlotLine(title.c_str(), GetKlist(curve).data(), GetMlist(curve).data(), GetMlist(curve).size());
}
void LogYPlotCurve(std::string title, const tMKCurve &curve)
{
    tVectorXf new_M = GetMlist(curve).array().log();
    ImPlot::PlotLine(title.c_str(), GetKlist(curve).data(), new_M.data(), new_M.size());
}

void cBendingGui::UpdatePlot()
{
    // ImPlot::SetNextPlotLimitsX(-0.1, 0.1);
    // ImPlot::SetNextPlotLimitsY(-0.15, 0.05);

    if (mEnableDrawMK == true)
    {
        auto data = GetCurData();
        if (ImPlot::BeginPlot("plot", "curvature", "torque", ImVec2(300, 300), ImPlotFlags_Equal))
        {

            // 1. draw the M-K curve

            // 1. get the M-K relationship from the bending data
            // plot the raw bezier
            // {
            //     auto bezier_phy = data->GetBezierPhysic();

            //     tMKCurve curve = bezier_phy->GetTorqueCurvatureCurve();
            //     PlotCurve("raw", curve);
            // }

            // // plot the cutted
            // {
            //     auto bezier_phy_cutted = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();

            //     tMKCurve curve = bezier_phy_cutted->GetTorqueCurvatureCurve();
            //     PlotCurve("cutted_raw", curve);
            // }

            // // plot the linear stiffness fitted curve (from cutted result)
            // {
            //     auto bezier_phy_cutted = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();
            //     auto bending_stiffness = bezier_phy_cutted->GetEstimatedBendingStiffness();
            //     tMKCurve curve = bending_stiffness->GetLinear_FittedMKList();
            //     PlotCurve("linear_fit", curve);
            // }
            // {
            //     auto bezier_phy_cutted = data->GetBezierPhysic_CuttedFromHighest_ToZeroCurvature();
            //     auto bending_stiffness = bezier_phy_cutted->GetEstimatedBendingStiffness();
            //     tMKCurve curve = bending_stiffness->GetNonLinear_FittedMKList();
            //     PlotCurve("nonlinear_fit", curve);
            // }
            // 2. draw the solved M-K curve
            {
                ImPlot::PlotLine("shoot", mSolvedX.data(), mSolveY.data(), mSolvedX.size());

                auto pt_lst = GetCurData()->GetBezierPhysicCuttedFromHighest()->GetPointList();
                tVectorXd Xraw(pt_lst.size()), Yraw(pt_lst.size());
                for (int i = 0; i < pt_lst.size(); i++)
                {
                    Xraw[i] = pt_lst[i][0];
                    Yraw[i] = pt_lst[i][1];
                }
                ImPlot::PlotLine("raw", Xraw.data(), Yraw.data(), Xraw.size());
            }
        }
    }

    ImPlot::EndPlot();
}

tBendingDataPtr cBendingGui::GetCurData()
{
    auto cur_cloth = mBendingData[mSelectState.mCurClothId];
    int face_id = mSelectState.mCurFaceId;
    int angle_id = mSelectState.mCurAngleId;
    float angle = std::stof(mAngleName[angle_id]);
    auto data = GetBendingData(cur_cloth, face_id, angle);
    return data;
}
