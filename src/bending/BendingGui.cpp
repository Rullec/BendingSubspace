#include "BendingGui.h"
#include "bending/load/BendingDataLoader.h"
#include "render/RenderResource.h"
#include "utils/LogUtil.h"
#include "imgui.h"
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
}
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

void cBendingGui::Init(std::string root_path)
{
    mRawBendingDataRootDir = root_path;
    mBendingData = BuildBendingClothArray(mRawBendingDataRootDir);
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
        cv::Mat img = data->GetPicture(mSelectState.mEnableDrawBezier);
        // cv::Point centerCircle2(100, 200);
        // cv::Scalar colorCircle2(0, 100, 0);

        // cv::circle(img, centerCircle2, 30, colorCircle2, CV_FILLED);
        // std::cout << img.rows << " " << img.cols << std::endl;
        mRealPictureResource->ConvertFromOpencv(img);
    }
}