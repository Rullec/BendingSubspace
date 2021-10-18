#include "BendingGui.h"
#include "bending/load/BendingDataLoader.h"
#include "render/RenderResource.h"
#include "utils/LogUtil.h"
#include "imgui.h"
#include "bending/bezier/BezierCurvePhysics.h"
#include <iostream>

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
    ImGui::Spacing();
    {
        auto data = GetCurData();
        ImGui::Text("rho*G = %.3f, unitCM = %.3f, scale = %d", data->GetRhoG(), data->GetUnitCM(), data->GetScale());
        ImGui::Spacing();

        auto bending_stiffness = data->GetBezierPhysicCutted()->GetEstimatedBendingStiffness();
        float linear_bs = bending_stiffness->GetLinearBendingStiffness();
        float nonlinear_bs_a, nonlinear_bs_b;
        bending_stiffness->GetNonLinearBendingStiffness(nonlinear_bs_a, nonlinear_bs_b);
        ImGui::Text("linear bs %.3e", linear_bs);
        ImGui::Text("nonlinear bs %.3e, %.3e", nonlinear_bs_a, nonlinear_bs_b);
        ImGui::Text("(bs stands for bending stiffness)");
    }
    if (mSelectState.IsChanged() == true)
    {
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

    // begin to do loading
    {
        auto data = GetBendingData(cur_cloth, face_id, angle);
        ;

        // load img
        cv::Mat img = data->GetPicture(mSelectState.mEnableDrawBezier);
        mRealPictureResource->ConvertFromOpencv(img);
        // load bending M-K list (cutted and raw)
        // auto raw_bezier = data->GetBezierPhysic();
        // auto cut_bezier = data->GetBezierPhysicCutted();
        // raw_bezier->GetTorqueCurvatureCurve(mBendingTorqueRawList, mBendingCurvatureRawList);
        // cut_bezier->GetTorqueCurvatureCurve(mBendingTorqueCutList, mBendingCurvatureCurList);
    }
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
    // ImPlot::ShowDemoWindow();
    if (mEnableDrawMK == true)
    {
        if (ImPlot::BeginPlot("plot", "curvature", "log(torque)"))
        {
            // 1. get the M-K relationship from the bending data
            auto data = GetCurData();
            // plot the raw bezier
            // {
            //     auto bezier_phy = data->GetBezierPhysic();

            //     tMKCurve curve = bezier_phy->GetTorqueCurvatureCurve();
            //     LogYPlotCurve("raw", curve);
            // }

            // plot the cutted
            {
                auto bezier_phy_cutted = data->GetBezierPhysicCutted();

                tMKCurve curve = bezier_phy_cutted->GetTorqueCurvatureCurve();
                LogYPlotCurve("cutted_raw", curve);
            }

            // plot the linear stiffness fitted curve (from cutted result)
            {
                auto bezier_phy_cutted = data->GetBezierPhysicCutted();
                auto bending_stiffness = bezier_phy_cutted->GetEstimatedBendingStiffness();
                tMKCurve curve = bending_stiffness->GetLinear_FittedMKList();
                LogYPlotCurve("linear_fit", curve);
            }
            {
                auto bezier_phy_cutted = data->GetBezierPhysicCutted();
                auto bending_stiffness = bezier_phy_cutted->GetEstimatedBendingStiffness();
                tMKCurve curve = bending_stiffness->GetNonLinear_FittedMKList();
                LogYPlotCurve("nonlinear_fit", curve);
            }
            ImPlot::EndPlot();
        }
    }
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
