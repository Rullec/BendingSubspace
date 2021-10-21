#include "BendingDataCloth.h"
#include "utils/FileUtil.h"
#include "utils/RegexUtil.h"
#include <iostream>
bool GetFrontAndBackMat(std::string root_dir,
                        std::vector<std::string> &front_mat_path,
                        std::vector<std::string> &back_mat_path)
{
    front_mat_path.clear();
    back_mat_path.clear();
    bool failed = false;
    for (auto file_with_dir : cFileUtil::ListDir(root_dir))
    {
        auto file = cFileUtil::GetFilename(file_with_dir);
        if ("mat" == cFileUtil::GetExtension(file))
        {
            if ("front" == cFileUtil::ConvertToLowerCase(file.substr(0, 5)))
            {
                front_mat_path.push_back(file);
            }
            else if ("back" == cFileUtil::ConvertToLowerCase(file.substr(0, 4)))
            {
                back_mat_path.push_back(file);
            }
            else
            {
                failed = true;
                SIM_WARN("unclassified mat path {}", file_with_dir);
                break;
            }
        }
    }
    return failed;
}

// bool CompareMat(std::string img_path0, std::string img_path1)
// {
//     double angle0 = GetWarpWeftAngleFromFilename(img_path0);
//     double angle1 = GetWarpWeftAngleFromFilename(img_path1);
//     return angle0 < angle1;
// }

tBendingDataCloth::tBendingDataCloth()
{
}

bool tBendingDataCloth::Init(std::string data_dir, const std::map<int, double> &density_map)
{
    mDataDir = data_dir;

    // begin to init
    // 1. parse the index
    std::string subdir_single = cFileUtil::GetFilename(data_dir);
    auto res = cRegexUtil::FindAllMatch(subdir_single, "^[0-9]+");
    SIM_ASSERT(res.size() == 1);
    mId = std::stoi(res[0]);
    {
        auto iter = density_map.find(mId);
        if (iter == density_map.end())
        {
            SIM_ERROR("failed to find cloth {} info in density map", mId);
        }
        else
        {
            mRhoG = iter->second * 9.81;
            // std::cout << "rhog = " << mRhoG << std::endl;
        }
    }
    // 2. create the front and back data
    std::vector<std::string> front_paths, back_paths;
    if (true == GetFrontAndBackMat(data_dir, front_paths, back_paths))
    {
        std::cout << "failed in dir " << data_dir << std::endl;
        return false;
    }

    // 2. split them into front and backs
    SIM_ASSERT(front_paths.size() == back_paths.size());

    mFrontData.resize(front_paths.size());
    mBackData.resize(back_paths.size());

    OMP_PARALLEL_FOR
    for (int i = 0; i < front_paths.size(); i++)
    {
        auto data = std::make_shared<tBendingData>();
        data->Init(cFileUtil::ConcatFilename(mDataDir, front_paths[i]), mRhoG);
        mFrontData[i] = data;
    }
    OMP_PARALLEL_FOR
    for (int i = 0; i < back_paths.size(); i++)
    {
        auto data = std::make_shared<tBendingData>();
        data->Init(cFileUtil::ConcatFilename(mDataDir, back_paths[i]), mRhoG);
        mBackData[i] = data;
    }

    auto data_sort = [](const tBendingDataPtr &ptr0, const tBendingDataPtr &ptr1)
    {
        return ptr0->GetWarpWeftAngle() < ptr1->GetWarpWeftAngle();
    };
    std::sort(mFrontData.begin(), mFrontData.end(), data_sort);
    std::sort(mBackData.begin(), mBackData.end(), data_sort);

    // after init, begin to collect
    InitBendingStiffness();
    return true;
}

tVectorXf tBendingDataCloth::GetFrontAngleList()
{
    tVectorXf front_angle_lst(mFrontData.size());
    for (int i = 0; i < mFrontData.size(); i++)
    {
        auto data = mFrontData[i];
        front_angle_lst[i] = data->GetWarpWeftAngle();
    }
    return front_angle_lst;
}
tVectorXf tBendingDataCloth::GetBackAngleList()
{
    tVectorXf back_angle_lst(mBackData.size());
    for (int i = 0; i < mBackData.size(); i++)
    {
        auto data = mBackData[i];
        back_angle_lst[i] = data->GetWarpWeftAngle();
    }
    return back_angle_lst;
}

tBendingDataPtr tBendingDataCloth::GetFrontDataByIdx(int idx)
{
    return this->mFrontData[idx];
}
tBendingDataPtr tBendingDataCloth::GetBackDataByIdx(int idx)
{
    return mBackData[idx];
}
tBendingDataPtr tBendingDataCloth::GetFrontDataByAngle(float angle)
{
    for (auto &x : mFrontData)
    {
        float diff = std::fabs(x->GetWarpWeftAngle() - angle);

        if (diff < 1e-3)
        {
            return x;
        }
    }
    SIM_WARN("failed to find angle {} item in front data of {}", angle, this->mDataDir);
    return nullptr;
}
tBendingDataPtr tBendingDataCloth::GetBackDataByAngle(float angle)
{
    for (auto &x : mBackData)
    {
        float diff = std::fabs(x->GetWarpWeftAngle() - angle);

        if (diff < 1e-3)
        {
            return x;
        }
    }
    SIM_WARN("failed to find angle {} item in back data of {}", angle, this->mDataDir);
    return nullptr;
}

double tBendingDataCloth::GetRhoG() const
{
    return this->mRhoG;
}
tBendingDataList tBendingDataCloth::GetFrontDataList() const
{
    return mFrontData;
}
tBendingDataList tBendingDataCloth::GetBackDataList() const
{
    return mBackData;
}

/**
 * \brief           init bending stiffness
 * 1. bending stiffness [SI] [N \cdot m^2]
 * 2. bending stiffness GUI 
*/
void tBendingDataCloth::InitBendingStiffness()
{
    mFrontBendingStiffness = std::make_shared<tBendingStiffnessCloth>();
    mFrontBendingStiffness->Init(mFrontData);
    mBackBendingStiffness = std::make_shared<tBendingStiffnessCloth>();
    mBackBendingStiffness->Init(mBackData);
    // std::cout << "[linc] data " << GetDir() << " front linear GUI = " << mFrontBendingStiffness->GetLinearGUIValue().transpose() << std::endl;
    // std::cout << "[linc] data " << GetDir() << " front linear GUI = " << mBackBendingStiffness->GetLinearGUIValue().transpose() << std::endl;
    // std::cout << "init bending stiffness done\n";
    // exit(1);
}

/**
 * \brief       return the warp, weft bias bending stiffness
*/
tBendingStiffnessClothPtr tBendingDataCloth::GetFrontBendingStiffness() const // get bending stiffness (warp, weft, bias) (non) linear data
{
    return mFrontBendingStiffness;
}
/**
 * \brief       return the warp, weft bias bending stiffness
*/
tBendingStiffnessClothPtr tBendingDataCloth::GetBackBendingStiffness() const // get bending stiffness (warp, weft, bias) (non) linear data
{
    return mBackBendingStiffness;
}